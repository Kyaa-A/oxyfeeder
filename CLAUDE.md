# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

OxyFeeder is an IoT aquaculture monitoring and feeding system consisting of:
- **Flutter mobile app** for monitoring dissolved oxygen, feed level, battery status, and controlling feeding schedules
- **Arduino Mega firmware** for sensor reading and motor control
- **ESP32 BLE bridge** for wireless communication between hardware and mobile app
- **ESP32-CAM server** (optional) for live video streaming of the fishpond

The system monitors fishpond conditions and automates fish feeding based on schedules.

## Development Environment Setup

This project uses a **dual-platform development approach**:

### WSL/Linux (For Fast UI Development)
```bash
# Run app on Linux desktop with WSLg (fast, no emulator needed)
cd /home/asnari/Project/OxyFeeder/app
$HOME/flutter/bin/flutter run -d linux
```

### Windows (For Android Testing)
```powershell
# Run app on real Android device via USB
cd "C:\Users\Asnari Pacalna\Project\OxyFeeder\app"
flutter run
```

**Key insight**: Use Linux desktop mode for 90% of UI development (instant hot reload, no lag). Only switch to Android device for Bluetooth testing with real ESP32 hardware.

## Flutter App Commands

All Flutter commands must be run from the `app/` directory:

```bash
# Install/update dependencies
flutter pub get

# Run on specific device
flutter devices                    # List available devices
flutter run -d linux              # Linux desktop (WSLg)
flutter run -d <device_id>        # Specific Android device

# Build for production
flutter build apk                 # Android APK for deployment
flutter build appbundle           # Android App Bundle for Play Store

# Troubleshooting
flutter doctor                    # Check environment setup
flutter doctor -v                 # Verbose diagnostics
flutter clean                     # Clean build artifacts
```

## Architecture

### Flutter App Structure (MVVM Pattern)

The app follows **Model-View-ViewModel (MVVM)** architecture with **Provider** for state management:

```
app/lib/
├── core/
│   ├── models/           # Data models (OxyFeederStatus, AppSettings)
│   └── services/         # Business logic (MockBluetoothService, future: RealBluetoothService)
└── features/
    ├── dashboard/        # Main screen showing DO, feed, battery status
    ├── sensors/          # Detailed sensor view with historical charts
    ├── settings/         # App configuration and feeding schedules
    └── navigation/       # Bottom navigation host
```

**Key architectural decisions**:
- **Centralized data source**: `MockBluetoothService` (or `RealBluetoothService`) provides a single `statusStream` that all ViewModels consume
- **No direct UI-to-service coupling**: ViewModels subscribe to the service stream and expose data to UI via `ChangeNotifier`
- **Provider dependency injection**: Services are provided before ViewModels in `main.dart` so ViewModels can access them via `context.read<>()`

### Data Flow

```
Hardware → ESP32 (BLE) → BluetoothService → Stream<OxyFeederStatus> → ViewModels → UI
```

Currently using `MockBluetoothService` for development (Phase 1 complete). Future Phase 3 will swap in `RealBluetoothService` with `flutter_blue_plus` package.

### State Management Pattern

In `main.dart`, providers are ordered intentionally:
1. Services first (e.g., `MockBluetoothService`)
2. ViewModels second (can access services via `context.read<>()`)

Example ViewModel consuming service:
```dart
class DashboardViewModel extends ChangeNotifier {
  DashboardViewModel(MockBluetoothService service) {
    service.statusStream.listen((status) {
      // Update local state and call notifyListeners()
    });
  }
}
```

## Hardware Architecture

### Communication Hub (The "Brain")
The system uses the **"Bridge Method"** - a Logic Level Shifter on a small perfboard acts as a bridge between Arduino and ESP32:

**Arduino Mega** (with Female Headers):
- TX1 (Pin 18) → Logic Shifter HV1
- RX1 (Pin 19) → Logic Shifter HV2
- 5V → Logic Shifter HV
- GND → Logic Shifter GND

**Logic Level Shifter Bridge** (on perfboard):
- Converts 5V (Arduino) ↔ 3.3V (ESP32)
- HV side connects to Arduino via female headers
- LV side connects to ESP32 Extender via screw terminals

**ESP32** (with Screw Terminal Extender):
- RX2 (Pin 17) ← Logic Shifter LV1
- TX2 (Pin 16) ← Logic Shifter LV2
- 3V3 ← Logic Shifter LV
- GND ← Logic Shifter GND
- VIN ← Arduino 5V (powered by Arduino)

### BLE Communication

