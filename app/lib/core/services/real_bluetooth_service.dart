import 'dart:async';
import 'dart:convert';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';
import '../models/oxyfeeder_status.dart';
import 'bluetooth_service_interface.dart';

/// Real Bluetooth service that connects to the OxyFeeder ESP32 hardware
/// via BLE and streams sensor data to the app.
///
/// Usage:
/// 1. Create instance (auto-connects on creation)
/// 2. Listen to statusStream for sensor data
/// 3. Listen to connectionStateStream for connection status UI
class RealBluetoothService implements BluetoothServiceInterface {
  // OxyFeeder BLE identifiers (must match ESP32 firmware)
  static const String deviceName = 'OxyFeeder';
  static const String serviceUuid = '0000abcd-0000-1000-8000-00805f9b34fb';
  static const String characteristicUuid = '0000abce-0000-1000-8000-00805f9b34fb';
  static const String commandCharacteristicUuid = '0000abcf-0000-1000-8000-00805f9b34fb';

  // Stream controller for status updates
  final StreamController<OxyFeederStatus> _statusStreamController =
      StreamController<OxyFeederStatus>.broadcast();

  @override
  Stream<OxyFeederStatus> get statusStream => _statusStreamController.stream;

  // Connection state
  BluetoothDevice? _connectedDevice;
  BluetoothCharacteristic? _dataCharacteristic;
  BluetoothCharacteristic? _commandCharacteristic;  // For sending commands
  StreamSubscription<List<int>>? _notificationSubscription;
  StreamSubscription<BluetoothConnectionState>? _connectionStateSubscription;
  StreamSubscription<List<ScanResult>>? _scanSubscription;

  // Connection status stream for UI feedback
  final StreamController<BleConnectionState> _connectionStateController =
      StreamController<BleConnectionState>.broadcast();
  Stream<BleConnectionState> get connectionStateStream =>
      _connectionStateController.stream;

  BleConnectionState _currentState = BleConnectionState.disconnected;
  BleConnectionState get currentState => _currentState;

  bool get isConnected => _currentState == BleConnectionState.connected;

  RealBluetoothService() {
    // Auto-initialize and connect on creation
    _autoConnect();
  }

  /// Auto-connect sequence: init -> scan -> connect
  Future<void> _autoConnect() async {
    final initialized = await init();
    if (initialized) {
      await scanForDevice();
    }
  }

  /// Initialize Bluetooth and request permissions
  Future<bool> init() async {
    _updateState(BleConnectionState.initializing);

    // Request permissions (Android 12+ requires these)
    final bluetoothScan = await Permission.bluetoothScan.request();
    final bluetoothConnect = await Permission.bluetoothConnect.request();
    final location = await Permission.locationWhenInUse.request();

    final allGranted = bluetoothScan.isGranted &&
        bluetoothConnect.isGranted &&
        (location.isGranted || location.isLimited);

    if (!allGranted) {
      print('RealBluetoothService: Permissions denied');
      _updateState(BleConnectionState.permissionDenied);
      return false;
    }

    // Check if Bluetooth is available
    if (await FlutterBluePlus.isSupported == false) {
      print('RealBluetoothService: Bluetooth not supported');
      _updateState(BleConnectionState.notSupported);
      return false;
    }

    // Check if Bluetooth is on
    final adapterState = await FlutterBluePlus.adapterState.first;
    if (adapterState != BluetoothAdapterState.on) {
      print('RealBluetoothService: Bluetooth is off');
      _updateState(BleConnectionState.bluetoothOff);
      // Try to turn on Bluetooth (Android only)
      try {
        await FlutterBluePlus.turnOn();
        await Future.delayed(const Duration(seconds: 2));
      } catch (e) {
        print('RealBluetoothService: Could not turn on Bluetooth: $e');
        return false;
      }
    }

    print('RealBluetoothService: Initialized successfully');
    _updateState(BleConnectionState.ready);
    return true;
  }

