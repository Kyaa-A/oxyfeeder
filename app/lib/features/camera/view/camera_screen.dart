import 'package:flutter/material.dart';
import 'package:flutter_mjpeg/flutter_mjpeg.dart';
import 'package:shared_preferences/shared_preferences.dart';

class CameraScreen extends StatefulWidget {
  const CameraScreen({super.key});

  @override
  State<CameraScreen> createState() => _CameraScreenState();
}

class _CameraScreenState extends State<CameraScreen> {
  static const String _cameraIpKey = 'camera_ip_address';

  String _cameraIp = '';
  bool _isStreaming = false;
  bool _isLoading = true;
  String? _error;
  final TextEditingController _ipController = TextEditingController();

  // Build stream URL from IP
  String get _streamUrl => 'http://$_cameraIp/stream';

  @override
  void initState() {
    super.initState();
    _loadCameraIp();
  }

  @override
  void dispose() {
    _ipController.dispose();
    super.dispose();
  }

  Future<void> _loadCameraIp() async {
    final prefs = await SharedPreferences.getInstance();
    final savedIp = prefs.getString(_cameraIpKey) ?? '';
    setState(() {
      _cameraIp = savedIp;
      _ipController.text = savedIp;
      _isLoading = false;
    });
  }

  Future<void> _saveCameraIp(String ip) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString(_cameraIpKey, ip);
    setState(() {
      _cameraIp = ip;
      _error = null;
    });
  }

  void _startStream() {
    if (_cameraIp.isEmpty) {
      _showSettingsDialog(context);
      return;
    }
    setState(() {
      _isStreaming = true;
      _error = null;
    });
  }

  void _stopStream() {
    setState(() {
      _isStreaming = false;
    });
  }

  void _refreshStream() {
    setState(() {
      _isStreaming = false;
      _error = null;
    });
    Future.delayed(const Duration(milliseconds: 300), () {
      setState(() {
        _isStreaming = true;
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      extendBodyBehindAppBar: true,
      appBar: AppBar(
        title: const Text('LIVE POND MONITOR'),
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
          IconButton(
            icon: const Icon(Icons.settings, color: Colors.white70),
            onPressed: () => _showSettingsDialog(context),
            tooltip: 'Camera Settings',
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
          child: _isLoading
              ? const Center(
                  child: CircularProgressIndicator(
                    valueColor: AlwaysStoppedAnimation<Color>(Color(0xFF14B8A6)),
                  ),
                )
              : Column(
                  children: [
                    // Camera View Area
                    Expanded(
                      child: Container(
                        margin: const EdgeInsets.all(16),
                        decoration: BoxDecoration(
                          color: Colors.black,
                          borderRadius: BorderRadius.circular(16),
                          border: Border.all(color: Colors.white.withOpacity(0.1)),
                        ),
                        child: ClipRRect(
                          borderRadius: BorderRadius.circular(15),
                          child: _buildCameraView(),
                        ),
                      ),
                    ),

                    // Controls
                    Padding(
                      padding: const EdgeInsets.all(16),
                      child: Column(
                        children: [
                          // Status indicator
                          Container(
                            padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
                            decoration: BoxDecoration(
                              color: const Color(0xFF1E293B),
                              borderRadius: BorderRadius.circular(12),
                              border: Border.all(color: Colors.white.withOpacity(0.08)),
                            ),
                            child: Row(
                              children: [
                                Container(
                                  width: 10,
                                  height: 10,
                                  decoration: BoxDecoration(
                                    shape: BoxShape.circle,
                                    color: _isStreaming
                                        ? (_error != null
                                            ? const Color(0xFFEF4444)
                                            : const Color(0xFF10B981))
                                        : const Color(0xFF6B7280),
                                  ),
                                ),
                                const SizedBox(width: 12),
                                Expanded(
                                  child: Column(
                                    crossAxisAlignment: CrossAxisAlignment.start,
                                    children: [
                                      Text(
                                        _isStreaming
                                            ? (_error != null ? 'Error' : 'Streaming')
                                            : 'Disconnected',
                                        style: const TextStyle(
                                          color: Colors.white,
                                          fontWeight: FontWeight.w600,
                                          fontSize: 13,
                                        ),
                                      ),
                                      Text(
                                        _cameraIp.isEmpty
                                            ? 'No camera configured'
                                            : _streamUrl,
                                        style: TextStyle(
                                          color: Colors.white.withOpacity(0.4),
                                          fontSize: 10,
                                          fontFamily: 'RobotoMono',
                                        ),
                                        overflow: TextOverflow.ellipsis,
                                      ),
                                    ],
                                  ),
                                ),
                                if (_cameraIp.isNotEmpty)
                                  TextButton(
                                    onPressed: _isStreaming ? _stopStream : _startStream,
                                    child: Text(
                                      _isStreaming ? 'STOP' : 'CONNECT',
                                      style: TextStyle(
                                        color: _isStreaming
                                            ? const Color(0xFFEF4444)
                                            : const Color(0xFF14B8A6),
                                        fontWeight: FontWeight.bold,
                                        fontSize: 12,
                                      ),
                                    ),
                                  ),
                              ],
                            ),
                          ),
                          const SizedBox(height: 12),

                          // Quick actions
                          Row(
                            children: [
                              Expanded(
                                child: _ActionButton(
                                  icon: Icons.refresh,
                                  label: 'REFRESH',
                                  enabled: _isStreaming && _error == null,
                                  onTap: _refreshStream,
                                ),
                              ),
                              const SizedBox(width: 12),
                              Expanded(
                                child: _ActionButton(
                                  icon: Icons.wifi_find,
                                  label: 'RESET WIFI',
                                  enabled: _cameraIp.isNotEmpty,
                                  onTap: () => _showResetWifiDialog(context),
                                ),
                              ),
                            ],
                          ),
                        ],
                      ),
                    ),
                  ],
                ),
        ),
      ),
    );
  }

  Widget _buildCameraView() {
    if (_cameraIp.isEmpty) {
      return _buildPlaceholder(
        icon: Icons.videocam_off_outlined,
        title: 'No Camera Configured',
        subtitle: 'Tap the settings icon to enter your\nESP32-CAM IP address',
      );
    }

    if (!_isStreaming) {
      return _buildPlaceholder(
        icon: Icons.videocam_outlined,
        title: 'Camera Ready',
        subtitle: 'Tap CONNECT to start streaming',
      );
    }

    // Use flutter_mjpeg to display the stream
    return Stack(
      fit: StackFit.expand,
      children: [
        Mjpeg(
          stream: _streamUrl,
          isLive: true,
          fit: BoxFit.cover,
          timeout: const Duration(seconds: 10),
          error: (context, error, stack) {
            // Update error state
            WidgetsBinding.instance.addPostFrameCallback((_) {
              if (mounted && _error == null) {
                setState(() {
                  _error = error.toString();
                });
              }
            });
            return _buildPlaceholder(
              icon: Icons.error_outline,
              title: 'Connection Failed',
              subtitle: 'Check if ESP32-CAM is powered on\nand connected to the same network',
              showRetry: true,
            );
          },
          loading: (context) => Center(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                const CircularProgressIndicator(
                  valueColor: AlwaysStoppedAnimation<Color>(Color(0xFF14B8A6)),
                  strokeWidth: 2,
                ),
                const SizedBox(height: 16),
                Text(
                  'Connecting to camera...',
                  style: TextStyle(color: Colors.white.withOpacity(0.6)),
                ),
                const SizedBox(height: 8),
                Text(
                  _cameraIp,
                  style: TextStyle(
                    color: Colors.white.withOpacity(0.4),
                    fontFamily: 'RobotoMono',
                    fontSize: 12,
                  ),
                ),
              ],
            ),
          ),
        ),
        // Live indicator
        Positioned(
          top: 12,
          right: 12,
          child: Container(
            padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 6),
            decoration: BoxDecoration(
              color: Colors.red.withOpacity(0.9),
              borderRadius: BorderRadius.circular(4),
            ),
            child: const Row(
              mainAxisSize: MainAxisSize.min,
              children: [
                Icon(Icons.fiber_manual_record, color: Colors.white, size: 10),
                SizedBox(width: 4),
                Text(
                  'LIVE',
                  style: TextStyle(
                    color: Colors.white,
                    fontWeight: FontWeight.bold,
                    fontSize: 10,
                    letterSpacing: 1,
                  ),
                ),
              ],
            ),
          ),
        ),
      ],
    );
  }

  Widget _buildPlaceholder({
    required IconData icon,
    required String title,
    required String subtitle,
    bool showRetry = false,
  }) {
    return Center(
      child: Padding(
        padding: const EdgeInsets.all(24),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Icon(icon, size: 64, color: Colors.white.withOpacity(0.2)),
            const SizedBox(height: 16),
            Text(
              title,
              style: TextStyle(
                color: Colors.white.withOpacity(0.6),
                fontSize: 16,
                fontWeight: FontWeight.w600,
              ),
            ),
            const SizedBox(height: 8),
            Text(
              subtitle,
              textAlign: TextAlign.center,
              style: TextStyle(
                color: Colors.white.withOpacity(0.4),
                fontSize: 12,
              ),
            ),
            if (showRetry) ...[
              const SizedBox(height: 20),
              ElevatedButton.icon(
                onPressed: _refreshStream,
                icon: const Icon(Icons.refresh, size: 18),
                label: const Text('RETRY'),
                style: ElevatedButton.styleFrom(
                  backgroundColor: const Color(0xFF14B8A6),
                  foregroundColor: Colors.white,
                  padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 12),
                  shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(8),
                  ),
                ),
              ),
            ],
          ],
        ),
      ),
    );
  }

  void _showSettingsDialog(BuildContext context) {
    _ipController.text = _cameraIp;
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        backgroundColor: const Color(0xFF1E293B),
        title: const Text(
          'CAMERA SETTINGS',
          style: TextStyle(
            color: Colors.white,
            fontSize: 14,
            letterSpacing: 1.0,
          ),
        ),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              'Enter the IP address of your ESP32-CAM.\nCheck Serial Monitor for the IP after WiFi connects.',
              style: TextStyle(color: Colors.white.withOpacity(0.5), fontSize: 12),
            ),
            const SizedBox(height: 16),
            TextField(
              controller: _ipController,
              style: const TextStyle(color: Colors.white, fontFamily: 'RobotoMono', fontSize: 14),
              decoration: InputDecoration(
                labelText: 'Camera IP Address',
                labelStyle: TextStyle(color: Colors.white.withOpacity(0.5), fontSize: 12),
                hintText: '192.168.1.100',
                hintStyle: TextStyle(color: Colors.white.withOpacity(0.2)),
                prefixIcon: Icon(Icons.wifi, color: Colors.white.withOpacity(0.4), size: 20),
                filled: true,
                fillColor: Colors.black.withOpacity(0.3),
                border: OutlineInputBorder(
                  borderRadius: BorderRadius.circular(8),
                  borderSide: BorderSide.none,
                ),
                enabledBorder: OutlineInputBorder(
                  borderRadius: BorderRadius.circular(8),
                  borderSide: BorderSide(color: Colors.white.withOpacity(0.1)),
                ),
                focusedBorder: OutlineInputBorder(
                  borderRadius: BorderRadius.circular(8),
                  borderSide: const BorderSide(color: Color(0xFF14B8A6)),
                ),
              ),
              keyboardType: TextInputType.number,
            ),
            const SizedBox(height: 12),
            Container(
              padding: const EdgeInsets.all(12),
              decoration: BoxDecoration(
                color: const Color(0xFF14B8A6).withOpacity(0.1),
                borderRadius: BorderRadius.circular(8),
                border: Border.all(color: const Color(0xFF14B8A6).withOpacity(0.3)),
              ),
              child: Row(
                children: [
                  const Icon(Icons.info_outline, color: Color(0xFF14B8A6), size: 18),
                  const SizedBox(width: 10),
                  Expanded(
                    child: Text(
                      'Stream URL will be:\nhttp://<IP>/stream',
                      style: TextStyle(
                        color: Colors.white.withOpacity(0.7),
                        fontSize: 11,
                        fontFamily: 'RobotoMono',
                      ),
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text('CANCEL', style: TextStyle(color: Colors.white54)),
          ),
          TextButton(
            onPressed: () async {
              final ip = _ipController.text.trim();
              if (ip.isNotEmpty) {
                await _saveCameraIp(ip);
                if (mounted) {
                  Navigator.pop(context);
                  _startStream();
                }
              }
            },
            child: const Text(
              'SAVE & CONNECT',
              style: TextStyle(color: Color(0xFF14B8A6), fontWeight: FontWeight.bold),
            ),
          ),
        ],
      ),
    );
  }

  void _showResetWifiDialog(BuildContext context) {
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        backgroundColor: const Color(0xFF1E293B),
        title: const Text(
          'RESET CAMERA WIFI',
          style: TextStyle(color: Colors.white, fontSize: 14, letterSpacing: 1.0),
        ),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              'To reset the camera\'s WiFi settings:',
              style: TextStyle(color: Colors.white.withOpacity(0.7), fontSize: 13),
            ),
            const SizedBox(height: 12),
            _buildStep('1', 'Open browser on your phone'),
            _buildStep('2', 'Go to: http://$_cameraIp/reset'),
            _buildStep('3', 'Camera will restart and create "OxyFeeder-CAM" hotspot'),
            _buildStep('4', 'Connect to hotspot and configure new WiFi'),
            const SizedBox(height: 12),
            Container(
              padding: const EdgeInsets.all(10),
              decoration: BoxDecoration(
                color: Colors.orange.withOpacity(0.1),
                borderRadius: BorderRadius.circular(8),
                border: Border.all(color: Colors.orange.withOpacity(0.3)),
              ),
              child: Row(
                children: [
                  const Icon(Icons.warning_amber, color: Colors.orange, size: 18),
                  const SizedBox(width: 8),
                  Expanded(
                    child: Text(
                      'You will lose connection after reset',
                      style: TextStyle(color: Colors.white.withOpacity(0.7), fontSize: 11),
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text('CLOSE', style: TextStyle(color: Colors.white54)),
          ),
        ],
      ),
    );
  }

  Widget _buildStep(String number, String text) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Container(
            width: 20,
            height: 20,
            decoration: BoxDecoration(
              color: const Color(0xFF14B8A6).withOpacity(0.2),
              shape: BoxShape.circle,
            ),
            child: Center(
              child: Text(
                number,
                style: const TextStyle(
                  color: Color(0xFF14B8A6),
                  fontSize: 11,
                  fontWeight: FontWeight.bold,
                ),
              ),
            ),
          ),
          const SizedBox(width: 10),
          Expanded(
            child: Text(
              text,
              style: TextStyle(
                color: Colors.white.withOpacity(0.6),
                fontSize: 12,
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class _ActionButton extends StatelessWidget {
  final IconData icon;
  final String label;
  final VoidCallback onTap;
  final bool enabled;

  const _ActionButton({
    required this.icon,
    required this.label,
    required this.onTap,
    this.enabled = true,
  });

  @override
  Widget build(BuildContext context) {
    return Material(
      color: Colors.transparent,
      child: InkWell(
        onTap: enabled ? onTap : null,
        borderRadius: BorderRadius.circular(10),
        child: Container(
          padding: const EdgeInsets.symmetric(vertical: 14),
          decoration: BoxDecoration(
            color: const Color(0xFF1E293B),
            borderRadius: BorderRadius.circular(10),
            border: Border.all(
              color: enabled
                  ? Colors.white.withOpacity(0.08)
                  : Colors.white.withOpacity(0.03),
            ),
          ),
          child: Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Icon(
                icon,
                color: enabled ? Colors.white54 : Colors.white24,
                size: 18,
              ),
              const SizedBox(width: 8),
              Text(
                label,
                style: TextStyle(
                  color: enabled ? Colors.white70 : Colors.white30,
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
