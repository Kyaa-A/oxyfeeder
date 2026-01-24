import 'package:flutter/material.dart';
import '../../dashboard/view/dashboard_screen.dart';
import '../../sensors/view/sensors_screen.dart';
import '../../settings/view/settings_screen.dart';

class MainNavHost extends StatefulWidget {
  const MainNavHost({super.key});

  @override
  State<MainNavHost> createState() => _MainNavHostState();
}

class _MainNavHostState extends State<MainNavHost> {
  int _currentIndex = 0;

  void _navigateToTab(int index) {
    setState(() => _currentIndex = index);
  }

  List<Widget> get _screens => <Widget>[
    DashboardScreen(onNavigateToSensors: () => _navigateToTab(1)),
    const SensorsScreen(),
    const SettingsScreen(),
  ];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: _screens[_currentIndex],
      bottomNavigationBar: BottomNavigationBar(
        currentIndex: _currentIndex,
        onTap: (int index) {
          setState(() => _currentIndex = index);
        },
        backgroundColor: const Color(0xFF0F172A), // Slate 900
        selectedItemColor: const Color(0xFF14B8A6), // Teal 500
        unselectedItemColor: Colors.white38,
        type: BottomNavigationBarType.fixed,
        selectedLabelStyle: const TextStyle(fontWeight: FontWeight.w600, fontSize: 12),
        unselectedLabelStyle: const TextStyle(fontSize: 11),
        items: const <BottomNavigationBarItem>[
          BottomNavigationBarItem(
            icon: Icon(Icons.dashboard_outlined),
            activeIcon: Icon(Icons.dashboard),
            label: 'Dashboard',
          ),
          BottomNavigationBarItem(
            icon: Icon(Icons.sensors_outlined),
            activeIcon: Icon(Icons.sensors),
            label: 'Sensors',
          ),
          BottomNavigationBarItem(
            icon: Icon(Icons.settings_outlined),
            activeIcon: Icon(Icons.settings),
            label: 'Settings',
          ),
        ],
      ),
    );
  }
}


