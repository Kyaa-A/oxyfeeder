import 'dart:async';
import 'package:flutter/foundation.dart';
import '../../../core/models/oxyfeeder_status.dart';
import '../../../core/services/bluetooth_service_interface.dart';

class DashboardViewModel extends ChangeNotifier {
  final BluetoothServiceInterface _bluetoothService;
  StreamSubscription<OxyFeederStatus>? _statusSubscription;

  OxyFeederStatus _status = const OxyFeederStatus(
    dissolvedOxygen: 0.0,
    feedLevel: 0,
    batteryStatus: 0,
  );

  DashboardViewModel(this._bluetoothService) {
    _statusSubscription = _bluetoothService.statusStream.listen((event) {
      updateStatus(event);
    });
  }

  OxyFeederStatus get status => _status;

  void updateStatus(OxyFeederStatus newStatus) {
    _status = newStatus;
    notifyListeners();
  }

  @override
  void dispose() {
    _statusSubscription?.cancel();
    super.dispose();
  }
}


