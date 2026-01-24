import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../viewmodel/settings_viewmodel.dart';
import '../../../core/services/real_bluetooth_service.dart';
import '../../history/view/history_screen.dart';
import '../../camera/view/camera_screen.dart';
import '../../about/view/about_screen.dart';

class SettingsScreen extends StatefulWidget {
  const SettingsScreen({super.key});

  @override
  State<SettingsScreen> createState() => _SettingsScreenState();
}

class _SettingsScreenState extends State<SettingsScreen> {
  bool _isSaving = false;
  bool _isTesting = false;

  Future<void> _handleSavePhone(SettingsViewModel vm) async {
    setState(() => _isSaving = true);
    final success = await vm.sendPhoneNumberToDevice();
    setState(() => _isSaving = false);
    
    if (mounted) {
      _showFeedback(
        success ? 'Phone number saved to device!' : 'Failed to save. Check connection.',
        success,
      );
    }
  }

  Future<void> _handleTestSms(SettingsViewModel vm) async {
    setState(() => _isTesting = true);
    final success = await vm.sendTestSms();
    setState(() => _isTesting = false);
    
    if (mounted) {
      _showFeedback(
        success ? 'Test SMS command sent!' : 'Failed to send. Check connection.',
        success,
      );
    }
  }

  void _showFeedback(String message, bool isSuccess) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Row(
          children: [
            Icon(
              isSuccess ? Icons.check_circle : Icons.error,
              color: Colors.white,
              size: 20,
            ),
            const SizedBox(width: 12),
            Expanded(child: Text(message)),
          ],
        ),
        backgroundColor: isSuccess ? const Color(0xFF10B981) : const Color(0xFFEF4444),
        behavior: SnackBarBehavior.floating,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
        margin: const EdgeInsets.all(16),
        duration: const Duration(seconds: 2),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      extendBodyBehindAppBar: true,
      appBar: AppBar(
        title: const Text('SYSTEM CONFIGURATION'),
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
      floatingActionButton: FloatingActionButton(
        backgroundColor: const Color(0xFF14B8A6), // Teal
        foregroundColor: Colors.white,
        child: const Icon(Icons.add),
        onPressed: () => _showAddScheduleDialog(context),
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
              // Feeding Schedule Section
              _buildSectionHeader('FEEDING AUTOMATION'),
              Consumer<SettingsViewModel>(
                builder: (context, viewModel, _) {
                  if (viewModel.feedingSchedules.isEmpty) {
                    return _buildEmptyState('No active feeding schedules');
                  }
                  return Container(
                    decoration: _cardDecoration,
                    clipBehavior: Clip.antiAlias,
                    child: Column(
                      children: [
                        for (int i = 0; i < viewModel.feedingSchedules.length; i++) ...[
                          _buildScheduleItem(context, viewModel, i),
                          if (i != viewModel.feedingSchedules.length - 1)
                            Divider(color: Colors.white.withOpacity(0.05), height: 1),
                        ],
                      ],
                    ),
                  );
                },
              ),
              const SizedBox(height: 24),

              // Alert Thresholds Section
              _buildSectionHeader('SAFETY THRESHOLDS'),
              Consumer<SettingsViewModel>(
                builder: (context, viewModel, _) {
                  return Container(
                    decoration: _cardDecoration,
                    padding: const EdgeInsets.symmetric(vertical: 8),
                    child: Column(
                      children: <Widget>[
                        _buildThresholdSlider(
                          label: 'Min Dissolved Oxygen',
                          value: viewModel.minDissolvedOxygen,
                          min: 2.0,
                          max: 8.0,
                          divisions: 12,
                          displayValue: '${viewModel.minDissolvedOxygen.toStringAsFixed(1)} mg/L',
                          onChanged: viewModel.updateMinDissolvedOxygen,
                          icon: Icons.water_drop_outlined,
                        ),
                        Divider(color: Colors.white.withOpacity(0.05), height: 1),
                        _buildThresholdSlider(
                          label: 'Low Feed Warning',
                          value: viewModel.lowFeedThreshold.toDouble(),
                          min: 0,
                          max: 100,
                          divisions: 20,
                          displayValue: '${viewModel.lowFeedThreshold} %',
                          onChanged: (v) => viewModel.updateLowFeedThreshold(v.toInt()),
                          icon: Icons.inventory_2_outlined,
                        ),
                        Divider(color: Colors.white.withOpacity(0.05), height: 1),
                        _buildThresholdSlider(
                          label: 'Low Battery Warning',
                          value: viewModel.lowBatteryThreshold.toDouble(),
                          min: 0,
                          max: 100,
                          divisions: 20,
                          displayValue: '${viewModel.lowBatteryThreshold} %',
                          onChanged: (v) => viewModel.updateLowBatteryThreshold(v.toInt()),
                          icon: Icons.bolt_outlined,
                        ),
                      ],
                    ),
                  );
                },
              ),
              const SizedBox(height: 24),

              // SMS Configuration Section
              _buildSectionHeader('ALERT SYSTEM'),
              Container(
                decoration: _cardDecoration,
                padding: const EdgeInsets.all(16),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: <Widget>[
                    Consumer<SettingsViewModel>(
                      builder: (context, viewModel, _) {
                        return Column(
                          crossAxisAlignment: CrossAxisAlignment.stretch,
                          children: [
                            TextFormField(
                              initialValue: viewModel.smsPhoneNumber,
                              style: const TextStyle(color: Colors.white, fontFamily: 'RobotoMono'),
                              decoration: InputDecoration(
                                labelText: 'RECIPIENT PHONE NUMBER',
                                labelStyle: TextStyle(color: Colors.white.withOpacity(0.4), fontSize: 10, letterSpacing: 1.0),
                                hintText: '+639XXXXXXXXX',
                                hintStyle: TextStyle(color: Colors.white.withOpacity(0.2)),
                                prefixIcon: Icon(Icons.phone_outlined, color: Colors.white.withOpacity(0.4), size: 18),
                                filled: true,
                                fillColor: Colors.black.withOpacity(0.2),
                                border: OutlineInputBorder(
                                  borderRadius: BorderRadius.circular(8),
                                  borderSide: BorderSide.none,
                                ),
                                enabledBorder: OutlineInputBorder(
                                  borderRadius: BorderRadius.circular(8),
                                  borderSide: BorderSide(color: Colors.white.withOpacity(0.05)),
                                ),
                              ),
                              keyboardType: TextInputType.phone,
                              onChanged: viewModel.updateSmsPhoneNumber,
                            ),
                            const SizedBox(height: 16),
                            Row(
                              children: [
                                Expanded(
                                  child: _buildLoadingButton(
                                    icon: Icons.save_alt_rounded,
                                    label: 'SAVE',
                                    color: const Color(0xFF14B8A6), // Teal
                                    isLoading: _isSaving,
                                    onTap: () => _handleSavePhone(viewModel),
                                  ),
                                ),
                                const SizedBox(width: 12),
                                Expanded(
                                  child: _buildLoadingButton(
                                    icon: Icons.send_rounded,
                                    label: 'TEST SMS',
                                    color: const Color(0xFF6366F1), // Indigo
                                    isLoading: _isTesting,
                                    onTap: () => _handleTestSms(viewModel),
                                  ),
                                ),
                              ],
                            ),
                          ],
                        );
                      },
                    ),
                  ],
                ),
              ),
              const SizedBox(height: 24),

              // Device Management
              _buildSectionHeader('CONNECTION'),
              Builder(
                builder: (context) {
                  final bleService = context.read<RealBluetoothService>();
                  return StreamBuilder<BleConnectionState>(
                    stream: bleService.connectionStateStream,
                    initialData: bleService.currentState,
                    builder: (context, snapshot) {
                      final state = snapshot.data ?? BleConnectionState.disconnected;
                      final isConnected = state.isConnected;
                      final statusColor = isConnected ? const Color(0xFF10B981) : const Color(0xFFF59E0B);
                      final statusText = isConnected ? 'CONNECTED' : 'SEARCHING';
                      final statusIcon = isConnected ? Icons.bluetooth_connected : Icons.bluetooth_searching;
                      
                      return Container(
                        decoration: _cardDecoration,
                        child: Column(
                          children: <Widget>[
                            ListTile(
                              leading: Container(
                                padding: const EdgeInsets.all(8),
                                decoration: BoxDecoration(color: statusColor.withOpacity(0.1), borderRadius: BorderRadius.circular(6)),
                                child: Icon(statusIcon, color: statusColor, size: 18),
                              ),
                              title: const Text('Device Status', style: TextStyle(color: Colors.white70, fontSize: 13)),
                              subtitle: Text(state.message, style: TextStyle(color: Colors.white.withOpacity(0.4), fontSize: 11)),
                              trailing: Text(statusText, style: TextStyle(color: statusColor, fontWeight: FontWeight.bold, fontSize: 12, letterSpacing: 1.0)),
                            ),
                            Divider(color: Colors.white.withOpacity(0.05), height: 1),
                            ListTile(
                              leading: Icon(isConnected ? Icons.link_off : Icons.refresh, color: Colors.white54, size: 20),
                              title: Text(isConnected ? 'Disconnect' : 'Retry Connection', style: const TextStyle(color: Colors.white, fontSize: 14)),
                              trailing: Icon(Icons.arrow_forward_ios, color: Colors.white.withOpacity(0.2), size: 14),
                              onTap: () {
                                if (isConnected) {
                                  bleService.disconnect();
                                } else {
                                  bleService.reconnect();
                                }
                              },
                            ),
                          ],
                        ),
                      );
                    },
                  );
                },
              ),
              const SizedBox(height: 24),

              // More Section
              _buildSectionHeader('MORE'),
              Container(
                decoration: _cardDecoration,
                child: Column(
                  children: [
                    _buildNavTile(
                      icon: Icons.history,
                      title: 'Event History',
                      subtitle: 'View past events and alerts',
                      onTap: () => Navigator.push(
                        context,
                        MaterialPageRoute(builder: (_) => const HistoryScreen()),
                      ),
                    ),
                    Divider(color: Colors.white.withOpacity(0.05), height: 1),
                    _buildNavTile(
                      icon: Icons.videocam,
                      title: 'Live Camera',
                      subtitle: 'ESP32-CAM feed',
                      onTap: () => Navigator.push(
                        context,
                        MaterialPageRoute(builder: (_) => const CameraScreen()),
                      ),
                    ),
                    Divider(color: Colors.white.withOpacity(0.05), height: 1),
                    _buildNavTile(
                      icon: Icons.info_outline,
                      title: 'About',
                      subtitle: 'App info and credits',
                      onTap: () => Navigator.push(
                        context,
                        MaterialPageRoute(builder: (_) => const AboutScreen()),
                      ),
                    ),
                  ],
                ),
              ),
              const SizedBox(height: 24),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildNavTile({
    required IconData icon,
    required String title,
    required String subtitle,
    required VoidCallback onTap,
  }) {
    return ListTile(
      leading: Container(
        padding: const EdgeInsets.all(8),
        decoration: BoxDecoration(
          color: const Color(0xFF14B8A6).withOpacity(0.1),
          borderRadius: BorderRadius.circular(8),
        ),
        child: Icon(icon, color: const Color(0xFF14B8A6), size: 20),
      ),
      title: Text(title, style: const TextStyle(color: Colors.white, fontSize: 14)),
      subtitle: Text(subtitle, style: TextStyle(color: Colors.white.withOpacity(0.4), fontSize: 11)),
      trailing: Icon(Icons.chevron_right, color: Colors.white.withOpacity(0.3)),
      onTap: onTap,
    );
  }

  // --- Widgets ---

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

  BoxDecoration get _cardDecoration => BoxDecoration(
    color: const Color(0xFF1E293B),
    borderRadius: BorderRadius.circular(12),
    border: Border.all(color: Colors.white.withOpacity(0.08)),
  );

  Widget _buildEmptyState(String text) {
    return Container(
      padding: const EdgeInsets.all(24),
      decoration: BoxDecoration(
        color: const Color(0xFF1E293B).withOpacity(0.5),
        borderRadius: BorderRadius.circular(12),
        border: Border.all(color: Colors.white.withOpacity(0.05), style: BorderStyle.solid),
      ),
      child: Center(
        child: Text(
          text,
          style: TextStyle(color: Colors.white.withOpacity(0.3), fontStyle: FontStyle.italic),
        ),
      ),
    );
  }

  Widget _buildScheduleItem(BuildContext context, SettingsViewModel viewModel, int index) {
    final schedule = viewModel.feedingSchedules[index];
    return ListTile(
      contentPadding: const EdgeInsets.symmetric(horizontal: 16, vertical: 4),
      leading: Container(
        padding: const EdgeInsets.all(8),
        decoration: BoxDecoration(
          color: Colors.white.withOpacity(0.05),
          borderRadius: BorderRadius.circular(8),
        ),
        child: const Icon(Icons.access_time, color: Colors.white70, size: 20),
      ),
      title: Text(
        schedule.timeLabel,
        style: const TextStyle(color: Colors.white, fontWeight: FontWeight.bold, fontSize: 16, fontFamily: 'RobotoMono'),
      ),
      subtitle: Text(
        'Duration: ${schedule.durationSeconds}s',
        style: TextStyle(color: Colors.white.withOpacity(0.5), fontSize: 12),
      ),
      trailing: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          Switch(
            value: schedule.enabled,
            onChanged: (v) => viewModel.toggleScheduleEnabled(index, v),
            activeColor: const Color(0xFF14B8A6),
          ),
          IconButton(
            icon: Icon(Icons.delete_outline, color: Colors.white.withOpacity(0.3), size: 20),
            onPressed: () => viewModel.removeSchedule(index),
          ),
        ],
      ),
    );
  }

  Widget _buildThresholdSlider({
    required String label,
    required double value,
    required double min,
    required double max,
    required int divisions,
    required String displayValue,
    required ValueChanged<double> onChanged,
    required IconData icon,
  }) {
    return Column(
      children: [
        Padding(
          padding: const EdgeInsets.fromLTRB(16, 12, 16, 0),
          child: Row(
            children: [
              Icon(icon, size: 16, color: Colors.white54),
              const SizedBox(width: 8),
              Text(label, style: const TextStyle(color: Colors.white70, fontSize: 13, fontWeight: FontWeight.w500)),
              const Spacer(),
              Text(displayValue, style: const TextStyle(color: Colors.white, fontFamily: 'RobotoMono', fontWeight: FontWeight.bold)),
            ],
          ),
        ),
        SliderTheme(
          data: SliderThemeData(
            activeTrackColor: const Color(0xFF14B8A6),
            inactiveTrackColor: Colors.black.withOpacity(0.3),
            thumbColor: Colors.white,
            trackHeight: 2,
            thumbShape: const RoundSliderThumbShape(enabledThumbRadius: 6),
            overlayShape: const RoundSliderOverlayShape(overlayRadius: 12),
          ),
          child: Slider(
            value: value,
            min: min,
            max: max,
            divisions: divisions,
            onChanged: onChanged,
          ),
        ),
      ],
    );
  }

  Widget _buildActionButton({required IconData icon, required String label, required Color color, required VoidCallback onTap}) {
    return Material(
      color: Colors.transparent,
      child: InkWell(
        onTap: onTap,
        borderRadius: BorderRadius.circular(8),
        child: Container(
          padding: const EdgeInsets.symmetric(vertical: 12),
          decoration: BoxDecoration(
            border: Border.all(color: color.withOpacity(0.5)),
            borderRadius: BorderRadius.circular(8),
            color: color.withOpacity(0.1),
          ),
          child: Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Icon(icon, size: 16, color: color),
              const SizedBox(width: 8),
              Text(
                label,
                style: TextStyle(
                  color: color,
                  fontWeight: FontWeight.bold,
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

  Widget _buildLoadingButton({
    required IconData icon,
    required String label,
    required Color color,
    required bool isLoading,
    required VoidCallback onTap,
  }) {
    return Material(
      color: Colors.transparent,
      child: InkWell(
        onTap: isLoading ? null : onTap,
        borderRadius: BorderRadius.circular(8),
        child: AnimatedContainer(
          duration: const Duration(milliseconds: 200),
          padding: const EdgeInsets.symmetric(vertical: 14),
          decoration: BoxDecoration(
            border: Border.all(color: isLoading ? color.withOpacity(0.3) : color.withOpacity(0.6)),
            borderRadius: BorderRadius.circular(8),
            color: isLoading ? color.withOpacity(0.05) : color.withOpacity(0.15),
          ),
          child: Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              if (isLoading)
                SizedBox(
                  width: 14,
                  height: 14,
                  child: CircularProgressIndicator(
                    strokeWidth: 2,
                    valueColor: AlwaysStoppedAnimation<Color>(color),
                  ),
                )
              else
                Icon(icon, size: 16, color: color),
              const SizedBox(width: 8),
              Text(
                isLoading ? 'SENDING...' : label,
                style: TextStyle(
                  color: isLoading ? color.withOpacity(0.6) : color,
                  fontWeight: FontWeight.bold,
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

  Future<void> _showAddScheduleDialog(BuildContext context) async {
    int hour = 8;
    int minute = 0;
    String period = 'AM';
    int duration = 10;
    bool enabled = true;

    await showDialog<void>(
      context: context,
      builder: (ctx) => StatefulBuilder(
        builder: (ctx, setState) => AlertDialog(
          backgroundColor: const Color(0xFF1E293B),
          title: const Text('ADD SCHEDULE', style: TextStyle(color: Colors.white, fontSize: 14, letterSpacing: 1.0)),
          content: SingleChildScrollView(
            child: Column(
              mainAxisSize: MainAxisSize.min,
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                // Time Selection Row
                Text('FEEDING TIME', style: TextStyle(color: Colors.white.withOpacity(0.5), fontSize: 10, letterSpacing: 1.0)),
                const SizedBox(height: 8),
                Row(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    // Hour Dropdown
                    Container(
                      padding: const EdgeInsets.symmetric(horizontal: 8),
                      decoration: BoxDecoration(
                        color: Colors.black.withOpacity(0.2),
                        borderRadius: BorderRadius.circular(8),
                        border: Border.all(color: Colors.white.withOpacity(0.1)),
                      ),
                      child: DropdownButtonHideUnderline(
                        child: DropdownButton<int>(
                          value: hour,
                          dropdownColor: const Color(0xFF1E293B),
                          style: const TextStyle(color: Colors.white, fontSize: 16, fontFamily: 'RobotoMono'),
                          items: List.generate(12, (i) => i + 1)
                              .map((h) => DropdownMenuItem(value: h, child: Text(h.toString().padLeft(2, '0'))))
                              .toList(),
                          onChanged: (v) => setState(() => hour = v!),
                        ),
                      ),
                    ),
                    const Padding(
                      padding: EdgeInsets.symmetric(horizontal: 4),
                      child: Text(':', style: TextStyle(color: Colors.white, fontSize: 20, fontWeight: FontWeight.bold)),
                    ),
                    // Minute Dropdown
                    Container(
                      padding: const EdgeInsets.symmetric(horizontal: 8),
                      decoration: BoxDecoration(
                        color: Colors.black.withOpacity(0.2),
                        borderRadius: BorderRadius.circular(8),
                        border: Border.all(color: Colors.white.withOpacity(0.1)),
                      ),
                      child: DropdownButtonHideUnderline(
                        child: DropdownButton<int>(
                          value: minute,
                          dropdownColor: const Color(0xFF1E293B),
                          style: const TextStyle(color: Colors.white, fontSize: 16, fontFamily: 'RobotoMono'),
                          items: [0, 15, 30, 45]
                              .map((m) => DropdownMenuItem(value: m, child: Text(m.toString().padLeft(2, '0'))))
                              .toList(),
                          onChanged: (v) => setState(() => minute = v!),
                        ),
                      ),
                    ),
                    const SizedBox(width: 8),
                    // AM/PM Selector
                    Container(
                      padding: const EdgeInsets.symmetric(horizontal: 8),
                      decoration: BoxDecoration(
                        color: Colors.black.withOpacity(0.2),
                        borderRadius: BorderRadius.circular(8),
                        border: Border.all(color: Colors.white.withOpacity(0.1)),
                      ),
                      child: DropdownButtonHideUnderline(
                        child: DropdownButton<String>(
                          value: period,
                          dropdownColor: const Color(0xFF1E293B),
                          style: const TextStyle(color: Colors.white, fontSize: 14, fontWeight: FontWeight.bold),
                          items: ['AM', 'PM']
                              .map((p) => DropdownMenuItem(value: p, child: Text(p)))
                              .toList(),
                          onChanged: (v) => setState(() => period = v!),
                        ),
                      ),
                    ),
                  ],
                ),
                const SizedBox(height: 16),
                
                // Duration Selection
                Text('DURATION', style: TextStyle(color: Colors.white.withOpacity(0.5), fontSize: 10, letterSpacing: 1.0)),
                const SizedBox(height: 8),
                Container(
                  padding: const EdgeInsets.symmetric(horizontal: 12),
                  decoration: BoxDecoration(
                    color: Colors.black.withOpacity(0.2),
                    borderRadius: BorderRadius.circular(8),
                    border: Border.all(color: Colors.white.withOpacity(0.1)),
                  ),
                  child: DropdownButtonHideUnderline(
                    child: DropdownButton<int>(
                      value: duration,
                      isExpanded: true,
                      dropdownColor: const Color(0xFF1E293B),
                      style: const TextStyle(color: Colors.white, fontSize: 16, fontFamily: 'RobotoMono'),
                      items: [5, 10, 15, 20, 30, 45, 60]
                          .map((d) => DropdownMenuItem(value: d, child: Text('$d sec')))
                          .toList(),
                      onChanged: (v) => setState(() => duration = v!),
                    ),
                  ),
                ),
                const SizedBox(height: 16),
                
                // Enable Switch
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceBetween,
                  children: [
                    const Text('Enable', style: TextStyle(color: Colors.white70, fontSize: 14)),
                    Switch(value: enabled, onChanged: (v) => setState(() => enabled = v), activeColor: const Color(0xFF14B8A6)),
                  ],
                ),
              ],
            ),
          ),
          actions: [
            TextButton(
              child: Text('CANCEL', style: TextStyle(color: Colors.white.withOpacity(0.4))),
              onPressed: () => Navigator.pop(ctx),
            ),
            TextButton(
              child: const Text('ADD', style: TextStyle(color: Color(0xFF14B8A6), fontWeight: FontWeight.bold)),
              onPressed: () {
                final timeLabel = '${hour.toString().padLeft(2, '0')}:${minute.toString().padLeft(2, '0')} $period';
                context.read<SettingsViewModel>().addSchedule(timeLabel: timeLabel, durationSeconds: duration, enabled: enabled);
                Navigator.pop(ctx);
              },
            ),
          ],
        ),
      ),
    );
  }
}


