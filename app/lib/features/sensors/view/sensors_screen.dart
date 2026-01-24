import 'package:flutter/material.dart';
import 'package:fl_chart/fl_chart.dart';
import 'package:provider/provider.dart';
import '../viewmodel/sensors_viewmodel.dart';

class SensorsScreen extends StatelessWidget {
  const SensorsScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      extendBodyBehindAppBar: true,
      appBar: AppBar(
        title: const Text('SENSOR TELEMETRY'),
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
              Color(0xFF0F172A), // Slate 900
              Color(0xFF020617), // Slate 950
            ],
          ),
        ),
        child: SafeArea(
          child: ListView(
            padding: const EdgeInsets.all(20),
            children: <Widget>[
              // System Status Card
              Consumer<SensorsViewModel>(
                builder: (context, viewModel, _) {
                  return Container(
                    padding: const EdgeInsets.all(20),
                    decoration: BoxDecoration(
                      color: const Color(0xFF1E293B),
                      borderRadius: BorderRadius.circular(12),
                      border: Border.all(color: Colors.white.withOpacity(0.08)),
                    ),
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Row(
                          mainAxisAlignment: MainAxisAlignment.spaceBetween,
                          children: [
                            const Text(
                              'LIVE READINGS',
                              style: TextStyle(
                                color: Colors.white54,
                                fontSize: 10,
                                fontWeight: FontWeight.bold,
                                letterSpacing: 1.5,
                              ),
                            ),
                            Container(
                              padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 4),
                              decoration: BoxDecoration(
                                color: viewModel.isReceivingData 
                                    ? const Color(0xFF10B981).withOpacity(0.2)
                                    : const Color(0xFFF59E0B).withOpacity(0.2),
                                borderRadius: BorderRadius.circular(4),
                              ),
                              child: Text(
                                viewModel.isReceivingData ? 'LIVE' : 'WAITING',
                                style: TextStyle(
                                  color: viewModel.isReceivingData 
                                      ? const Color(0xFF10B981)
                                      : const Color(0xFFF59E0B),
                                  fontSize: 9,
                                  fontWeight: FontWeight.bold,
                                ),
                              ),
                            ),
                          ],
                        ),
                        const SizedBox(height: 16),
                        _buildSensorRow(
                          'Dissolved Oxygen',
                          '${viewModel.currentStatus.dissolvedOxygen.toStringAsFixed(2)} mg/L',
                          Icons.water_drop_outlined,
                          Colors.tealAccent,
                        ),
                        Divider(color: Colors.white.withOpacity(0.05), height: 24),
                        _buildSensorRow(
                          'Feed Level',
                          '${viewModel.currentStatus.feedLevel}%',
                          Icons.inventory_2_outlined,
                          Colors.amberAccent,
                        ),
                        Divider(color: Colors.white.withOpacity(0.05), height: 24),
                        _buildSensorRow(
                          'Battery',
                          '${viewModel.currentStatus.batteryStatus}% (${viewModel.batteryVoltage})',
                          Icons.bolt_outlined,
                          Colors.greenAccent,
                        ),
                      ],
                    ),
                  );
                },
              ),
              const SizedBox(height: 24),

              // Chart Section Header
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  const Text(
                    'DO HISTORY',
                    style: TextStyle(
                      color: Colors.white54,
                      fontSize: 12,
                      fontWeight: FontWeight.bold,
                      letterSpacing: 1.2,
                    ),
                  ),
                  Consumer<SensorsViewModel>(
                    builder: (context, viewModel, _) {
                      return Text(
                        '${viewModel.historicalDoData.length} readings',
                        style: TextStyle(
                          color: Colors.white.withOpacity(0.3),
                          fontSize: 10,
                          fontFamily: 'RobotoMono',
                        ),
                      );
                    },
                  ),
                ],
              ),
              const SizedBox(height: 12),
              
              Consumer<SensorsViewModel>(
                builder: (context, viewModel, _) {
                  final hasData = viewModel.historicalDoData.isNotEmpty;
                  
                  return Container(
                    height: 280,
                    padding: const EdgeInsets.all(20),
                    decoration: BoxDecoration(
                      color: const Color(0xFF1E293B),
                      borderRadius: BorderRadius.circular(12),
                      border: Border.all(color: Colors.white.withOpacity(0.08)),
                    ),
                    child: hasData
                        ? LineChart(
                            LineChartData(
                              backgroundColor: Colors.transparent,
                              gridData: FlGridData(
                                show: true,
                                drawVerticalLine: false,
                                horizontalInterval: 2,
                                getDrawingHorizontalLine: (value) => FlLine(
                                  color: Colors.white.withOpacity(0.05),
                                  strokeWidth: 1,
                                ),
                              ),
                              titlesData: FlTitlesData(
                                show: true,
                                rightTitles: const AxisTitles(sideTitles: SideTitles(showTitles: false)),
                                topTitles: const AxisTitles(sideTitles: SideTitles(showTitles: false)),
                                bottomTitles: AxisTitles(
                                  sideTitles: SideTitles(
                                    showTitles: true,
                                    reservedSize: 24,
                                    interval: (viewModel.historicalDoData.length / 6).clamp(1, 10).toDouble(),
                                    getTitlesWidget: (value, meta) {
                                      return Text(
                                        '${value.toInt()}',
                                        style: TextStyle(color: Colors.white.withOpacity(0.4), fontSize: 10, fontFamily: 'RobotoMono'),
                                      );
                                    },
                                  ),
                                ),
                                leftTitles: AxisTitles(
                                  sideTitles: SideTitles(
                                    showTitles: true,
                                    reservedSize: 30,
                                    interval: 2,
                                    getTitlesWidget: (value, meta) {
                                      return Text(
                                        value.toInt().toString(),
                                        style: TextStyle(color: Colors.white.withOpacity(0.4), fontSize: 10, fontFamily: 'RobotoMono'),
                                      );
                                    },
                                  ),
                                ),
                              ),
                              borderData: FlBorderData(show: false),
                              minX: 0,
                              maxX: (viewModel.historicalDoData.length - 1).clamp(0, 23).toDouble(),
                              minY: 0,
                              maxY: 12,
                              lineBarsData: [
                                LineChartBarData(
                                  spots: viewModel.historicalDoData,
                                  isCurved: true,
                                  color: const Color(0xFF14B8A6), // Teal-500
                                  barWidth: 2,
                                  isStrokeCapRound: true,
                                  dotData: const FlDotData(show: false),
                                  belowBarData: BarAreaData(
                                    show: true,
                                    gradient: LinearGradient(
                                      begin: Alignment.topCenter,
                                      end: Alignment.bottomCenter,
                                      colors: [
                                        const Color(0xFF14B8A6).withOpacity(0.2),
                                        const Color(0xFF14B8A6).withOpacity(0.0),
                                      ],
                                    ),
                                  ),
                                ),
                              ],
                            ),
                          )
                        : Center(
                            child: Column(
                              mainAxisAlignment: MainAxisAlignment.center,
                              children: [
                                Icon(Icons.show_chart, color: Colors.white.withOpacity(0.2), size: 48),
                                const SizedBox(height: 12),
                                Text(
                                  'Waiting for sensor data...',
                                  style: TextStyle(color: Colors.white.withOpacity(0.4), fontSize: 14),
                                ),
                              ],
                            ),
                          ),
                  );
                },
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildSensorRow(String label, String value, IconData icon, Color color) {
    return Row(
      children: [
        Container(
          padding: const EdgeInsets.all(8),
          decoration: BoxDecoration(
            color: color.withOpacity(0.1),
            borderRadius: BorderRadius.circular(6),
          ),
          child: Icon(icon, color: color, size: 18),
        ),
        const SizedBox(width: 16),
        Expanded(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Text(
                label.toUpperCase(),
                style: TextStyle(
                  color: Colors.white.withOpacity(0.6),
                  fontSize: 10,
                  fontWeight: FontWeight.w600,
                  letterSpacing: 0.5,
                ),
              ),
              const SizedBox(height: 2),
              Text(
                value,
                style: const TextStyle(
                  color: Colors.white,
                  fontSize: 14,
                  fontFamily: 'RobotoMono',
                  fontWeight: FontWeight.w500,
                ),
              ),
            ],
          ),
        ),
      ],
    );
  }
}
