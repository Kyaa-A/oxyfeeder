import 'dart:async';
import 'dart:math';
import '../../core/models/oxyfeeder_status.dart';
import 'bluetooth_service_interface.dart';

class MockBluetoothService implements BluetoothServiceInterface {
  final StreamController<OxyFeederStatus> _statusStreamController = StreamController<OxyFeederStatus>.broadcast();

  @override
  Stream<OxyFeederStatus> get statusStream => _statusStreamController.stream;

  Timer? _timer;

  MockBluetoothService() {
    _timer = Timer.periodic(const Duration(seconds: 3), (_) {
      final random = Random();
      final double dissolvedOxygen = 5.0 + random.nextDouble() * (8.5 - 5.0);
      final int feedLevel = random.nextInt(101);
      final int batteryStatus = random.nextInt(101);

      final status = OxyFeederStatus(
        dissolvedOxygen: dissolvedOxygen,
        feedLevel: feedLevel,
        batteryStatus: batteryStatus,
      );
      _statusStreamController.add(status);
    });
  }

  @override
  void dispose() {
    _timer?.cancel();
    _statusStreamController.close();
  }
}