  /// Scan for the OxyFeeder device
  Future<bool> scanForDevice() async {
    if (_currentState == BleConnectionState.connected) {
      return true; // Already connected
    }

    _updateState(BleConnectionState.scanning);
    print('RealBluetoothService: Starting scan for $deviceName or service $serviceUuid...');

    try {
      // Stop any existing scan
      await FlutterBluePlus.stopScan();

      BluetoothDevice? targetDevice;

      // Start scanning - no filter, scan all devices
      await FlutterBluePlus.startScan(
        timeout: const Duration(seconds: 20),
        androidScanMode: AndroidScanMode.lowLatency, // Faster scanning
      );

      // Listen for scan results
      final completer = Completer<BluetoothDevice?>();

      _scanSubscription = FlutterBluePlus.scanResults.listen((results) {
        for (final result in results) {
          // Try multiple name sources
          final platformName = result.device.platformName;
          final advName = result.advertisementData.advName;
          final serviceUuids = result.advertisementData.serviceUuids;

          // Debug: print all devices found
          print('RealBluetoothService: Found device - name: "$platformName/$advName", services: $serviceUuids, id: ${result.device.remoteId}');

          // Check if device advertises our service UUID
          final hasServiceUuid = serviceUuids.any((uuid) =>
            uuid.toString().toLowerCase() == serviceUuid.toLowerCase()
          );

          // Check name match
          final names = [platformName, advName];
          final hasNameMatch = names.any((n) =>
            n.toLowerCase() == deviceName.toLowerCase() ||
            n.toLowerCase().contains('oxy')
          );

          if (hasServiceUuid || hasNameMatch) {
            print('RealBluetoothService: MATCH! Found device: ${result.device.remoteId} (serviceMatch: $hasServiceUuid, nameMatch: $hasNameMatch)');
            targetDevice = result.device;
            FlutterBluePlus.stopScan();
            if (!completer.isCompleted) {
              completer.complete(result.device);
            }
            break;
          }
        }
      });

      // Wait for device to be found or timeout
      targetDevice = await completer.future.timeout(
        const Duration(seconds: 15),
        onTimeout: () => null,
      );

      await _scanSubscription?.cancel();
      _scanSubscription = null;

      if (targetDevice == null) {
        print('RealBluetoothService: Device not found');
        _updateState(BleConnectionState.deviceNotFound);
        return false;
      }

      // Connect to the found device
      return await connectToDevice(targetDevice!);
    } catch (e) {
      print('RealBluetoothService: Scan error: $e');
      _updateState(BleConnectionState.error);
      return false;
    }
  }

