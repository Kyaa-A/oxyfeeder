import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../../../core/services/event_log_service.dart';
import '../../../core/models/event_log.dart';

class HistoryScreen extends StatelessWidget {
  const HistoryScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      extendBodyBehindAppBar: true,
      appBar: AppBar(
        title: const Text('EVENT HISTORY'),
        centerTitle: true,
        backgroundColor: Colors.transparent,
        elevation: 0,
        titleTextStyle: const TextStyle(
          fontFamily: 'RobotoMono',
          fontWeight: FontWeight.bold,
          fontSize: 16,
          letterSpacing: 1.5,
          color: Colors.white,
        ),
        actions: [
          PopupMenuButton<String>(
            icon: const Icon(Icons.more_vert, color: Colors.white),
            color: const Color(0xFF1E293B),
            onSelected: (value) async {
              final logService = context.read<EventLogService>();
              if (value == 'export') {
                final csv = logService.exportToCsv();
                _showExportDialog(context, csv);
              } else if (value == 'clear') {
                final confirm = await _showConfirmClearDialog(context);
                if (confirm == true) {
                  logService.clearLogs();
                  ScaffoldMessenger.of(context).showSnackBar(
                    const SnackBar(content: Text('Event history cleared')),
                  );
                }
              }
            },
            itemBuilder: (context) => [
              const PopupMenuItem(
                value: 'export',
                child: Row(
                  children: [
                    Icon(Icons.download, color: Colors.white70, size: 18),
                    SizedBox(width: 12),
                    Text('Export CSV', style: TextStyle(color: Colors.white)),
                  ],
                ),
              ),
              const PopupMenuItem(
                value: 'clear',
                child: Row(
                  children: [
                    Icon(Icons.delete_outline, color: Colors.redAccent, size: 18),
                    SizedBox(width: 12),
                    Text('Clear All', style: TextStyle(color: Colors.redAccent)),
                  ],
                ),
              ),
            ],
          ),
        ],
      ),
      body: Container(
        decoration: const BoxDecoration(
          gradient: LinearGradient(
            begin: Alignment.topCenter,
            end: Alignment.bottomCenter,
            colors: [
              Color(0xFF0F172A),
              Color(0xFF020617),
            ],
          ),
        ),
        child: SafeArea(
          child: Consumer<EventLogService>(
            builder: (context, logService, _) {
              final logs = logService.logs;

              if (logs.isEmpty) {
                return Center(
                  child: Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      Icon(Icons.history, size: 64, color: Colors.white.withOpacity(0.2)),
                      const SizedBox(height: 16),
                      Text(
                        'No events recorded yet',
                        style: TextStyle(color: Colors.white.withOpacity(0.4), fontSize: 16),
                      ),
                      const SizedBox(height: 8),
                      Text(
                        'Events will appear here as they occur',
                        style: TextStyle(color: Colors.white.withOpacity(0.3), fontSize: 12),
                      ),
                    ],
                  ),
                );
              }

              return ListView.builder(
                padding: const EdgeInsets.all(16),
                itemCount: logs.length,
                itemBuilder: (context, index) {
                  final log = logs[index];
                  return _EventLogCard(log: log);
                },
              );
            },
          ),
        ),
      ),
    );
  }

  void _showExportDialog(BuildContext context, String csv) {
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        backgroundColor: const Color(0xFF1E293B),
        title: const Text('Export Data', style: TextStyle(color: Colors.white)),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              'CSV data ready to copy:',
              style: TextStyle(color: Colors.white.withOpacity(0.7)),
            ),
            const SizedBox(height: 12),
            Container(
              height: 200,
              padding: const EdgeInsets.all(8),
              decoration: BoxDecoration(
                color: Colors.black.withOpacity(0.3),
                borderRadius: BorderRadius.circular(8),
              ),
              child: SingleChildScrollView(
                child: SelectableText(
                  csv,
                  style: const TextStyle(
                    fontFamily: 'RobotoMono',
                    fontSize: 10,
                    color: Colors.white70,
                  ),
                ),
              ),
            ),
          ],
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text('CLOSE', style: TextStyle(color: Color(0xFF14B8A6))),
          ),
        ],
      ),
    );
  }

  Future<bool?> _showConfirmClearDialog(BuildContext context) {
    return showDialog<bool>(
      context: context,
      builder: (context) => AlertDialog(
        backgroundColor: const Color(0xFF1E293B),
        title: const Text('Clear History?', style: TextStyle(color: Colors.white)),
        content: Text(
          'This will delete all event logs. This action cannot be undone.',
          style: TextStyle(color: Colors.white.withOpacity(0.7)),
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context, false),
            child: const Text('CANCEL', style: TextStyle(color: Colors.white54)),
          ),
          TextButton(
            onPressed: () => Navigator.pop(context, true),
            child: const Text('CLEAR', style: TextStyle(color: Colors.redAccent)),
          ),
        ],
      ),
    );
  }
}

