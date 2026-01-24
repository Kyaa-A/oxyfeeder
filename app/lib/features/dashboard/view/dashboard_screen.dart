import 'package:flutter/material.dart';
import '../widgets/sensor_card.dart';
import 'package:provider/provider.dart';
import '../viewmodel/dashboard_viewmodel.dart';
import '../../settings/viewmodel/settings_viewmodel.dart';
import '../../../core/services/real_bluetooth_service.dart';

class DashboardScreen extends StatefulWidget {
  final VoidCallback? onNavigateToSensors;

  const DashboardScreen({super.key, this.onNavigateToSensors});

  @override
  State<DashboardScreen> createState() => _DashboardScreenState();
}

class _DashboardScreenState extends State<DashboardScreen> {
  bool _isFeeding = false;

  Future<void> _handleFeedNow(SettingsViewModel vm) async {
    setState(() => _isFeeding = true);
    final success = await vm.triggerManualFeed();
    setState(() => _isFeeding = false);
    
    if (mounted) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Row(
            children: [
              Icon(
                success ? Icons.check_circle : Icons.error,
                color: Colors.white,
                size: 20,
              ),
              const SizedBox(width: 12),
              Text(success ? 'Feed command sent!' : 'Failed to send. Check connection.'),
            ],
          ),
          backgroundColor: success ? const Color(0xFF10B981) : const Color(0xFFEF4444),
          behavior: SnackBarBehavior.floating,
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
          margin: const EdgeInsets.all(16),
          duration: const Duration(seconds: 2),
        ),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    final vm = context.watch<DashboardViewModel>();
    final settingsVm = context.read<SettingsViewModel>();
    final bleService = context.read<RealBluetoothService>();
    final status = vm.status;

    // Helper to determine status color
    Color getDoColor(double val) {
      if (val < 4.0) return const Color(0xFFEF4444); // Red-500
      if (val < 6.0) return const Color(0xFFF59E0B); // Amber-500
      return const Color(0xFF10B981); // Emerald-500
    }

    Color getLevelColor(int val) {
      if (val < 20) return const Color(0xFFEF4444);
      if (val < 50) return const Color(0xFFF59E0B);
      return const Color(0xFF10B981);
    }
    
    // Status text helpers
    String getDoStatus(double val) {
      if (val < 4.0) return 'CRITICAL';
      if (val < 6.0) return 'WARNING';
      return 'OPTIMAL';
    }

