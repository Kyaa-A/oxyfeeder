class FeedingSchedule {
  final String timeLabel; // e.g., "08:00 AM"
  final int durationSeconds; // e.g., 10
  final bool enabled;

  const FeedingSchedule({
    required this.timeLabel,
    required this.durationSeconds,
    required this.enabled,
  });

  FeedingSchedule copyWith({String? timeLabel, int? durationSeconds, bool? enabled}) {
    return FeedingSchedule(
      timeLabel: timeLabel ?? this.timeLabel,
      durationSeconds: durationSeconds ?? this.durationSeconds,
      enabled: enabled ?? this.enabled,
    );
  }
}

class AppSettings {
  final List<FeedingSchedule> feedingSchedules;
  final double minDissolvedOxygen; // mg/L
  final int lowFeedThreshold; // %
  final int lowBatteryThreshold; // %
  final bool notificationsEnabled;

  const AppSettings({
    required this.feedingSchedules,
    required this.minDissolvedOxygen,
    required this.lowFeedThreshold,
    required this.lowBatteryThreshold,
    required this.notificationsEnabled,
  });

  factory AppSettings.defaults() => const AppSettings(
        feedingSchedules: [],
        minDissolvedOxygen: 4.5,
        lowFeedThreshold: 20,
        lowBatteryThreshold: 25,
        notificationsEnabled: true,
      );

  AppSettings copyWith({
    List<FeedingSchedule>? feedingSchedules,
    double? minDissolvedOxygen,
    int? lowFeedThreshold,
    int? lowBatteryThreshold,
    bool? notificationsEnabled,
  }) {
    return AppSettings(
      feedingSchedules: feedingSchedules ?? this.feedingSchedules,
      minDissolvedOxygen: minDissolvedOxygen ?? this.minDissolvedOxygen,
      lowFeedThreshold: lowFeedThreshold ?? this.lowFeedThreshold,
      lowBatteryThreshold: lowBatteryThreshold ?? this.lowBatteryThreshold,
      notificationsEnabled: notificationsEnabled ?? this.notificationsEnabled,
    );
  }
}