**ESP32 BLE Bridge** (`firmware/esp32_communicator/`):
- Listens on `Serial2` for JSON data from Arduino Mega
- Service UUID: `0000abcd-0000-1000-8000-00805f9b34fb`
- Characteristic UUID: `0000abce-0000-1000-8000-00805f9b34fb`
- Device name: "OxyFeeder"
- Broadcasts data via BLE notifications

### Arduino Firmware

**Current State** (`firmware/oxyfeeder_firmware/`):
- Sends **simulated** sensor data as JSON via `Serial1`
- JSON format: `{"do": 6.5, "feed": 75, "battery": 82}`

**Future State** (Phase 5 - Real Sensors):
Will read from actual hardware with these pin mappings:

**Sensors:**
- DO Sensor (Analog): A1
- Voltage Sensor (Battery monitor): A0
- RTC (DS3231): SDA → Pin 20, SCL → Pin 21
- Load Cell (HX711): DT → Pin 8, SCK → Pin 9
- LCD Display: SDA → Pin 20, SCL → Pin 21 (shared I2C bus with RTC)

**Actuators:**
- Motor Driver: IN1 → Pin 10, IN2 → Pin 11, ENA → Pin 12
- Servo: PWM → Pin 6

**Required Libraries** (Phase 5):
- `DFRobot_OxygenSensor` - DO sensor
- `RTClib` - Real-time clock
- `HX711` - Load cell amplifier
- `Adafruit_GFX` - LCD display

### Communication Protocol
- Arduino → ESP32: Hardware serial (`Serial1` on Arduino, `Serial2` on ESP32) through logic level shifter
- ESP32 → Mobile App: BLE notifications on characteristic
- Data format: JSON string with `do`, `feed`, `battery` fields

### Power System (Phase 4)

**Power Flow:**
```
Solar Panel → MPPT Controller → Battery
              MPPT Load Output → Fuse → Master Switch → 12V Fuse Box
                                                        ├→ Arduino Vin (powers both Arduino + ESP32)
                                                        └→ Motor Driver 12V
```

**Components:**
- Solar Panel: Charges battery via MPPT controller
- MPPT Charge Controller: Manages charging and provides regulated load output
- Battery: Main power storage
- Master Switch: System on/off control
- 12V Fuse Box: Distributes power to subsystems with individual fuse protection
- Arduino powers ESP32 via 5V connection (VIN on ESP32 Extender)

## Project Development Phases (Master Blueprint)

### Part 1: Software Foundation ✅ COMPLETE
- UI/UX built (Dashboard, Settings, Sensors screens)
- MVVM architecture with Provider state management
- App runs dynamically with `MockBluetoothService`

### Part 2: Communication Hub (Current Focus - Phase 2B)
**Status:** Building the "Bridge"

**2A - Firmware Preparation:** ✅ COMPLETE
- `oxyfeeder_firmware.ino` - prints JSON to Serial1
- `esp32_communicator.ino` - listens on Serial2, broadcasts BLE

**2B - Physical Assembly:** 🔄 IN PROGRESS
Build Logic Level Shifter "Bridge" board:
1. Solder shifter to small perfboard
2. Connect HV side to Arduino (via female headers): 5V, GND, TX1, RX1
3. Connect LV side to ESP32 Extender (via screw terminals): 3V3, GND, RX2, TX2
4. Power ESP32 from Arduino 5V

**2C - Hub Testing:** ⏳ NEXT
- Flash both firmwares via USB
- Monitor Serial output (should see JSON on ESP32)
- Use nRF Connect app to find "OxyFeeder" BLE device

### Part 3: App-to-Hardware Connection ⏳ TO DO
**3A - Real Bluetooth Service:**
- Add `flutter_blue_plus` package
- Create `real_bluetooth_service.dart`
- Scan for "OxyFeeder", subscribe to characteristic

**3B - The Swap:**
- Comment out `MockBluetoothService` in `main.dart`
- Provide `RealBluetoothService` instead
- Test live data from hardware on phone

### Part 4: Final Assembly ⏳ TO DO
**4A - Mounting:** Install all components in waterproof enclosure with cable glands

**4B - Power Wiring:**
- Solar Panel → MPPT Controller → Battery
- MPPT Load → Fuse → Switch → 12V Fuse Box
- Distribute to Arduino (via Vin) and Motor Driver

**4C - Sensor Wiring:** Connect all sensors to Arduino pins (see Hardware Architecture)

**4D - Actuator Wiring:** Connect Motor Driver and Servo

### Part 5: Real Firmware Integration ⏳ TO DO
- Install Arduino libraries (DFRobot_OxygenSensor, RTClib, HX711, Adafruit_GFX)
- Replace mock functions with real sensor reads
- Implement RTC-based scheduling
- Flash to Arduino Mega