class _EventLogCard extends StatelessWidget {
  final EventLog log;

  const _EventLogCard({required this.log});

  @override
  Widget build(BuildContext context) {
    final iconData = _getIconForType(log.type);
    final color = _getColorForType(log.type);
    final timeStr = _formatTime(log.timestamp);
    final dateStr = _formatDate(log.timestamp);

    return Container(
      margin: const EdgeInsets.only(bottom: 8),
      padding: const EdgeInsets.all(12),
      decoration: BoxDecoration(
        color: const Color(0xFF1E293B),
        borderRadius: BorderRadius.circular(10),
        border: Border.all(color: Colors.white.withOpacity(0.05)),
      ),
      child: Row(
        children: [
          Container(
            padding: const EdgeInsets.all(8),
            decoration: BoxDecoration(
              color: color.withOpacity(0.15),
              borderRadius: BorderRadius.circular(8),
            ),
            child: Icon(iconData, color: color, size: 18),
          ),
          const SizedBox(width: 12),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  log.title,
                  style: const TextStyle(
                    color: Colors.white,
                    fontWeight: FontWeight.w600,
                    fontSize: 13,
                  ),
                ),
                if (log.description != null)
                  Text(
                    log.description!,
                    style: TextStyle(
                      color: Colors.white.withOpacity(0.5),
                      fontSize: 11,
                    ),
                  ),
              ],
            ),
          ),
          Column(
            crossAxisAlignment: CrossAxisAlignment.end,
            children: [
              Text(
                timeStr,
                style: TextStyle(
                  color: Colors.white.withOpacity(0.4),
                  fontSize: 11,
                  fontFamily: 'RobotoMono',
                ),
              ),
              Text(
                dateStr,
                style: TextStyle(
                  color: Colors.white.withOpacity(0.3),
                  fontSize: 9,
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }

  IconData _getIconForType(EventType type) {
    switch (type) {
      case EventType.feeding:
        return Icons.set_meal;
      case EventType.manualFeed:
        return Icons.touch_app;
      case EventType.alert:
        return Icons.warning_amber;
      case EventType.warning:
        return Icons.info_outline;
      case EventType.connection:
        return Icons.bluetooth;
      case EventType.notification:
        return Icons.sms;
      case EventType.settings:
        return Icons.settings;
      case EventType.system:
        return Icons.memory;
    }
  }

  Color _getColorForType(EventType type) {
    switch (type) {
      case EventType.feeding:
        return const Color(0xFF10B981); // Green
      case EventType.manualFeed:
        return const Color(0xFFF59E0B); // Amber
      case EventType.alert:
        return const Color(0xFFEF4444); // Red
      case EventType.warning:
        return const Color(0xFFF59E0B); // Amber
      case EventType.connection:
        return const Color(0xFF14B8A6); // Teal
      case EventType.notification:
        return const Color(0xFF6366F1); // Indigo
      case EventType.settings:
        return const Color(0xFF8B5CF6); // Purple
      case EventType.system:
        return const Color(0xFF64748B); // Slate
    }
  }

  String _formatTime(DateTime dt) {
    final hour = dt.hour.toString().padLeft(2, '0');
    final min = dt.minute.toString().padLeft(2, '0');
    return '$hour:$min';
  }

  String _formatDate(DateTime dt) {
    final now = DateTime.now();
    final today = DateTime(now.year, now.month, now.day);
    final date = DateTime(dt.year, dt.month, dt.day);

    if (date == today) return 'Today';
    if (date == today.subtract(const Duration(days: 1))) return 'Yesterday';
    
    return '${dt.month}/${dt.day}';
  }
}
