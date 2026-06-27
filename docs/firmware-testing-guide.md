# 🎓 Firmware & Testing Guide - Complete Beginner's Guide

**Purpose:** Step-by-step instructions for uploading firmware and testing your OxyFeeder hardware after Phase 2B assembly.

**When to use this:** After you've built the Communication Hub (Logic Level Shifter bridge connecting Arduino + ESP32)

---

## 📖 Part 1: What is Firmware? (Simple Explanation)

**Think of it like this:**

- **Hardware** = The physical brain (Arduino, ESP32 chips)
- **Firmware** = The instructions that tell the brain what to do
- **Without firmware** = Your Arduino/ESP32 are just expensive paperweights!

**Real-world analogy:**
- Your **microwave** has a chip inside (hardware)
- The chip runs **firmware** that knows: "When user presses 2 minutes, turn on magnetron for 120 seconds"
- Without that firmware, the buttons do nothing

**In your OxyFeeder:**
- Arduino firmware says: "Read sensors every 5 seconds, send data to ESP32"
- ESP32 firmware says: "Take data from Arduino, broadcast it via Bluetooth"

---

## 🔧 Part 2: What Each Firmware Does

### Arduino Firmware (`oxyfeeder_firmware.ino`)

**Job:** The "Brain" - Controls everything

**What it does every loop (every 5 seconds):**
```
1. READ sensors (DO, feed level, battery)
2. PROCESS data (check if feeding time)
3. OUTPUT data as JSON to ESP32 via Serial1
```

**Example output:**
```json
{"do": 7.8, "feed": 62, "battery": 85}
```

**Current state:** Uses fake/simulated data (Phase 2)
**Future state:** Will read real sensors (Phase 5)

---

### ESP32 Firmware (`esp32_communicator.ino`)

**Job:** The "Messenger" - Wireless communication bridge

**What it does:**
```
1. LISTEN to Serial2 for data from Arduino
2. RECEIVE JSON string
3. BROADCAST via Bluetooth to your phone
```

**Think of it as:** A walkie-talkie that translates Arduino's language into Bluetooth signals your phone understands.

---

## 💻 Part 3: How to Upload Firmware (Step-by-Step)

### What You Need:
- [ ] Arduino IDE installed on your computer
- [ ] 2 USB cables (one for Arduino, one for ESP32)
- [ ] Arduino Mega board
- [ ] ESP32 board

---

### 🔵 **Step 1: Upload Arduino Firmware**

#### A. Open Arduino IDE

#### B. Open the firmware file
- Click: **File → Open**
- Navigate to: `OxyFeeder/firmware/oxyfeeder_firmware/oxyfeeder_firmware.ino`
- Click **Open**

#### C. Select the board
- Click: **Tools → Board → Arduino AVR Boards → Arduino Mega or Mega 2560**

