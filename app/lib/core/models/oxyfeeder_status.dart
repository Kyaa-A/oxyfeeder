class OxyFeederStatus {
  final double dissolvedOxygen; // mg/L
  final int feedLevel; // percentage 0-100
  final int batteryStatus; // percentage 0-100

  const OxyFeederStatus({
    required this.dissolvedOxygen,
    required this.feedLevel,
    required this.batteryStatus,
  });

  OxyFeederStatus copyWith({
    double? dissolvedOxygen,
    int? feedLevel,
    int? batteryStatus,
  }) {
    return OxyFeederStatus(
      dissolvedOxygen: dissolvedOxygen ?? this.dissolvedOxygen,
      feedLevel: feedLevel ?? this.feedLevel,
      batteryStatus: batteryStatus ?? this.batteryStatus,
    );
  }

  factory OxyFeederStatus.fromJson(Map<String, dynamic> json) {
    // Safely parse JSON with null checks and fallback defaults
    // This prevents crashes if ESP32 sends malformed/incomplete data
    return OxyFeederStatus(
      dissolvedOxygen: (json['do'] as num?)?.toDouble() ?? 0.0,
      feedLevel: (json['feed'] as num?)?.toInt() ?? 0,
      batteryStatus: (json['battery'] as num?)?.toInt() ?? 0,
    );
  }
}