    return Scaffold(
      extendBodyBehindAppBar: true,
      appBar: AppBar(
        title: const Text('OxyFeeder'),
        centerTitle: false,
        backgroundColor: Colors.transparent,
        elevation: 0,
        actions: [
          // Connection status from RealBluetoothService
          StreamBuilder<BleConnectionState>(
            stream: bleService.connectionStateStream,
            initialData: bleService.currentState,
            builder: (context, snapshot) {
              final state = snapshot.data ?? BleConnectionState.disconnected;
              final isConnected = state.isConnected;
              final color = isConnected ? const Color(0xFF10B981) : const Color(0xFFF59E0B);
              final label = isConnected ? 'CONNECTED' : 'SCANNING...';
              
              return GestureDetector(
                onTap: () {
                  bleService.reconnect();
                  ScaffoldMessenger.of(context).showSnackBar(
                    SnackBar(content: Text('Reconnecting... ${state.message}')),
                  );
                },
                child: Container(
                  padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 6),
                  margin: const EdgeInsets.only(right: 16),
                  decoration: BoxDecoration(
                    color: color.withOpacity(0.15),
                    borderRadius: BorderRadius.circular(20),
                    border: Border.all(color: color.withOpacity(0.5)),
                  ),
                  child: Row(
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      Icon(
                        isConnected ? Icons.bluetooth_connected : Icons.bluetooth_searching,
                        size: 14,
                        color: color,
                      ),
                      const SizedBox(width: 6),
                      Text(
                        label,
                        style: TextStyle(
                          fontSize: 10,
                          fontWeight: FontWeight.bold,
                          color: color,
                          letterSpacing: 0.5,
                        ),
                      ),
                    ],
                  ),
                ),
              );
            },
          ),
        ],
      ),
      body: Container(
        decoration: const BoxDecoration(
          gradient: LinearGradient(
            begin: Alignment.topCenter,
            end: Alignment.bottomCenter,
            colors: [
              Color(0xFF0F172A), // Slate 900
              Color(0xFF020617), // Slate 950
            ],
          ),
        ),
        child: SafeArea(
          child: ListView(
            padding: const EdgeInsets.all(20),
            children: <Widget>[
              // Connection Status Banner
              StreamBuilder<BleConnectionState>(
                stream: bleService.connectionStateStream,
                initialData: bleService.currentState,
                builder: (context, snapshot) {
                  final state = snapshot.data ?? BleConnectionState.disconnected;
                  final isConnected = state.isConnected;
                  
                  return Container(
                    width: double.infinity,
                    padding: const EdgeInsets.all(20),
                    margin: const EdgeInsets.only(bottom: 24),
                    decoration: BoxDecoration(
                      color: isConnected 
                          ? const Color(0xFF0F766E) // Teal-700
                          : const Color(0xFF92400E), // Amber-800
                      borderRadius: BorderRadius.circular(16),
                    ),
                    child: Row(
                      children: [
                        Container(
                          padding: const EdgeInsets.all(10),
                          decoration: BoxDecoration(
                            color: Colors.white.withOpacity(0.15),
                            borderRadius: BorderRadius.circular(12),
                          ),
                          child: Icon(
                            isConnected ? Icons.waves : Icons.bluetooth_searching,
                            color: Colors.white,
                            size: 24,
                          ),
                        ),
                        const SizedBox(width: 16),
                        Expanded(
                          child: Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              Text(
                                isConnected ? 'Main Pond' : 'Searching...',
                                style: const TextStyle(color: Colors.white70, fontSize: 13, fontWeight: FontWeight.w500),
                              ),
                              const SizedBox(height: 2),
                              Text(
                                isConnected ? 'System Active' : state.message,
                                style: const TextStyle(color: Colors.white, fontSize: 16, fontWeight: FontWeight.bold),
                              ),
                            ],
                          ),
                        ),
                        if (!isConnected)
                          IconButton(
                            icon: const Icon(Icons.refresh, color: Colors.white),
                            onPressed: () => bleService.reconnect(),
                          ),
                      ],
                    ),
                  );
                },
              ),

              const Text(
                'SENSORS',
                style: TextStyle(
                  color: Colors.white54,
                  fontSize: 12,
                  fontWeight: FontWeight.bold,
                  letterSpacing: 1.0,
                ),
              ),
              const SizedBox(height: 12),

              // Sensor Cards with Logic
              SensorCard(
                title: 'Dissolved Oxygen',
                value: '${status.dissolvedOxygen.toStringAsFixed(1)} mg/L',
                subtitle: getDoStatus(status.dissolvedOxygen),
                icon: Icons.water_drop_outlined,
                statusColor: getDoColor(status.dissolvedOxygen),
                progress: status.dissolvedOxygen / 10.0,
                onTap: widget.onNavigateToSensors,
              ),
              SensorCard(
                title: 'Feed Level',
                value: '${status.feedLevel}%',
                subtitle: '${status.feedLevel > 20 ? "SUFFICIENT" : "REFILL NEEDED"}',
                icon: Icons.inventory_2_outlined,
                statusColor: getLevelColor(status.feedLevel),
                progress: status.feedLevel / 100.0,
                onTap: widget.onNavigateToSensors,
              ),
              SensorCard(
                title: 'Battery',
                value: '${status.batteryStatus}%',
                subtitle: '${status.batteryStatus > 20 ? "HEALTHY" : "LOW POWER"}',
                icon: Icons.bolt_outlined,
                statusColor: getLevelColor(status.batteryStatus),
                progress: status.batteryStatus / 100.0,
                onTap: widget.onNavigateToSensors,
              ),

              const SizedBox(height: 24),
              Row(
                children: [
                   Container(
                     height: 1, 
                     width: 20, 
                     color: Colors.white.withOpacity(0.1)
                   ),
                   const SizedBox(width: 8),
                   Text(
                    'CONTROLS',
                    style: TextStyle(
                      color: Colors.white.withOpacity(0.4),
                      fontSize: 11,
                      fontWeight: FontWeight.bold,
                      letterSpacing: 1.5,
                    ),
                  ),
                  const SizedBox(width: 8),
                  Expanded(
                    child: Container(
                     height: 1, 
                     color: Colors.white.withOpacity(0.1)
                   ),
                  ),
                ],
              ),
              const SizedBox(height: 16),

              // Quick Actions Row
              Row(
                children: [
                  Expanded(
                    child: _QuickActionButton(
                      icon: _isFeeding ? null : Icons.set_meal_rounded,
                      label: _isFeeding ? 'FEEDING...' : 'FEED NOW',
                      color: const Color(0xFFF59E0B), // Amber
                      isLoading: _isFeeding,
                      onTap: _isFeeding ? null : () => _handleFeedNow(settingsVm),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 40),
            ],
          ),
        ),
      ),
    );
  }
}

class _QuickActionButton extends StatelessWidget {
  final IconData? icon;
  final String label;
  final Color color;
  final VoidCallback? onTap;
  final bool isLoading;

  const _QuickActionButton({
    required this.icon,
    required this.label,
    required this.color,
    required this.onTap,
    this.isLoading = false,
  });

  @override
  Widget build(BuildContext context) {
    return Material(
      color: Colors.transparent,
      child: InkWell(
        onTap: onTap,
        borderRadius: BorderRadius.circular(12),
        child: AnimatedContainer(
          duration: const Duration(milliseconds: 200),
          padding: const EdgeInsets.symmetric(vertical: 16),
          decoration: BoxDecoration(
            color: isLoading 
                ? color.withOpacity(0.1) 
                : const Color(0xFF1E293B), // Slate 800
            borderRadius: BorderRadius.circular(12),
            border: Border.all(
              color: isLoading ? color.withOpacity(0.5) : Colors.white.withOpacity(0.08),
            ),
          ),
          child: Column(
            children: [
              if (isLoading)
                SizedBox(
                  width: 22,
                  height: 22,
                  child: CircularProgressIndicator(
                    strokeWidth: 2,
                    valueColor: AlwaysStoppedAnimation<Color>(color),
                  ),
                )
              else if (icon != null)
                Icon(icon, color: color, size: 22),
              const SizedBox(height: 8),
              Text(
                label,
                style: TextStyle(
                  color: isLoading ? color : Colors.white,
                  fontWeight: FontWeight.w600,
                  fontSize: 11,
                  letterSpacing: 0.5,
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
