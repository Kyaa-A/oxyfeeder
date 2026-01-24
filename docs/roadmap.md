# OxyFeeder Master Guide: Complete Development Roadmap

This document outlines the complete development roadmap for the OxyFeeder project, tracking progress from initial app development through final deployment.

## 📊 **Overall Project Status: 60% Complete**

---

## **Phase 1: App Development (Software)**
**Goal:** A fully designed, architecturally sound mobile application ready to connect to hardware.  
**Status:** ✅ **COMPLETE**

### **Sub-Phase 1A: UI/UX Foundation**
**Goal:** Create a visually complete, non-functional app shell.  
**Status:** ✅ **COMPLETE**

**Detailed Steps:**
1. ✅ **Build Static Dashboard:** Create `dashboard_screen.dart` with `Scaffold` and three `SensorCard` widgets displaying hardcoded placeholder data for DO, Feed Level, and Battery.
2. ✅ **Build Static Settings:** Create `settings_screen.dart` with `Scaffold` and UI elements like sliders and switches, all with hardcoded placeholder values.
3. ✅ **Build Placeholder Sensors:** Create `sensors_screen.dart` with a simple `Scaffold` and a `Center` widget showing "Sensors Screen".
4. ✅ **Implement Navigation:** Create a main navigation host widget (`main_nav_host.dart`) containing a `BottomNavigationBar` to switch between the three screens. Update `main.dart` to use this host as the home.

### **Sub-Phase 1B: Architectural Foundation & State Management**
**Goal:** Make the UI dynamic and ready for data by separating UI from business logic.  
**Status:** ✅ **COMPLETE**

**Detailed Steps:**
1. ✅ **Create Data Models:** Create `oxyfeeder_status.dart` and `app_settings.dart` in `lib/core/models/` to define the structure of your app's data.
2. ✅ **Add Provider Package:** Run `flutter pub add provider` to add the state management library.
3. ✅ **Create ViewModels:** Create `DashboardViewModel` and `SettingsViewModel` in their respective feature folders. Make them use `ChangeNotifier`.
4. ✅ **Integrate Provider:** In `main.dart`, wrap the `MaterialApp` with a `MultiProvider` that provides both the `DashboardViewModel` and `SettingsViewModel`.
5. ✅ **Refactor UI:** Update the Dashboard and Settings screens to use `Consumer` widgets, connecting their UI elements to the data and methods in their respective ViewModels.

### **Sub-Phase 1C: Final UI and Mock Data Simulation**
**Goal:** Create a fully dynamic, self-contained app that simulates real-world behavior.  
**Status:** ✅ **COMPLETE**

**Detailed Steps:**
1. ✅ **Add Charting Package:** Run `flutter pub add fl_chart`.
2. ✅ **Build Sensors UI:** In `sensors_screen.dart`, build the detailed UI with a status section and a LineChart using static sample data.
3. ✅ **Create Sensors ViewModel:** Create `SensorsViewModel` and add it to the `MultiProvider`.
4. ✅ **Create Mock Service:** Create `mock_bluetooth_service.dart` in `lib/core/services/`. This class contains a `Timer` that emits a stream of random `OxyFeederStatus` objects.
5. ✅ **Centralize Data Source:** Add the `MockBluetoothService` to the `MultiProvider` (before the ViewModels).
6. ✅ **Refactor ViewModels:** Remove all internal timers from the ViewModels. Instead, make them listen to the stream from the `MockBluetoothService` and update their state based on the data they receive.

---

## **Phase 2: Core Hardware & Firmware Integration**
**Goal:** Create a functional, stand-alone 'Communication Hub' that broadcasts simulated data over Bluetooth.  
**Status:** 🔄 **IN PROGRESS**

### **Sub-Phase 2A: Firmware Preparation**
**Goal:** Have all necessary microcontroller code ready before physical assembly.  
**Status:** ✅ **COMPLETE**

**Detailed Steps:**
1. ✅ **Generate Arduino Scaffold:** Create `firmware/oxyfeeder_firmware/oxyfeeder_firmware.ino` with simulated sensor and motor functions.
2. ✅ **Redirect Arduino Output:** Modify the scaffold to change all `Serial.print` calls (for the JSON packet) to `Serial1.print`, directing the data to the hardware pins for the ESP32.
3. ✅ **Generate ESP32 Firmware:** Create `firmware/esp32_communicator/esp32_communicator.ino`. This code listens on its `Serial2` port and sets up a complete BLE server to advertise the data it receives.