  /// Connect to a specific Bluetooth device
  Future<bool> connectToDevice(BluetoothDevice device) async {
    _updateState(BleConnectionState.connecting);
    print('RealBluetoothService: Connecting to ${device.remoteId}...');

    try {
      _connectedDevice = device;

      // Connect to device
      await device.connect(
        timeout: const Duration(seconds: 30),
        autoConnect: false,
        license: License.free,
      );
      print('RealBluetoothService: Connected successfully');

      // Longer delay to let connection stabilize
      await Future.delayed(const Duration(seconds: 1));

      // Listen for connection state changes AFTER connection is established
      _connectionStateSubscription = device.connectionState.listen((state) {
        print('RealBluetoothService: Connection state: $state');
        if (state == BluetoothConnectionState.disconnected &&
            _currentState == BleConnectionState.connected) {
          _updateState(BleConnectionState.disconnected);
          _handleDisconnection();
        }
      });

      // Another delay before service discovery
      await Future.delayed(const Duration(milliseconds: 500));

      // Discover services
      print('RealBluetoothService: Discovering services...');
      final services = await device.discoverServices();

      // Debug: print all discovered services
      print('RealBluetoothService: Found ${services.length} services:');
      for (final service in services) {
        print('RealBluetoothService:   Service: ${service.uuid}');
      }

      // Find OxyFeeder service - check if UUID contains our short UUID
      BluetoothService? targetService;
      for (final service in services) {
        final uuid = service.uuid.toString().toLowerCase();
        // Check both full UUID and short UUID (abcd)
        if (uuid == serviceUuid.toLowerCase() || uuid.contains('abcd')) {
          targetService = service;
          print('RealBluetoothService: Found OxyFeeder service: $uuid');
          break;
        }
      }

      if (targetService == null) {
        print('RealBluetoothService: Service not found');
        await device.disconnect();
        _updateState(BleConnectionState.serviceNotFound);
        return false;
      }

      // Find data characteristic and command characteristic
      print('RealBluetoothService: Found ${targetService.characteristics.length} characteristics:');
      for (final characteristic in targetService.characteristics) {
        print('RealBluetoothService:   Characteristic: ${characteristic.uuid}');
        final uuid = characteristic.uuid.toString().toLowerCase();
        // Check for data characteristic (abce)
        if (uuid == characteristicUuid.toLowerCase() || uuid.contains('abce')) {
          _dataCharacteristic = characteristic;
          print('RealBluetoothService: Found data characteristic: $uuid');
        }
        // Check for command characteristic (abcf)
        if (uuid == commandCharacteristicUuid.toLowerCase() || uuid.contains('abcf')) {
          _commandCharacteristic = characteristic;
          print('RealBluetoothService: Found command characteristic: $uuid');
        }
      }

      if (_dataCharacteristic == null) {
        print('RealBluetoothService: Data characteristic not found');
        await device.disconnect();
        _updateState(BleConnectionState.characteristicNotFound);
        return false;
      }

      // Enable notifications and start listening
      await _listenToData();

      _updateState(BleConnectionState.connected);
      print('RealBluetoothService: Setup complete, receiving data...');
      return true;
    } catch (e) {
      print('RealBluetoothService: Connection error: $e');
      _updateState(BleConnectionState.error);
      return false;
    }
  }

  /// Subscribe to characteristic notifications and parse incoming data
  Future<void> _listenToData() async {
    if (_dataCharacteristic == null) return;

    try {
      // Small delay before enabling notifications
      await Future.delayed(const Duration(milliseconds: 500));

      // Enable notifications
      print('RealBluetoothService: Enabling notifications...');
      await _dataCharacteristic!.setNotifyValue(true);
      print('RealBluetoothService: Notifications enabled');

      // Another delay after enabling
      await Future.delayed(const Duration(milliseconds: 500));

      // Listen to incoming data using onValueReceived instead of lastValueStream
      _notificationSubscription =
          _dataCharacteristic!.onValueReceived.listen((data) {
        if (data.isNotEmpty) {
          _parseAndEmitData(data);
        }
      }, onError: (error) {
        print('RealBluetoothService: Notification stream error: $error');
      }, cancelOnError: false);

      print('RealBluetoothService: Listening to data stream');
    } catch (e) {
      print('RealBluetoothService: Error setting up notifications: $e');
    }
  }

  /// Parse JSON data from ESP32 and emit OxyFeederStatus
  void _parseAndEmitData(List<int> data) {
    try {
      final jsonString = utf8.decode(data);
      print('RealBluetoothService: Received: $jsonString');

      final json = jsonDecode(jsonString) as Map<String, dynamic>;
      final status = OxyFeederStatus.fromJson(json);
      _statusStreamController.add(status);
    } catch (e) {
      // Ignore parse errors - might be partial data
      print('RealBluetoothService: Parse error: $e');
    }
  }

  /// Handle disconnection - attempt reconnect
  void _handleDisconnection() {
    print('RealBluetoothService: Handling disconnection...');
    _notificationSubscription?.cancel();
    _notificationSubscription = null;
    _dataCharacteristic = null;

    // Auto-reconnect after delay
    Future.delayed(const Duration(seconds: 3), () {
      if (_currentState == BleConnectionState.disconnected) {
        print('RealBluetoothService: Attempting reconnect...');
        scanForDevice();
      }
    });
  }

  /// Update connection state and notify listeners
  void _updateState(BleConnectionState state) {
    _currentState = state;
    _connectionStateController.add(state);
    print('RealBluetoothService: State changed to ${state.message}');
  }

