import '../models/oxyfeeder_status.dart';

/// Interface for Bluetooth services
///
/// Both MockBluetoothService and RealBluetoothService implement this interface
/// so ViewModels can work with either one.
abstract class BluetoothServiceInterface {
  /// Stream of OxyFeederStatus updates
  Stream<OxyFeederStatus> get statusStream;

  /// Dispose of resources
  void dispose();
}
