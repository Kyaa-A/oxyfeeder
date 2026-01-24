import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../../../core/services/real_bluetooth_service.dart';

class AboutScreen extends StatelessWidget {
  const AboutScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      extendBodyBehindAppBar: true,
      appBar: AppBar(
        title: const Text('ABOUT'),
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
          child: ListView(
            padding: const EdgeInsets.all(20),
            children: [
              // App Logo & Title
              const SizedBox(height: 20),
              Center(
                child: Container(
                  padding: const EdgeInsets.all(20),
                  decoration: BoxDecoration(
                    color: const Color(0xFF14B8A6).withOpacity(0.15),
                    borderRadius: BorderRadius.circular(24),
                  ),
                  child: const Icon(
                    Icons.water_drop,
                    size: 64,
                    color: Color(0xFF14B8A6),
                  ),
                ),
              ),
              const SizedBox(height: 20),
              const Center(
                child: Text(
                  'OxyFeeder',
                  style: TextStyle(
                    color: Colors.white,
                    fontSize: 28,
                    fontWeight: FontWeight.bold,
                    letterSpacing: 1.0,
                  ),
                ),
              ),
              Center(
                child: Text(
                  'Smart Aquaculture Monitoring System',
                  style: TextStyle(
                    color: Colors.white.withOpacity(0.5),
                    fontSize: 13,
                  ),
                ),
              ),
              const SizedBox(height: 32),

              // Version Info
              _buildSectionHeader('APP INFO'),
              _buildInfoCard([
                _InfoRow(label: 'Version', value: '1.0.0'),
                _InfoRow(label: 'Build', value: '2024.01.24'),
                _InfoRow(label: 'Platform', value: 'Android'),
              ]),
              const SizedBox(height: 20),

              // Device Info
              _buildSectionHeader('DEVICE INFO'),
              Consumer<RealBluetoothService>(
                builder: (context, bleService, _) {
                  return StreamBuilder<BleConnectionState>(
                    stream: bleService.connectionStateStream,
                    initialData: bleService.currentState,
                    builder: (context, snapshot) {
                      final state = snapshot.data ?? BleConnectionState.disconnected;
                      final isConnected = state.isConnected;

                      return _buildInfoCard([
                        _InfoRow(
                          label: 'Device Name',
                          value: 'OxyFeeder',
                        ),
                        _InfoRow(
                          label: 'Status',
                          value: isConnected ? 'Connected' : 'Disconnected',
                          valueColor: isConnected 
                              ? const Color(0xFF10B981) 
                              : const Color(0xFFEF4444),
                        ),
                        _InfoRow(
                          label: 'Connection',
                          value: 'Bluetooth Low Energy',
                        ),
                      ]);
                    },
                  );
                },
              ),
              const SizedBox(height: 20),

              // Hardware Info
              _buildSectionHeader('HARDWARE'),
              _buildInfoCard([
                const _InfoRow(label: 'Controller', value: 'Arduino Mega 2560'),
                const _InfoRow(label: 'BLE Bridge', value: 'ESP32'),
                const _InfoRow(label: 'Camera', value: 'ESP32-CAM'),
                const _InfoRow(label: 'DO Sensor', value: 'DFRobot Analog'),
                const _InfoRow(label: 'Load Cell', value: 'HX711 + 5kg Cell'),
                const _InfoRow(label: 'RTC', value: 'DS3231'),
                const _InfoRow(label: 'GSM', value: 'SIM800L'),
              ]),
              const SizedBox(height: 20),

              // Features
              _buildSectionHeader('FEATURES'),
              _buildInfoCard([
                const _InfoRow(
                  label: '✓',
                  value: 'Real-time dissolved oxygen monitoring',
                  isFeature: true,
                ),
                const _InfoRow(
                  label: '✓',
                  value: 'Automated fish feeding schedules',
                  isFeature: true,
                ),
                const _InfoRow(
                  label: '✓',
                  value: 'SMS alerts for critical conditions',
                  isFeature: true,
                ),
                const _InfoRow(
                  label: '✓',
                  value: 'Battery & feed level monitoring',
                  isFeature: true,
                ),
                const _InfoRow(
                  label: '✓',
                  value: 'Live camera feed (ESP32-CAM)',
                  isFeature: true,
                ),
              ]),
              const SizedBox(height: 20),

              // Credits
              _buildSectionHeader('CREDITS'),
              Container(
                padding: const EdgeInsets.all(16),
                decoration: BoxDecoration(
                  color: const Color(0xFF1E293B),
                  borderRadius: BorderRadius.circular(12),
                  border: Border.all(color: Colors.white.withOpacity(0.08)),
                ),
                child: Column(
                  children: [
                    const Text(
                      'Developed by Team',
                      style: TextStyle(
                        color: Colors.white,
                        fontSize: 16,
                        fontWeight: FontWeight.bold,
                      ),
                      textAlign: TextAlign.center,
                    ),
                    const SizedBox(height: 8),
                    Text(
                      'Smart Aquaculture Monitoring System',
                      style: TextStyle(
                        color: Colors.white.withOpacity(0.5),
                        fontSize: 12,
                      ),
                      textAlign: TextAlign.center,
                    ),
                    const SizedBox(height: 12),
                    Text(
                      '© 2024 OxyFeeder Project',
                      style: TextStyle(
                        color: Colors.white.withOpacity(0.4),
                        fontSize: 11,
                      ),
                      textAlign: TextAlign.center,
                    ),
                  ],
                ),
              ),
              const SizedBox(height: 40),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildSectionHeader(String title) {
    return Padding(
      padding: const EdgeInsets.only(left: 4, bottom: 8),
      child: Text(
        title,
        style: const TextStyle(
          color: Colors.white54,
          fontSize: 10,
          fontWeight: FontWeight.bold,
          letterSpacing: 1.5,
        ),
      ),
    );
  }

  Widget _buildInfoCard(List<_InfoRow> rows) {
    return Container(
      decoration: BoxDecoration(
        color: const Color(0xFF1E293B),
        borderRadius: BorderRadius.circular(12),
        border: Border.all(color: Colors.white.withOpacity(0.08)),
      ),
      child: Column(
        children: [
          for (int i = 0; i < rows.length; i++) ...[
            rows[i],
            if (i < rows.length - 1)
              Divider(
                color: Colors.white.withOpacity(0.05),
                height: 1,
                indent: 16,
                endIndent: 16,
              ),
          ],
        ],
      ),
    );
  }
}

class _InfoRow extends StatelessWidget {
  final String label;
  final String value;
  final Color? valueColor;
  final bool isFeature;

  const _InfoRow({
    required this.label,
    required this.value,
    this.valueColor,
    this.isFeature = false,
  });

  @override
  Widget build(BuildContext context) {
    if (isFeature) {
      return Padding(
        padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 10),
        child: Row(
          children: [
            const Icon(Icons.check_circle, color: Color(0xFF10B981), size: 16),
            const SizedBox(width: 12),
            Expanded(
              child: Text(
                value,
                style: TextStyle(
                  color: Colors.white.withOpacity(0.8),
                  fontSize: 12,
                ),
              ),
            ),
          ],
        ),
      );
    }

    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(
            label,
            style: TextStyle(
              color: Colors.white.withOpacity(0.5),
              fontSize: 12,
            ),
          ),
          Text(
            value,
            style: TextStyle(
              color: valueColor ?? Colors.white,
              fontWeight: FontWeight.w500,
              fontSize: 12,
              fontFamily: 'RobotoMono',
            ),
          ),
        ],
      ),
    );
  }
}