### Part 6: Deployment ⏳ TO DO
- Calibrate sensors (DO sensor, load cell)
- Seal enclosure and deploy at pond
- Test end-to-end system

## Testing on Real Device

**Android via USB**:
1. Enable Developer Options + USB Debugging on phone
2. Connect via USB cable, select "File Transfer" mode
3. Allow USB debugging popup
4. In Windows PowerShell: `adb devices` → `flutter run`

**Bluetooth Testing** (Phase 3):
- Cannot test Bluetooth on emulator or Linux desktop
- Must use real Android phone with physical ESP32 hardware
- Use BLE scanner apps (e.g., nRF Connect) for initial BLE verification

## Important File Locations

- **App entry point**: `app/lib/main.dart`
- **Data model**: `app/lib/core/models/oxyfeeder_status.dart`
- **Mock service**: `app/lib/core/services/mock_bluetooth_service.dart`
- **Arduino firmware**: `firmware/oxyfeeder_firmware/oxyfeeder_firmware.ino`
- **ESP32 BLE firmware**: `firmware/esp32_communicator/esp32_communicator.ino`
- **ESP32-CAM firmware**: `firmware/camera_server/camera_server.ino`
- **Development guide**: `docs/development_cheatsheet.md`
- **Project roadmap**: `docs/roadmap.md`

## Common Workflows

### Adding New Sensor Data
1. Update `OxyFeederStatus` model in `core/models/oxyfeeder_status.dart`
2. Update JSON parsing in `fromJson()` method
3. Update Arduino firmware to include new field in JSON output
4. Update `MockBluetoothService` to generate mock data for new field
5. Update UI in relevant screen (Dashboard, Sensors, etc.)

### Swapping Mock → Real Bluetooth (Phase 3)
1. Add package: `flutter pub add flutter_blue_plus`
2. Create `lib/core/services/real_bluetooth_service.dart`
3. Implement scanning for "OxyFeeder" device
4. Subscribe to characteristic notifications with UUIDs from ESP32 firmware
5. Parse JSON and emit to `statusStream`
6. In `main.dart`: Comment out `MockBluetoothService`, provide `RealBluetoothService` instead

### Implementing Real Sensors (Phase 5)
When upgrading from simulated to real sensor data:

1. **Install Arduino Libraries** (via Arduino IDE Library Manager):
   - DFRobot_OxygenSensor
   - RTClib
   - HX711
   - Adafruit_GFX

2. **Update Firmware Code:**
   - Replace `readDissolvedOxygen()` mock with `analogRead(A1)` + DFRobot library
   - Replace `readBatteryVoltage()` mock with `analogRead(A0)` + voltage divider math
   - Replace `readFeedLevel()` mock with HX711 library reading
   - Add RTC scheduling logic using RTClib
   - Implement motor control functions for feed dispensing

3. **Pin Mappings Reference:**
   - See "Hardware Architecture" section for complete pin assignments
   - Ensure I2C devices (RTC, LCD) share SDA/SCL bus correctly

4. **Flash and Test:**
   - Upload to Arduino Mega
   - Monitor serial output for JSON format consistency
   - Verify ESP32 still receives and broadcasts data correctly

### Flashing Firmware

**Arduino Mega:**
- Open `firmware/oxyfeeder_firmware/oxyfeeder_firmware.ino` in Arduino IDE
- Select board: "Arduino Mega or Mega 2560"
- Select correct COM port
- Upload

**ESP32 (BLE Bridge):**
- Open `firmware/esp32_communicator/esp32_communicator.ino` in Arduino IDE
- Select board: "ESP32 Dev Module" (or appropriate variant)
- Select correct COM port
- Upload

**ESP32-CAM (Camera Server):**
- Connect FTDI programmer: 5V→5V, GND→GND, TX→U0R, RX→U0T, IO0→GND
- Open `firmware/camera_server/camera_server.ino` in Arduino IDE
- Update `WIFI_SSID` and `WIFI_PASSWORD` in the code
- Select board: "AI Thinker ESP32-CAM"
- Select correct COM port, click Upload
- After upload: Disconnect IO0 from GND, press RESET
- Open Serial Monitor (115200 baud) to see the camera's IP address
- Access stream at: `http://<IP_ADDRESS>/stream`

**Testing Hub After Flash:**
1. Connect both devices to PC via USB
2. Open Serial Monitor for ESP32 (9600 baud)
3. Should see: `Received: {"do": X.X, "feed": XX, "battery": XX}`
4. Use nRF Connect app to verify BLE broadcast
