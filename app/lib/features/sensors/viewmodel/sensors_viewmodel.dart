import 'dart:async';
import 'package:flutter/foundation.dart';
import 'package:fl_chart/fl_chart.dart';
import '../../../core/models/oxyfeeder_status.dart';
import '../../../core/services/bluetooth_service_interface.dart';

class SensorsViewModel extends ChangeNotifier {
  final BluetoothServiceInterface _bluetoothService;
  StreamSubscription<OxyFeederStatus>? _statusSubscription;

  OxyFeederStatus _currentStatus = const OxyFeederStatus(
    dissolvedOxygen: 0.0,
    feedLevel: 0,
    batteryStatus: 0,
  );

  final List<FlSpot> _historicalDoData = <FlSpot>[];

  // Timestamp of last update
  DateTime _lastUpdate = DateTime.now();

  SensorsViewModel(this._bluetoothService) {
    _statusSubscription = _bluetoothService.statusStream.listen((event) {
      updateLiveData(event);
      addHistoricalDoData(event.dissolvedOxygen);
    });
  }

  // Getters
  OxyFeederStatus get currentStatus => _currentStatus;
  List<FlSpot> get historicalDoData => List<FlSpot>.unmodifiable(_historicalDoData);
  DateTime get lastUpdate => _lastUpdate;
  
  /// Calculated battery voltage from percentage (12V nominal, 10.5V cutoff, 14.4V full)
  String get batteryVoltage {
    // Map 0-100% to 10.5V - 14.4V range
    final voltage = 10.5 + (_currentStatus.batteryStatus / 100.0) * (14.4 - 10.5);
    return '${voltage.toStringAsFixed(1)}V';
  }
  
  /// Feed level raw value (simulated from percentage)
  String get feedLoadCellRawValue {
    // Simulate raw ADC value based on percentage (0-100% -> 0-4095)
    final rawValue = (_currentStatus.feedLevel / 100.0 * 4095).toInt();
    return rawValue.toString();
  }

  /// Check if we're receiving live data
  bool get isReceivingData {
    return DateTime.now().difference(_lastUpdate).inSeconds < 10;
  }

  void updateLiveData(OxyFeederStatus newStatus) {
    _currentStatus = newStatus;
    _lastUpdate = DateTime.now();
    notifyListeners();
  }

  void addHistoricalDoData(double doValue) {
    // Append new Y value
    final List<double> ys = _historicalDoData.map((e) => e.y).toList()..add(doValue);
    // Keep sliding window of last 24 values
    const int window = 24;
    if (ys.length > window) {
      ys.removeRange(0, ys.length - window);
    }
    // Reindex X consecutively so the chart spans the full width
    _historicalDoData
      ..clear()
      ..addAll(List<FlSpot>.generate(ys.length, (i) => FlSpot(i.toDouble(), ys[i])));
    notifyListeners();
  }

  @override
  void dispose() {
    _statusSubscription?.cancel();
    super.dispose();
  }
}