  /// Manually trigger a reconnection attempt
  Future<void> reconnect() async {
    await disconnect();
    await Future.delayed(const Duration(milliseconds: 500));
    await init();
    await scanForDevice();
  }

  /// Disconnect from the device
  Future<void> disconnect() async {
    print('RealBluetoothService: Disconnecting...');
    await _scanSubscription?.cancel();
    _scanSubscription = null;
    await _notificationSubscription?.cancel();
    _notificationSubscription = null;
    await _connectionStateSubscription?.cancel();
    _connectionStateSubscription = null;
    await _connectedDevice?.disconnect();
    _connectedDevice = null;
    _dataCharacteristic = null;
    _commandCharacteristic = null;
    _updateState(BleConnectionState.disconnected);
  }

  // ==========================================================================
  // COMMAND SENDING METHODS (App -> ESP32 -> Arduino)
  // ==========================================================================

  /// Send a raw command to Arduino via ESP32
  Future<bool> sendCommand(String command) async {
    if (_commandCharacteristic == null) {
      print('RealBluetoothService: Command characteristic not available');
      return false;
    }
    
    try {
      print('RealBluetoothService: Sending command: $command');
      await _commandCharacteristic!.write(
        utf8.encode(command),
        withoutResponse: true,
      );
      print('RealBluetoothService: Command sent successfully');
      return true;
    } catch (e) {
      print('RealBluetoothService: Failed to send command: $e');
      return false;
    }
  }

  /// Set the SMS phone number for alerts
  Future<bool> setPhoneNumber(String phoneNumber) async {
    return sendCommand('PHONE:$phoneNumber');
  }

  /// Trigger a manual feeding
  Future<bool> triggerManualFeed({int durationSeconds = 5}) async {
    return sendCommand('FEED:$durationSeconds');
  }

  /// Send a test SMS to verify the GSM module
  Future<bool> sendTestSms() async {
    return sendCommand('TEST_SMS:1');
  }

  /// Request the current phone number from Arduino
  Future<bool> requestPhoneNumber() async {
    return sendCommand('GET_PHONE:1');
  }

  @override
  void dispose() {
    print('RealBluetoothService: Disposing...');
    disconnect();
    _statusStreamController.close();
    _connectionStateController.close();
  }
}

/// Connection states for UI feedback
enum BleConnectionState {
  disconnected,
  initializing,
  permissionDenied,
  notSupported,
  bluetoothOff,
  ready,
  scanning,
  deviceNotFound,
  connecting,
  connected,
  serviceNotFound,
  characteristicNotFound,
  error,
}

/// Extension for user-friendly state messages
extension BleConnectionStateExtension on BleConnectionState {
  String get message {
    switch (this) {
      case BleConnectionState.disconnected:
        return 'Disconnected';
      case BleConnectionState.initializing:
        return 'Initializing Bluetooth...';
      case BleConnectionState.permissionDenied:
        return 'Bluetooth permission denied';
      case BleConnectionState.notSupported:
        return 'Bluetooth not supported';
      case BleConnectionState.bluetoothOff:
        return 'Please turn on Bluetooth';
      case BleConnectionState.ready:
        return 'Ready to scan';
      case BleConnectionState.scanning:
        return 'Scanning for OxyFeeder...';
      case BleConnectionState.deviceNotFound:
        return 'OxyFeeder not found';
      case BleConnectionState.connecting:
        return 'Connecting...';
      case BleConnectionState.connected:
        return 'Connected to OxyFeeder';
      case BleConnectionState.serviceNotFound:
        return 'Service not found';
      case BleConnectionState.characteristicNotFound:
        return 'Characteristic not found';
      case BleConnectionState.error:
        return 'Connection error';
    }
  }

  bool get isConnected => this == BleConnectionState.connected;
  bool get isScanning => this == BleConnectionState.scanning;
  bool get hasError =>
      this == BleConnectionState.error ||
      this == BleConnectionState.deviceNotFound ||
      this == BleConnectionState.permissionDenied ||
      this == BleConnectionState.serviceNotFound ||
      this == BleConnectionState.characteristicNotFound;
}