#### D. Plug in Arduino
- Connect Arduino Mega to your computer with USB cable
- Wait for Windows to recognize it (you'll hear a "ding" sound)

#### E. Select the port
- Click: **Tools → Port**
- Select the port that says something like: **COM3 (Arduino Mega)** or **/dev/ttyACM0**
- **Tip:** If you see multiple ports and don't know which one, unplug Arduino, note which ports disappear, then plug back in

#### F. Upload!
- Click the **Upload** button (→ arrow icon) in Arduino IDE
- Wait for "Done uploading" message
- You should see Arduino's onboard LED blink during upload

#### G. Verify it's working
- Click: **Tools → Serial Monitor**
- Set baud rate to **9600** (bottom right corner dropdown)
- You should see:
  ```
  OxyFeeder Firmware Initializing...
  USB Serial: Ready for debug messages
  Hardware Serial1: Ready for ESP32 communication
  Setup complete - entering main loop
  ```

**✅ Success!** Arduino is now running your firmware!

---

### 🟢 **Step 2: Upload ESP32 Firmware**

#### A. Install ESP32 board support (one-time setup)

**If you haven't done this before:**

1. Click: **File → Preferences**
2. In "Additional Board Manager URLs", paste:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Click **OK**
4. Click: **Tools → Board → Boards Manager**
5. Search for "**ESP32**"
6. Install "**ESP32 by Espressif Systems**"
7. Wait for installation to complete

#### B. Open ESP32 firmware
- Click: **File → Open**
- Navigate to: `OxyFeeder/firmware/esp32_communicator/esp32_communicator.ino`
- Click **Open**

#### C. Select ESP32 board
- Click: **Tools → Board → ESP32 Arduino → ESP32 Dev Module**
- (If you know your specific ESP32 model, select that instead)

#### D. Plug in ESP32
- **IMPORTANT:** Unplug Arduino first! (Only one device at a time during upload)
- Connect ESP32 to computer with USB cable
- Wait for recognition

#### E. Select the port
- Click: **Tools → Port**
- Select the new port (probably different from Arduino's port)
- Example: **COM4** or **/dev/ttyUSB0**

#### F. Upload!
- Click the **Upload** button
- **ESP32 note:** You might need to press and hold the "BOOT" button on your ESP32 when you see "Connecting..." in the Arduino IDE output
- Release button when upload starts
- Wait for "Done uploading"

#### G. Verify it's working
- Click: **Tools → Serial Monitor**
- Set baud rate to **115200** (different from Arduino!)
- You should see:
  ```
  ESP32 OxyFeeder Communicator Starting...
  Serial2 initialized for Arduino communication
  BLE Server Started - Advertising as 'OxyFeeder'
  Waiting for mobile app connection...
  Ready to receive data from Arduino via Serial2
  ```

**✅ Success!** ESP32 is now running and broadcasting Bluetooth!

---

## 🧪 Part 4: Testing After Building Hardware (Phase 2C)

### Test Setup

Once you've built the "Bridge" (Logic Level Shifter connecting Arduino + ESP32):

**What you'll have:**
```
Arduino Mega ←→ Logic Level Shifter ←→ ESP32
     ↓                                    ↓
  (USB to PC)                        (USB to PC)
```

---

### 🔬 **Test 1: Individual Components**

You already did this when uploading! Each device works independently.

---

### 🔬 **Test 2: Communication Test (The Important One)**

**Goal:** Verify Arduino can send data to ESP32 through the Logic Level Shifter

#### Setup:
1. **Build your bridge first** (solder Logic Level Shifter to perfboard, connect wires as per master blueprint)
2. **Connect both devices:**
   - Arduino USB to PC
   - ESP32 USB to PC
3. **Both devices powered on** (green LEDs should be lit)

#### Testing Steps:

**A. Open TWO Arduino IDE windows**
- Window 1: For Arduino monitoring
- Window 2: For ESP32 monitoring

**Window 1 (Arduino):**
- Select: **Arduino Mega** board + its COM port
- Open **Serial Monitor** (9600 baud)
- You should see the initialization messages

**Window 2 (ESP32):**
- Select: **ESP32 Dev Module** board + its COM port
- Open **Serial Monitor** (115200 baud)
- You should see BLE initialization

**B. Watch for the magic moment!**

**In ESP32 Serial Monitor, you should see:**
```
Received from Arduino: {"do": 7.8, "feed": 62, "battery": 85}
Data sent to mobile app via BLE
Received from Arduino: {"do": 7.9, "feed": 62, "battery": 86}
Data sent to mobile app via BLE
```

This appears **every 5 seconds** with **changing values**!

**✅ If you see this:** Communication is working! Arduino → Logic Shifter → ESP32 path is good!

**❌ If you DON'T see this:** Jump to [Troubleshooting](#part-5-troubleshooting-common-issues) section below.

---

### 🔬 **Test 3: Bluetooth Test (Phone App)**

**Goal:** Verify ESP32 can broadcast to your phone

**You need:**
- Your Android phone
- "nRF Connect" app (free on Play Store)

#### Steps:

**1. Install nRF Connect**
- Open Play Store on your phone
- Search: "**nRF Connect**"
- Install "**nRF Connect for Mobile**" by Nordic Semiconductor

**2. Open nRF Connect**
- Grant Bluetooth and Location permissions when asked
- You'll see a "SCAN" screen

**3. Scan for OxyFeeder**
- Tap the "**SCAN**" button (or it starts automatically)
- Look for a device named **"OxyFeeder"**
- Should appear within a few seconds

**4. Connect to OxyFeeder**
- Tap "**CONNECT**" button next to OxyFeeder
- Wait for connection (turns from gray to green)

**5. View the data**
- Tap the **down arrow** to expand services
- You'll see a service with UUID: `0000abcd-0000-1000-8000-00805f9b34fb`
- Tap the **down arrow** on that service
- You'll see a characteristic with UUID: `0000abce-0000-1000-8000-00805f9b34fb`
- Tap the **down arrow with three lines** icon (enable notifications)

**6. See live data!**
- You should see data appearing like:
  ```
  Value (0x): {"do": 7.8, "feed": 62, "battery": 85}
  ```
- It updates **every 5 seconds** with **changing values**!

**✅ Success!** Your complete chain is working:
```
Arduino → Logic Shifter → ESP32 → Bluetooth → Phone
```

---

## 🐛 Part 5: Troubleshooting Common Issues

### Problem 1: "Port not found" or "Permission denied"

**On Windows:**
- Install CH340 or CP2102 USB drivers
  - Google: "CH340 driver Windows download"
  - Install and restart computer
- Unplug/replug USB cable
- Try a different USB port
- Check Device Manager for yellow exclamation marks

**On Linux/WSL:**
- Add yourself to dialout group:
  ```bash
  sudo usermod -a -G dialout $USER
  ```
- Restart your computer
- Or try temporary fix:
  ```bash
  sudo chmod 666 /dev/ttyUSB0
  ```
  (Replace `/dev/ttyUSB0` with your actual port)

---

### Problem 2: ESP32 "Connecting..." forever

**Solution:**
1. Press and HOLD the "**BOOT**" button on ESP32
2. Click "**Upload**" in Arduino IDE
3. Keep holding BOOT until you see "Writing at 0x00001000..." in output window
4. Release BOOT button
5. Upload should complete normally

**Alternative:**
- Try different USB cable (some are charge-only, not data)
- Try different USB port on computer

---

### Problem 3: ESP32 NOT receiving data from Arduino

**Check this in order:**

**1. Are both devices powered?**
- Look for green power LEDs on both boards
- If no LED: Check USB cable, try different port

**2. Is the Logic Level Shifter connected correctly?**

**Wiring should be:**
```
Arduino Side (HV - High Voltage 5V):
- Arduino 5V      → Shifter HV
- Arduino GND     → Shifter GND
- Arduino Pin 18  → Shifter HV1 (TX1 signal)
- Arduino Pin 19  → Shifter HV2 (RX1 signal)

ESP32 Side (LV - Low Voltage 3.3V):
- Shifter LV      → ESP32 3V3 terminal
- Shifter GND     → ESP32 GND terminal (shared with Arduino GND)
- Shifter LV1     → ESP32 Pin 17 terminal (RX2)
- Shifter LV2     → ESP32 Pin 16 terminal (TX2)
```

**Common mistakes:**
- ❌ Swapping TX and RX
- ❌ Forgetting to connect GND (must be shared!)
- ❌ Connecting HV to 3.3V or LV to 5V (kills the shifter!)

**3. Verify voltages with multimeter (if available):**
- Arduino 5V pin should read ~5V
- ESP32 3V3 terminal should read ~3.3V
- If no multimeter, skip this step

**4. Re-upload both firmwares:**
- Sometimes helps to start fresh
- Upload Arduino firmware again
- Upload ESP32 firmware again

**5. Check Serial Monitor settings:**
- Arduino window: **9600 baud**
- ESP32 window: **115200 baud**
- If wrong baud rate, you'll see garbage characters

---

### Problem 4: Can't find "OxyFeeder" in Bluetooth scan

**Check:**

**1. Is ESP32 running?**
- Look at Serial Monitor (115200 baud)
- Should say "**BLE Server Started - Advertising as 'OxyFeeder'**"
- If not, re-upload firmware

**2. Is Bluetooth enabled on phone?**
- Settings → Bluetooth → **ON**

**3. Is Location enabled? (Android requirement)**
- Settings → Location → **ON**
- Android requires this for BLE scanning (security requirement)

**4. Try restarting ESP32:**
- Unplug USB cable
- Wait 5 seconds
- Plug back in
- Check Serial Monitor to see it restart

**5. Too many devices nearby?**
- Try in a different location (fewer Bluetooth devices)
- Restart nRF Connect app

**6. Clear Bluetooth cache on phone (Android):**
- Settings → Apps → Bluetooth → Storage → Clear Cache

---

### Problem 5: Random/Garbage Data from Arduino

**Check:**
- Baud rate mismatch
- Both Arduino and ESP32 firmware use **9600 baud** for Serial communication
- Verify in code:
  - Arduino: `Serial1.begin(9600);`
  - ESP32: `Serial2.begin(9600, ...);`

---

### Problem 6: Data stops after a while

**Possible causes:**
- Loose wire connection (jiggle wires gently to test)
- Bad solder joint (re-solder if needed)
- USB power saving (disable USB selective suspend in Windows)

**Windows fix for USB power saving:**
1. Control Panel → Power Options
2. Click "Change plan settings" on active plan
3. Click "Change advanced power settings"
4. Expand "USB settings" → "USB selective suspend setting"
5. Set to "**Disabled**"
6. Click Apply

---

## 📊 Part 6: Phase 2C Testing Checklist

**Print this and check off as you complete each step:**

### Hardware Assembly
- [ ] Logic Level Shifter soldered to perfboard
- [ ] HV side wires soldered (5V, GND, HV1, HV2)
- [ ] LV side wires soldered (3V3, GND, LV1, LV2)
- [ ] Wires connected to Arduino female headers
- [ ] Wires connected to ESP32 screw terminals
- [ ] Verified no short circuits (visual inspection or continuity test)

### Firmware Upload
- [ ] Arduino IDE installed and working
- [ ] ESP32 board support installed in Arduino IDE
- [ ] Arduino firmware uploaded successfully
- [ ] ESP32 firmware uploaded successfully
- [ ] Arduino Serial Monitor shows initialization (9600 baud)
- [ ] ESP32 Serial Monitor shows BLE advertising (115200 baud)

### Communication Test
- [ ] Both devices connected to PC via USB
- [ ] Both devices showing power LED (green light)
- [ ] Two Serial Monitor windows open
- [ ] Arduino window at 9600 baud
- [ ] ESP32 window at 115200 baud
- [ ] ESP32 shows "Received from Arduino: {JSON}" every 5 seconds
- [ ] JSON values are changing (not static)
- [ ] DO values vary between 5.0 - 8.5 mg/L
- [ ] Feed level occasionally decreases
- [ ] Battery level fluctuates

### Bluetooth Test
- [ ] nRF Connect app installed on phone
- [ ] Bluetooth enabled on phone
- [ ] Location enabled on phone (Android requirement)
- [ ] "OxyFeeder" device visible in nRF Connect scan
- [ ] Successfully connected to OxyFeeder
- [ ] Found service UUID: `0000abcd-...`
- [ ] Found characteristic UUID: `0000abce-...`
- [ ] Enabled notifications on characteristic
- [ ] Seeing live JSON data on phone
- [ ] Data updates every 5 seconds
- [ ] Values match what Arduino is sending

### Final Verification
- [ ] Left system running for 5 minutes - no disconnects
- [ ] Gently moved wires - connection stays stable
- [ ] Disconnected and reconnected BLE - works reliably

**✅ If ALL boxes checked:** Phase 2C COMPLETE! You're ready for Phase 3! 🎉

---

## 🎬 Part 7: What Happens Next (Phase 3)

Once Phase 2C testing is successful, you'll:

### Phase 3A: Implement Real Bluetooth Service

**In Flutter app:**
1. Add package:
   ```bash
   flutter pub add flutter_blue_plus
   ```

2. Create new file: `lib/core/services/real_bluetooth_service.dart`
   - Scan for "OxyFeeder" device
   - Connect to device
   - Subscribe to characteristic notifications
   - Parse JSON and emit to `statusStream`

### Phase 3B: Swap Services

**In `main.dart`:**
```dart
// Comment out MockBluetoothService
// Provider<MockBluetoothService>(
//   create: (_) => MockBluetoothService(),
//   dispose: (_, svc) => svc.dispose(),
// ),

// Add RealBluetoothService
Provider<RealBluetoothService>(
  create: (_) => RealBluetoothService(),
  dispose: (_, svc) => svc.dispose(),
),
```

### Phase 3C: Test End-to-End

1. Run Flutter app on your phone:
   ```bash
   flutter run
   ```

2. Add a "Connect" button in Settings screen

3. Tap Connect → App connects to hardware

4. See REAL data from Arduino on your Dashboard!

**✅ Phase 3 Complete:** App now controls real hardware! 🎊

---

## 💡 Quick Reference Card

**Keep this handy while testing:**

### Serial Monitor Settings

| **Device** | **Baud Rate** | **Typical Port** | **What to See** |
|------------|---------------|------------------|-----------------|
| Arduino Mega | **9600** | COM3 / ttyACM0 | Initialization messages |
| ESP32 | **115200** | COM4 / ttyUSB0 | BLE advertising + JSON received |

### Bluetooth UUIDs

| **Type** | **UUID** |
|----------|----------|
| Service | `0000abcd-0000-1000-8000-00805f9b34fb` |
| Characteristic | `0000abce-0000-1000-8000-00805f9b34fb` |

### Pin Connections (Bridge Wiring)

```
Arduino Pin    →  Shifter HV  →  Shifter LV  →  ESP32 Pin
18 (TX1)       →  HV1         →  LV1         →  17 (RX2)
19 (RX1)       ←  HV2         ←  LV2         ←  16 (TX2)
5V             →  HV
GND            →  GND (shared)               →  GND
                  LV          →              →  3V3

Extra 5V       →                             →  VIN (powers ESP32)
```

### Data Flow

```
Sensors (simulated)
    ↓
Arduino reads every 5 seconds
    ↓
Creates JSON: {"do": X, "feed": Y, "battery": Z}
    ↓
Sends via Serial1 (TX1, Pin 18) @ 9600 baud
    ↓
Logic Level Shifter (5V → 3.3V conversion)
    ↓
ESP32 receives via Serial2 (RX2, Pin 17) @ 9600 baud
    ↓
ESP32 broadcasts via BLE notifications
    ↓
Phone receives (nRF Connect or Flutter app)
```

### Common Linux/WSL Commands

```bash
# List available serial ports
ls /dev/tty*

# Check port details
ls -l /dev/ttyUSB0

# Give temporary permission
sudo chmod 666 /dev/ttyUSB0

# Add yourself to dialout group (permanent fix)
sudo usermod -a -G dialout $USER
# Then restart computer
```

---

## 📚 Additional Resources

### Arduino IDE Help
- Official tutorial: https://www.arduino.cc/en/Guide/HomePage
- ESP32 setup: https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/

### Bluetooth Low Energy
- nRF Connect app guide: https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-mobile
- BLE basics: https://learn.adafruit.com/introduction-to-bluetooth-low-energy/

### Troubleshooting
- Arduino port issues: https://support.arduino.cc/hc/en-us/articles/4412955149586
- ESP32 upload issues: https://randomnerdtutorials.com/solved-failed-to-connect-to-esp32-timed-out-waiting-for-packet-header/

---

## ✅ Success Criteria

**You've successfully completed Phase 2C when:**

1. ✅ Arduino firmware uploads without errors
2. ✅ ESP32 firmware uploads without errors
3. ✅ Arduino Serial Monitor shows initialization
4. ✅ ESP32 Serial Monitor shows "Received from Arduino: {JSON}" every 5 seconds
5. ✅ JSON data shows realistic, changing values
6. ✅ Phone's nRF Connect app finds "OxyFeeder" device
7. ✅ Can connect to OxyFeeder via Bluetooth
8. ✅ Characteristic notifications show live JSON data
9. ✅ System runs stably for 5+ minutes without disconnects

**When all criteria met:** 🎊 **CONGRATULATIONS!** Your Communication Hub is fully functional! You're ready to proceed to Phase 3 - connecting your Flutter app to the real hardware!

---

*Last Updated: 2025-11-22*
*For: OxyFeeder Project - Phase 2C Testing*