### **Sub-Phase 2B: Communication Hub Assembly**
**Goal:** A permanently soldered, reliable electronic 'brain' for the project.  
**Status:** 🎯 **YOUR CURRENT TASK**

**Detailed Steps:**
1. ⏳ **Gather Materials:** Collect all items from the list: Arduino Mega, ESP32, Logic Level Shifter, Perfboard, soldering tools, and wires.
2. ⏳ **Plan Layout:** Arrange the three components on the perfboard for efficient wiring.
3. ⏳ **Solder Sockets:** Solder male header pins onto the perfboard where the components will sit.
4. ⏳ **Mount Components:** Plug the boards into their sockets.
5. ⏳ **Solder Power Connections:** Create permanent 5V and GND rails and wire the power pins for all three components (Arduino 5V, ESP32 Vin, Shifter HV & LV).
6. ⏳ **Solder Serial Data Connections:** Solder the four crucial data wires, connecting Arduino (TX1, RX1) to ESP32 (RX2, TX2) through the Logic Level Shifter.

### **Sub-Phase 2C: Hub Testing & Verification**
**Goal:** Prove the hardware and firmware work together perfectly before connecting the app.  
**Status:** ⏳ **TO DO**

**Detailed Steps:**
1. ⏳ **Flash Arduino:** Connect ONLY the Arduino to your PC. Open `oxyfeeder_firmware.ino` in the Arduino IDE, select the correct Board/Port, and click Upload.
2. ⏳ **Flash ESP32:** Disconnect the Arduino. Connect ONLY the ESP32. Open `esp32_communicator.ino`, select the correct Board/Port, and click Upload.
3. ⏳ **Power System:** Connect both boards to USB power simultaneously.
4. ⏳ **Monitor Arduino:** Open an Arduino IDE instance, select the Arduino's COM port, and open the Serial Monitor (9600 baud) to see its debug messages.
5. ⏳ **Monitor ESP32:** Open a second Arduino IDE instance, select the ESP32's COM port, and open the Serial Monitor (9600 baud). You should see the JSON data being received from the Arduino.
6. ⏳ **Verify BLE Broadcast:** On your smartphone, use a BLE scanner app (e.g., nRF Connect). Scan for a device named 'OxyFeeder'. Connect to it, find the characteristic, and confirm that it contains the correct JSON string.

---

## **Phase 3: App-to-Hardware Connection (The "Magic Moment")**
**Goal:** Prove the end-to-end communication chain works by connecting the Flutter app to the physical hardware.  
**Status:** ⏳ **TO DO**

### **Sub-Phase 3A: Implement the Real Bluetooth Service**
**Goal:** Build the software bridge to the real hardware.  
**Status:** ⏳ **TO DO**

**Detailed Steps:**
1. ⏳ **Add Package:** Run `flutter pub add flutter_blue_plus`.
2. ⏳ **Create Service File:** Create `real_bluetooth_service.dart` in `lib/core/services/`.
3. ⏳ **Implement Scanning:** Write a method that starts a BLE scan and looks for devices with the name "OxyFeeder".
4. ⏳ **Implement Connection:** Write a method that connects to the found device.
5. ⏳ **Implement Data Subscription:** Write logic that, upon connection, discovers the correct service and characteristic (using the UUIDs from your ESP32 firmware) and subscribes to its notifications.
6. ⏳ **Implement Data Parsing:** When data is received, parse the JSON string, create an `OxyFeederStatus` object, and add it to a public `statusStream`.

### **Sub-Phase 3B: Integrate the Real Service into the App**
**Goal:** "Plug in" the real hardware connection to the app's architecture.  
**Status:** ⏳ **TO DO**

**Detailed Steps:**
1. ⏳ **Navigate to `main.dart`:** Open the file in Cursor.
2. ⏳ **Swap Providers:** In the `MultiProvider` list, comment out the line that provides the `MockBluetoothService`.
3. ⏳ **Add Real Provider:** Add a new line that provides the `RealBluetoothService`.

### **Sub-Phase 3C: End-to-End "Smoke Test"**
**Goal:** The "Magic Moment" - prove the entire communication chain works.  
**Status:** ⏳ **TO DO**

