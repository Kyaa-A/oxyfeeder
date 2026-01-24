import 'dart:convert';
import 'package:flutter/foundation.dart';
import 'package:shared_preferences/shared_preferences.dart';
import '../../../core/models/app_settings.dart';
import '../../../core/services/real_bluetooth_service.dart';
import '../../../core/services/event_log_service.dart';

class SettingsViewModel extends ChangeNotifier {
  AppSettings _appSettings = AppSettings.defaults();
  RealBluetoothService? _bluetoothService;
  EventLogService? _eventLogService;
  SharedPreferences? _prefs;

  // SMS Phone Number (stored locally, sent to device when connected)
  String _smsPhoneNumber = '';
  bool _isLoading = true;

  // Getters
  List<FeedingSchedule> get feedingSchedules => _appSettings.feedingSchedules;
  double get minDissolvedOxygen => _appSettings.minDissolvedOxygen;
  int get lowFeedThreshold => _appSettings.lowFeedThreshold;
  int get lowBatteryThreshold => _appSettings.lowBatteryThreshold;
  bool get notificationsEnabled => _appSettings.notificationsEnabled;
  String get smsPhoneNumber => _smsPhoneNumber;
  bool get isLoading => _isLoading;

  // Initialize - call this after construction
  Future<void> init() async {
    _prefs = await SharedPreferences.getInstance();
    await _loadSettings();
    _isLoading = false;
    notifyListeners();
  }

  // Set the bluetooth service reference (called from outside)
  void setBluetoothService(RealBluetoothService service) {
    _bluetoothService = service;
    // Request current phone number from device when connected
    _requestPhoneNumberFromDevice();
  }

  // Set the event log service reference
  void setEventLogService(EventLogService service) {
    _eventLogService = service;
  }

  // ============================================================================
  // Persistence - Load/Save Settings
  // ============================================================================

  Future<void> _loadSettings() async {
    if (_prefs == null) return;

    // Load phone number
    _smsPhoneNumber = _prefs!.getString('sms_phone_number') ?? '';

    // Load thresholds
    final minDO = _prefs!.getDouble('min_dissolved_oxygen');
    final lowFeed = _prefs!.getInt('low_feed_threshold');
    final lowBattery = _prefs!.getInt('low_battery_threshold');
    final notifications = _prefs!.getBool('notifications_enabled');

    // Load schedules from JSON
    final schedulesJson = _prefs!.getString('feeding_schedules');
    List<FeedingSchedule> schedules = [];
    if (schedulesJson != null) {
      try {
        final List<dynamic> decoded = jsonDecode(schedulesJson);
        schedules = decoded.map((e) => FeedingSchedule(
          timeLabel: e['timeLabel'],
          durationSeconds: e['durationSeconds'],
          enabled: e['enabled'],
        )).toList();
      } catch (e) {
        print('Failed to load schedules: $e');
      }
    }

    _appSettings = AppSettings(
      feedingSchedules: schedules.isNotEmpty ? schedules : _appSettings.feedingSchedules,
      minDissolvedOxygen: minDO ?? _appSettings.minDissolvedOxygen,
      lowFeedThreshold: lowFeed ?? _appSettings.lowFeedThreshold,
      lowBatteryThreshold: lowBattery ?? _appSettings.lowBatteryThreshold,
      notificationsEnabled: notifications ?? _appSettings.notificationsEnabled,
    );

    print('SettingsViewModel: Loaded settings from storage');
  }

  Future<void> _saveSettings() async {
    if (_prefs == null) return;

    await _prefs!.setString('sms_phone_number', _smsPhoneNumber);
    await _prefs!.setDouble('min_dissolved_oxygen', _appSettings.minDissolvedOxygen);
    await _prefs!.setInt('low_feed_threshold', _appSettings.lowFeedThreshold);
    await _prefs!.setInt('low_battery_threshold', _appSettings.lowBatteryThreshold);
    await _prefs!.setBool('notifications_enabled', _appSettings.notificationsEnabled);

    // Save schedules as JSON
    final schedulesJson = jsonEncode(_appSettings.feedingSchedules.map((s) => {
      'timeLabel': s.timeLabel,
      'durationSeconds': s.durationSeconds,
      'enabled': s.enabled,
    }).toList());
    await _prefs!.setString('feeding_schedules', schedulesJson);

    print('SettingsViewModel: Saved settings to storage');
  }

  // ============================================================================
  // Updaters (with auto-save)
  // ============================================================================

  void updateMinDissolvedOxygen(double value) {
    _appSettings = _appSettings.copyWith(minDissolvedOxygen: value);
    notifyListeners();
    _saveSettings();
  }

  void updateLowFeedThreshold(int value) {
    _appSettings = _appSettings.copyWith(lowFeedThreshold: value);
    notifyListeners();
    _saveSettings();
  }

  void updateLowBatteryThreshold(int value) {
    _appSettings = _appSettings.copyWith(lowBatteryThreshold: value);
    notifyListeners();
    _saveSettings();
  }

