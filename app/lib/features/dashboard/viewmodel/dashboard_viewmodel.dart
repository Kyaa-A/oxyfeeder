import 'dart:async';
import 'package:flutter/foundation.dart';
import '../../../core/models/oxyfeeder_status.dart';
import '../../../core/services/bluetooth_service_interface.dart';
import '../../../core/services/real_bluetooth_service.dart';
import '../../../core/services/event_log_service.dart';
import '../../settings/viewmodel/settings_viewmodel.dart';

class DashboardViewModel extends ChangeNotifier {
  final BluetoothServiceInterface _bluetoothService;
  EventLogService? _eventLogService;
  SettingsViewModel? _settingsViewModel;
  StreamSubscription<OxyFeederStatus>? _statusSubscription;
  StreamSubscription<BleConnectionState>? _connectionSubscription;

  OxyFeederStatus _status = const OxyFeederStatus(
    dissolvedOxygen: 0.0,
    feedLevel: 0,
    batteryStatus: 0,
  );

  // Track alert state to avoid spamming logs
  bool _lowOxygenAlerted = false;
  bool _lowFeedAlerted = false;
  bool _lowBatteryAlerted = false;

  // Active alerts exposed to UI
  List<String> _activeAlerts = [];
  List<String> get activeAlerts => _activeAlerts;

  DashboardViewModel(this._bluetoothService) {
    _statusSubscription = _bluetoothService.statusStream.listen((event) {
      updateStatus(event);
    });
  }

  void setEventLogService(EventLogService service) {
    _eventLogService = service;
  }

  void listenToConnectionState(RealBluetoothService bleService) {
    bool wasConnected = false;
    _connectionSubscription = bleService.connectionStateStream.listen((state) {
      if (state == BleConnectionState.connected && !wasConnected) {
        wasConnected = true;
        _eventLogService?.logConnected();
        // Sync time + schedules to ESP32 on connect
        _settingsViewModel?.syncAllToDevice();
      } else if (state == BleConnectionState.disconnected && wasConnected) {
        wasConnected = false;
        _eventLogService?.logDisconnected();
      }
    });
  }

  void setSettingsViewModel(SettingsViewModel vm) {
    _settingsViewModel = vm;
  }

  OxyFeederStatus get status => _status;

  void updateStatus(OxyFeederStatus newStatus) {
    _status = newStatus;
    _checkThresholds(newStatus);
    notifyListeners();
  }

  void _checkThresholds(OxyFeederStatus status) {
    if (_eventLogService == null || _settingsViewModel == null) return;

    // Skip alerts if notifications are disabled in settings
    if (!_settingsViewModel!.notificationsEnabled) {
      _activeAlerts = [];
      _lowOxygenAlerted = false;
      _lowFeedAlerted = false;
      _lowBatteryAlerted = false;
      return;
    }

    final minDO = _settingsViewModel!.minDissolvedOxygen;
    final lowFeed = _settingsViewModel!.lowFeedThreshold;
    final lowBattery = _settingsViewModel!.lowBatteryThreshold;
    final alerts = <String>[];

    // Check DO threshold
    if (status.dissolvedOxygen > 0 && status.dissolvedOxygen < minDO) {
      alerts.add('Low Dissolved Oxygen: ${status.dissolvedOxygen.toStringAsFixed(1)} mg/L');
      if (!_lowOxygenAlerted) {
        _lowOxygenAlerted = true;
        _eventLogService!.logLowOxygen(status.dissolvedOxygen);
      }
    } else {
      _lowOxygenAlerted = false;
    }

    // Check feed level threshold
    if (status.feedLevel < lowFeed && status.feedLevel >= 0) {
      alerts.add('Low Feed Level: ${status.feedLevel}%');
      if (!_lowFeedAlerted) {
        _lowFeedAlerted = true;
        _eventLogService!.logLowFeed(status.feedLevel);
      }
    } else {
      _lowFeedAlerted = false;
    }

    // Check battery threshold
    if (status.batteryStatus > 0 && status.batteryStatus < lowBattery) {
      alerts.add('Low Battery: ${status.batteryStatus}%');
      if (!_lowBatteryAlerted) {
        _lowBatteryAlerted = true;
        _eventLogService!.logLowBattery(status.batteryStatus);
      }
    } else {
      _lowBatteryAlerted = false;
    }

    _activeAlerts = alerts;
  }

  @override
  void dispose() {
    _statusSubscription?.cancel();
    _connectionSubscription?.cancel();
    super.dispose();
  }
}