**Detailed Steps:**
1. ⏳ **Power Hardware:** Ensure your physical "Communication Hub" is powered on.
2. ⏳ **Run App on Phone:** Run the Flutter app on a physical smartphone (not an emulator).
3. ⏳ **Trigger Connection:** Implement and use a "Connect" button in your app's UI (e.g., on the Settings screen) to call the `connect()` method in your new service.
4. ⏳ **Verify Live Data:** Watch the app's dashboard and confirm that it is now displaying the simulated JSON data being broadcast from your physical hardware.

---

## **Phase 4: Full System Assembly (Final Hardware Build)**
**Goal:** A fully assembled, self-contained, and enclosed OxyFeeder unit.  
**Status:** ⏳ **TO DO**

### **Sub-Phase 4A: Component Mounting & Wiring**
**Goal:** Create a clean and robust physical build.  
**Status:** ⏳ **TO DO**

**Detailed Steps:**
1. ⏳ **Gather Materials:** Collect all remaining items: Solar Panel, Battery, MPPT Controller, Sensors, Motor, etc.
2. ⏳ **Mount Components:** Mount the "Communication Hub" and all other components securely inside the waterproof enclosure.
3. ⏳ **Wire the Power System:** Wire the Battery, Solar Panel, and your main system to the correct terminals on the MPPT Charge Controller, including fuses and a master power switch.
4. ⏳ **Wire Sensors & Motor:** Wire all sensors (DO, RTC, HX711, Voltage) and the motor driver to the correct pins on the Arduino Mega.

---

## **Phase 5: Final Firmware Integration (The "Real Data" Upgrade)**
**Goal:** Upgrade the firmware to read from the real world instead of simulating data.  
**Status:** ⏳ **TO DO**

### **Sub-Phase 5A: Firmware Refactoring**
**Goal:** Make the firmware's "brain" interact with its "senses".  
**Status:** ⏳ **TO DO**

**Detailed Steps:**
1. ⏳ **Add Libraries:** In `oxyfeeder_firmware.ino`, add the real hardware libraries for all your sensors (DFRobot_DO, HX711, DS3231).
2. ⏳ **Replace Placeholder Functions:** Replace the code in each placeholder function (`readDissolvedOxygen`, `controlFeederMotor`, etc.) with the real code that interacts with the hardware.
3. ⏳ **Implement Real Scheduling:** Implement the scheduling logic using the DS3231 RTC to trigger feeding.

### **Sub-Phase 5B: Final Firmware Flash**
**Goal:** Load the final, intelligent program onto the device.  
**Status:** ⏳ **TO DO**

**Step 1:** Upload this final, complete `oxyfeeder_firmware.ino` to the Arduino Mega inside your fully assembled unit.

---

## **Phase 6: Final System Deployment & Testing (The Finish Line)**
**Goal:** A fully operational, field-tested OxyFeeder system.  
**Status:** ⏳ **TO DO**

### **Sub-Phase 6A: Field Deployment and The Ultimate Test**
**Goal:** Verify the system works in a real-world environment.  
**Status:** ⏳ **TO DO**

**Detailed Steps:**
1. ⏳ **Deploy:** Place the OxyFeeder unit at its operational location.
2. ⏳ **Power On:** Turn on the system with the master switch.
3. ⏳ **Connect:** Open the Flutter app and connect to the device.
4. ⏳ **Verify Live Data:** Confirm that the dashboard displays live, real-world data.
5. ⏳ **Test Actuation:** Use the app to set a feeding schedule and watch the physical motor activate at the correct time.
6. ⏳ **Congratulations, your project is complete.**

---

## **📋 Quick Status Summary**

| Phase | Status | Progress |
|-------|--------|----------|
| Phase 1: App Development | ✅ Complete | 100% |
| Phase 2: Hardware & Firmware | ✅ Complete | 100% |
| Phase 3: App-Hardware Connection | ✅ Complete | 100% |
| Phase 4: Full System Assembly | ✅ Complete | 100% |
| Phase 5: Real Data Integration | ✅ Complete | 100% |
| Phase 6: Deployment & Testing | 🎯 NEXT | 0% |

**Next Immediate Task:** Deploy to field, calibrate sensors, and run full system test