  void updateNotificationsEnabled(bool enabled) {
    _appSettings = _appSettings.copyWith(notificationsEnabled: enabled);
    notifyListeners();
    _saveSettings();
  }

  void toggleScheduleEnabled(int index, bool enabled) {
    final schedules = List<FeedingSchedule>.from(_appSettings.feedingSchedules);
    final item = schedules[index].copyWith(enabled: enabled);
    schedules[index] = item;
    _appSettings = _appSettings.copyWith(feedingSchedules: schedules);
    notifyListeners();
    _saveSettings();
    _syncScheduleToDevice(item);
  }

  void addSchedule({required String timeLabel, required int durationSeconds, bool enabled = true}) {
    final schedule = FeedingSchedule(timeLabel: timeLabel, durationSeconds: durationSeconds, enabled: enabled);
    final schedules = List<FeedingSchedule>.from(_appSettings.feedingSchedules)..add(schedule);
    _appSettings = _appSettings.copyWith(feedingSchedules: schedules);
    notifyListeners();
    _saveSettings();
    _syncScheduleToDevice(schedule);
  }

  void removeSchedule(int index) {
    final schedules = List<FeedingSchedule>.from(_appSettings.feedingSchedules)..removeAt(index);
    _appSettings = _appSettings.copyWith(feedingSchedules: schedules);
    notifyListeners();
    _saveSettings();
    _syncAllSchedulesToDevice();
  }

  // ============================================================================
  // SMS Phone Number Methods
  // ============================================================================

  void updateSmsPhoneNumber(String phoneNumber) {
    _smsPhoneNumber = phoneNumber.trim();
    notifyListeners();
    _saveSettings();
  }

  Future<bool> sendPhoneNumberToDevice() async {
    if (_bluetoothService == null) {
      print('SettingsViewModel: Bluetooth service not available');
      return false;
    }
    
    final success = await _bluetoothService!.setPhoneNumber(_smsPhoneNumber);
    if (success) {
      print('SettingsViewModel: Phone number sent to device');
    } else {
      print('SettingsViewModel: Failed to send phone number');
    }
    return success;
  }

  Future<bool> sendTestSms() async {
    if (_bluetoothService == null) {
      print('SettingsViewModel: Bluetooth service not available');
      return false;
    }
    
    final success = await _bluetoothService!.sendTestSms();
    if (success) {
      print('SettingsViewModel: Test SMS command sent');
      _eventLogService?.logSmsSent(_smsPhoneNumber.isNotEmpty ? _smsPhoneNumber : null);
    } else {
      print('SettingsViewModel: Failed to send test SMS command');
    }
    return success;
  }

  Future<bool> triggerManualFeed({int durationSeconds = 5}) async {
    if (_bluetoothService == null) {
      print('SettingsViewModel: Bluetooth service not available');
      return false;
    }
    
    final success = await _bluetoothService!.triggerManualFeed(durationSeconds: durationSeconds);
    if (success) {
      print('SettingsViewModel: Manual feed command sent');
      _eventLogService?.logManualFeed();
    } else {
      print('SettingsViewModel: Failed to send manual feed command');
    }
    return success;
  }


  // ============================================================================
  // Sync with Arduino Device
  // ============================================================================

  Future<void> _requestPhoneNumberFromDevice() async {
    if (_bluetoothService == null) return;
    
    // Try to get current phone number from device
    final success = await _bluetoothService!.requestPhoneNumber();
    if (success) {
      print('SettingsViewModel: Requested phone number from device');
      // Note: The response will come via a notification if we add that feature
    }
  }

  Future<void> _syncScheduleToDevice(FeedingSchedule schedule) async {
    if (_bluetoothService == null) return;
    
    // Send schedule to Arduino: SCHEDULE:HH:MM,duration,enabled
    final command = 'SCHEDULE:${schedule.timeLabel},${schedule.durationSeconds},${schedule.enabled ? 1 : 0}';
    final success = await _bluetoothService!.sendCommand(command);
    if (success) {
      print('SettingsViewModel: Synced schedule to device: $command');
    }
  }

  Future<void> _syncAllSchedulesToDevice() async {
    if (_bluetoothService == null) return;
    
    // Clear schedules on device first
    await _bluetoothService!.sendCommand('CLEAR_SCHEDULES:');
    
    // Send all schedules
    for (final schedule in _appSettings.feedingSchedules) {
      await _syncScheduleToDevice(schedule);
      await Future.delayed(const Duration(milliseconds: 100)); // Small delay between commands
    }
    print('SettingsViewModel: Synced all ${_appSettings.feedingSchedules.length} schedules to device');
  }

  /// Sync all settings to device (call after connecting)
  Future<void> syncAllToDevice() async {
    if (_bluetoothService == null) return;
    
    // Sync phone number
    if (_smsPhoneNumber.isNotEmpty) {
      await sendPhoneNumberToDevice();
    }
    
    // Sync all schedules
    await _syncAllSchedulesToDevice();
    
    print('SettingsViewModel: Full sync complete');
  }
}
