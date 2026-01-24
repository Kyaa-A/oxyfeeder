import 'dart:convert';
import 'package:flutter/foundation.dart';
import 'package:shared_preferences/shared_preferences.dart';
import '../models/event_log.dart';

/// Service to manage event logs with persistence
class EventLogService extends ChangeNotifier {
  static const String _storageKey = 'event_logs';
  static const int _maxLogs = 100; // Keep last 100 events

  List<EventLog> _logs = [];
  SharedPreferences? _prefs;

  List<EventLog> get logs => List.unmodifiable(_logs);
  List<EventLog> get recentLogs => _logs.take(20).toList();

  // Filter by type
  List<EventLog> getLogsByType(EventType type) => 
      _logs.where((l) => l.type == type).toList();

  List<EventLog> get feedingLogs => 
      _logs.where((l) => l.type == EventType.feeding || l.type == EventType.manualFeed).toList();

  List<EventLog> get alertLogs => 
      _logs.where((l) => l.type == EventType.alert || l.type == EventType.warning).toList();

  /// Initialize and load from storage
  Future<void> init() async {
    _prefs = await SharedPreferences.getInstance();
    await _loadLogs();
  }

  Future<void> _loadLogs() async {
    if (_prefs == null) return;

    final jsonStr = _prefs!.getString(_storageKey);
    if (jsonStr == null) return;

    try {
      final List<dynamic> jsonList = jsonDecode(jsonStr);
      _logs = jsonList.map((j) => EventLog.fromJson(j)).toList();
      // Sort by newest first
      _logs.sort((a, b) => b.timestamp.compareTo(a.timestamp));
      notifyListeners();
    } catch (e) {
      print('EventLogService: Failed to load logs: $e');
    }
  }

  Future<void> _saveLogs() async {
    if (_prefs == null) return;

    // Trim to max size
    if (_logs.length > _maxLogs) {
      _logs = _logs.take(_maxLogs).toList();
    }

    final jsonStr = jsonEncode(_logs.map((l) => l.toJson()).toList());
    await _prefs!.setString(_storageKey, jsonStr);
  }

  /// Add a new event log
  Future<void> addLog(EventLog log) async {
    _logs.insert(0, log); // Add to beginning (newest first)
    notifyListeners();
    await _saveLogs();
  }

  /// Add multiple logs at once
  Future<void> addLogs(List<EventLog> logs) async {
    for (final log in logs) {
      _logs.insert(0, log);
    }
    notifyListeners();
    await _saveLogs();
  }

  /// Clear all logs
  Future<void> clearLogs() async {
    _logs.clear();
    notifyListeners();
    await _saveLogs();
  }

  /// Export logs to CSV format
  String exportToCsv() {
    final buffer = StringBuffer();
    buffer.writeln('Timestamp,Type,Title,Description');
    
    for (final log in _logs) {
      final timestamp = log.timestamp.toIso8601String();
      final type = log.type.name;
      final title = _escapeCsv(log.title);
      final desc = _escapeCsv(log.description ?? '');
      buffer.writeln('$timestamp,$type,$title,$desc');
    }
    
    return buffer.toString();
  }

  String _escapeCsv(String value) {
    if (value.contains(',') || value.contains('"') || value.contains('\n')) {
      return '"${value.replaceAll('"', '""')}"';
    }
    return value;
  }

  // Convenience methods for logging common events
  Future<void> logManualFeed() async => addLog(EventLog.manualFeed());
  Future<void> logFeeding({int? duration}) async => addLog(EventLog.feeding(duration: duration));
  Future<void> logLowOxygen(double level) async => addLog(EventLog.lowOxygen(level: level));
  Future<void> logLowBattery(int level) async => addLog(EventLog.lowBattery(level: level));
  Future<void> logLowFeed(int level) async => addLog(EventLog.lowFeed(level: level));
  Future<void> logConnected() async => addLog(EventLog.connected());
  Future<void> logDisconnected() async => addLog(EventLog.disconnected());
  Future<void> logSmsSent(String? recipient) async => addLog(EventLog.smsSent(recipient: recipient));
  Future<void> logSettingsChanged(String setting) async => addLog(EventLog.settingsChanged(setting: setting));
}
