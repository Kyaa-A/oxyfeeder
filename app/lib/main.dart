import 'package:flutter/material.dart';
import 'features/navigation/view/main_nav_host.dart';
import 'package:provider/provider.dart';
import 'features/dashboard/viewmodel/dashboard_viewmodel.dart';
import 'features/settings/viewmodel/settings_viewmodel.dart';
import 'features/sensors/viewmodel/sensors_viewmodel.dart';
// import 'core/services/mock_bluetooth_service.dart'; // Mock data for UI testing
import 'core/services/real_bluetooth_service.dart'; // Real hardware
import 'core/services/event_log_service.dart'; // Event history

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  runApp(const OxyFeederApp());
}

class OxyFeederApp extends StatelessWidget {
  const OxyFeederApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MultiProvider(
      providers: [
        // ============================================================
        // Real Bluetooth Service - ACTIVE
        // ============================================================
        Provider<RealBluetoothService>(
          create: (_) => RealBluetoothService(),
          dispose: (_, svc) => svc.dispose(),
        ),
        // Event Log Service
        ChangeNotifierProvider<EventLogService>(
          create: (_) {
            final svc = EventLogService();
            svc.init();
            return svc;
          },
        ),
        ChangeNotifierProvider<DashboardViewModel>(
          create: (ctx) => DashboardViewModel(ctx.read<RealBluetoothService>()),
        ),
        ChangeNotifierProvider<SensorsViewModel>(
          create: (ctx) => SensorsViewModel(ctx.read<RealBluetoothService>()),
        ),
        ChangeNotifierProvider<SettingsViewModel>(
          create: (ctx) {
            final vm = SettingsViewModel();
            vm.setBluetoothService(ctx.read<RealBluetoothService>());
            vm.setEventLogService(ctx.read<EventLogService>());
            vm.init(); // Load persistent settings
            return vm;
          },
        ),
      ],
      child: MaterialApp(
        debugShowCheckedModeBanner: false,
        themeMode: ThemeMode.dark,
        theme: ThemeData(
          useMaterial3: true,
          brightness: Brightness.dark,
          scaffoldBackgroundColor: const Color(0xFF0F172A), // Slate 900
          cardColor: const Color(0xFF1E293B), // Slate 800
          colorScheme: const ColorScheme.dark(
            primary: Color(0xFF14B8A6), // Teal 500
            secondary: Color(0xFF6366F1), // Indigo 500
            surface: Color(0xFF1E293B), // Slate 800
            surfaceContainerHighest: Color(0xFF334155), // Slate 700
            onSurface: Color(0xFFF8FAFC), // Slate 50
            error: Color(0xFFEF4444), // Red 500
          ),
          appBarTheme: const AppBarTheme(
            backgroundColor: Colors.transparent,
            elevation: 0,
            centerTitle: true,
            titleTextStyle: TextStyle(
              fontSize: 20,
              fontWeight: FontWeight.bold,
              letterSpacing: 0.5,
              color: Colors.white,
            ),
          ),
          textTheme: const TextTheme(
            headlineMedium: TextStyle(fontWeight: FontWeight.w800, color: Colors.white),
            titleLarge: TextStyle(fontWeight: FontWeight.w700, color: Colors.white),
            titleMedium: TextStyle(fontWeight: FontWeight.w600, color: Color(0xFFF1F5F9)),
            bodyLarge: TextStyle(fontWeight: FontWeight.normal, color: Color(0xFFCBD5E1)),
          ),
        ),
        home: const MainNavHost(),
      ),
    );
  }
}
