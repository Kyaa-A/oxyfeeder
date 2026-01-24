/// Represents an event in the OxyFeeder system
class EventLog {
  final DateTime timestamp;
  final EventType type;
  final String title;
  final String? description;
  final Map<String, dynamic>? data;

  const EventLog({
    required this.timestamp,
    required this.type,
    required this.title,
    this.description,
    this.data,
  });

  Map<String, dynamic> toJson() => {
    'timestamp': timestamp.toIso8601String(),
    'type': type.name,
    'title': title,
    'description': description,
    'data': data,
  };

  factory EventLog.fromJson(Map<String, dynamic> json) => EventLog(
    timestamp: DateTime.parse(json['timestamp']),
    type: EventType.values.firstWhere((e) => e.name == json['type']),
    title: json['title'],
    description: json['description'],
    data: json['data'],
  );

  // Factory constructors for common events
  factory EventLog.feeding({DateTime? time, int? duration}) => EventLog(
    timestamp: time ?? DateTime.now(),
    type: EventType.feeding,
    title: 'Feeding Completed',
    description: duration != null ? 'Duration: ${duration}s' : null,
    data: {'duration': duration},
  );

  factory EventLog.manualFeed({DateTime? time}) => EventLog(
    timestamp: time ?? DateTime.now(),
    type: EventType.manualFeed,
    title: 'Manual Feed Triggered',
    description: 'Feed command sent from app',
  );

  factory EventLog.lowOxygen({DateTime? time, double? level}) => EventLog(
    timestamp: time ?? DateTime.now(),
    type: EventType.alert,
    title: 'Low Oxygen Alert',
    description: level != null ? 'Level: ${level.toStringAsFixed(1)} mg/L' : 'Critical dissolved oxygen detected',
    data: {'level': level},
  );

  factory EventLog.lowBattery({DateTime? time, int? level}) => EventLog(
    timestamp: time ?? DateTime.now(),
    type: EventType.warning,
    title: 'Low Battery Warning',
    description: level != null ? 'Level: $level%' : 'Battery running low',
    data: {'level': level},
  );

  factory EventLog.lowFeed({DateTime? time, int? level}) => EventLog(
    timestamp: time ?? DateTime.now(),
    type: EventType.warning,
    title: 'Low Feed Warning',
    description: level != null ? 'Level: $level%' : 'Feed hopper running low',
    data: {'level': level},
  );

  factory EventLog.connected({DateTime? time}) => EventLog(
    timestamp: time ?? DateTime.now(),
    type: EventType.connection,
    title: 'Device Connected',
    description: 'OxyFeeder device connected via Bluetooth',
  );

  factory EventLog.disconnected({DateTime? time}) => EventLog(
    timestamp: time ?? DateTime.now(),
    type: EventType.connection,
    title: 'Device Disconnected',
    description: 'Lost connection to OxyFeeder device',
  );

  factory EventLog.smsSent({DateTime? time, String? recipient}) => EventLog(
    timestamp: time ?? DateTime.now(),
    type: EventType.notification,
    title: 'SMS Alert Sent',
    description: recipient != null ? 'Sent to: $recipient' : 'Alert SMS dispatched',
    data: {'recipient': recipient},
  );

  factory EventLog.settingsChanged({DateTime? time, String? setting}) => EventLog(
    timestamp: time ?? DateTime.now(),
    type: EventType.settings,
    title: 'Settings Updated',
    description: setting ?? 'Configuration changed',
  );
}

enum EventType {
  feeding,
  manualFeed,
  alert,
  warning,
  connection,
  notification,
  settings,
  system,
}
