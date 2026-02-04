# OxyFeeder Hardware Testing Guide

Complete step-by-step guide for testing OxyFeeder with real hardware connected to your laptop.

---

## Prerequisites

### Hardware Needed
- [ ] Arduino Mega 2560
- [ ] ESP32 Dev Module (with screw terminal extender)
- [ ] Logic Level Shifter (connects Arduino ↔ ESP32)
- [ ] 2x USB cables (one for Arduino, one for ESP32)
- [ ] Android phone with OxyFeeder app installed
- [ ] Laptop with Arduino IDE installed

### Software Needed
- [ ] Arduino IDE (with ESP32 board support installed)
- [ ] OxyFeeder app on phone (from `OxyFeeder.apk`)

---

## Phase 1: Hardware Connection

### Step 1.1: Connect Logic Level Shifter

The Logic Level Shifter bridges 5V Arduino ↔ 3.3V ESP32:

```
Arduino Mega          Logic Level Shifter         ESP32 (Screw Terminal)
-----------          -------------------         ----------------------
5V        ────────►  HV                LV  ◄──── 3V3
GND       ────────►  GND              GND  ◄──── GND
TX1 (18)  ────────►  HV1              LV1  ◄──── RX2 (GPIO 16)
RX1 (19)  ────────►  HV2              LV2  ◄──── TX2 (GPIO 17)
```

### Step 1.2: Connect USB Cables

```
Laptop USB Port 1  ────►  Arduino Mega (USB-B port)
Laptop USB Port 2  ────►  ESP32 (micro-USB port)
```

### Step 1.3: Verify Connections in Device Manager

1. Press `Win + X` → Select **Device Manager**
2. Expand **Ports (COM & LPT)**
3. You should see two ports:
   ```
   USB-SERIAL CH340 (COM3)      ← Arduino Mega
   Silicon Labs CP210x (COM5)   ← ESP32
   ```
4. Note down your COM port numbers (yours may differ)

---

## Phase 2: Flash Firmware

### Step 2.1: Flash Arduino Mega

1. Open **Arduino IDE**
2. **File** → **Open** → Navigate to:
   ```
   c:\dev\oxyfeeder_app\firmware\oxyfeeder_firmware\oxyfeeder_firmware.ino
   ```
3. **Tools** → **Board** → **Arduino AVR Boards** → **Arduino Mega or Mega 2560**
4. **Tools** → **Port** → Select your Arduino COM port (e.g., COM3)
5. Click **Upload** button (→ arrow)
6. Wait for "Done uploading" message

### Step 2.2: Flash ESP32

1. Open **another Arduino IDE window** (File → New Window)
2. **File** → **Open** → Navigate to:
   ```
   c:\dev\oxyfeeder_app\firmware\esp32_communicator\esp32_communicator.ino
   ```
3. **Tools** → **Board** → **ESP32 Arduino** → **ESP32 Dev Module**
4. **Tools** → **Port** → Select your ESP32 COM port (e.g., COM5)
5. Click **Upload** button
6. Wait for "Done uploading" message

---

## Phase 3: Open Serial Monitors

### Step 3.1: Arduino Serial Monitor

1. In the Arduino IDE window (Arduino Mega)
2. **Tools** → **Serial Monitor**
3. Set baud rate to **9600** (bottom-right dropdown)
4. You should see:
   ```
   ==============================================
     OxyFeeder Firmware v2.0 - PRODUCTION BUILD
   ==============================================

   [INIT] Loading phone number from EEPROM...
   [INIT] Initializing actuators...
   ...
   ==============================================
           SYSTEM READY - Entering Main Loop
   ==============================================

   DO: 7.5 mg/L | Feed: 80% | Battery: 90%
   DO: 7.4 mg/L | Feed: 80% | Battery: 90%
   ```

### Step 3.2: ESP32 Serial Monitor

1. In the ESP32 Arduino IDE window
2. **Tools** → **Serial Monitor**
3. Set baud rate to **115200**
4. You should see:
   ```
   ESP32 OxyFeeder Communicator v2.0 Starting...
   Serial2 initialized for Arduino communication
   BLE Server Started - Advertising as 'OxyFeeder'
   Waiting for mobile app connection...

   Received from Arduino: {"do": 7.5, "feed": 80, "battery": 90}
   No BLE client connected - data not sent
   ```

---

## Phase 4: Connect Phone App

### Step 4.1: Install App (if not already)

1. Transfer `OxyFeeder.apk` to your phone
2. Open the APK file and install
3. Grant permissions when prompted:
   - Bluetooth
   - Location (required for BLE scanning)

### Step 4.2: Connect to OxyFeeder

1. Turn on Bluetooth on your phone
2. Open **OxyFeeder** app
3. App should auto-connect to "OxyFeeder" device
4. Dashboard should show live sensor values

### Step 4.3: Verify in ESP32 Serial Monitor

You should see:
```
BLE Client Connected
Received from Arduino: {"do": 7.5, "feed": 80, "battery": 90}
Data sent to mobile app via BLE
```

---

## Phase 5: Test Features

### Test 5.1: Live Sensor Updates

**What to check:**
- [ ] Dashboard shows DO, Feed Level, Battery values
- [ ] Values update every ~2 seconds
- [ ] Charts in Sensors screen show data points

**In Arduino Serial Monitor:**
```
DO: 7.5 mg/L | Feed: 80% | Battery: 90%
DO: 7.4 mg/L | Feed: 80% | Battery: 90%
```

### Test 5.2: Manual Feed (CRITICAL TEST)

This tests the non-blocking feeding fix!

**Steps:**
1. In app, go to **Settings** screen
2. Tap **Feed Now** button
3. Select **10 seconds** duration
4. Tap **Confirm**

**In Arduino Serial Monitor (watch carefully):**
```
[CMD] Received: FEED:10
[CMD] Manual feed for 10 seconds
[ACTUATOR] Starting feeding sequence for 10 seconds...
[FEEDING] Gate opening...
DO: 7.5 mg/L | Feed: 80% | Battery: 90%    ← Still updating! (non-blocking works)
[FEEDING] Dispensing feed...
DO: 7.4 mg/L | Feed: 80% | Battery: 90%    ← Still updating!
DO: 7.3 mg/L | Feed: 80% | Battery: 90%    ← Still updating!
DO: 7.4 mg/L | Feed: 80% | Battery: 90%    ← Still updating!
DO: 7.5 mg/L | Feed: 80% | Battery: 90%    ← Still updating!
[FEEDING] Feed dispensed, closing gate...
[FEEDING] Closing gate...
[FEEDING] Feeding sequence complete!
```

**What to verify:**
- [ ] Motor runs for ~10 seconds (not 5!)
- [ ] Sensor readings continue during feeding
- [ ] Gate opens, motor runs, gate closes, buzzer beeps
- [ ] App still shows live updates during feeding

### Test 5.3: SMS Phone Number

**Steps:**
1. In app **Settings**, enter your phone number
2. Tap **Save Phone Number**
3. Tap **Send Test SMS**

**In Arduino Serial Monitor:**
```
[CMD] Received: PHONE:+639XXXXXXXXX
[CMD] Phone number updated to: +639XXXXXXXXX
[EEPROM] Phone number saved!

[CMD] Received: TEST_SMS:1
[CMD] Sending test SMS...
[GSM] Sending SMS to: +639XXXXXXXXX
[GSM] Message: OxyFeeder Test: SMS system is working!
[GSM] SMS sent!
```

**What to verify:**
- [ ] Phone number saved confirmation appears
- [ ] Test SMS received on your phone (if SIM card installed)

### Test 5.4: Feeding Schedule

**Steps:**
1. In app **Settings**, add a feeding schedule
2. Set time to 2 minutes from now
3. Enable the schedule
4. Wait for scheduled time

**In Arduino Serial Monitor (at scheduled time):**
```
========================================
    SCHEDULED FEEDING TIME!
========================================
[FEEDING] Gate opening...
[FEEDING] Dispensing feed...
...
[FEEDING] Feeding sequence complete!
```

### Test 5.5: Low DO Alert (Simulated)

To test the safety alert system, temporarily modify Arduino code:

1. In `oxyfeeder_firmware.ino`, find `readDissolvedOxygen()` function
2. Temporarily change return value to `3.0` (below 4.0 threshold)
3. Re-upload to Arduino
4. Watch for alert

**In Arduino Serial Monitor:**
```
[ALERT] CRITICAL: Low Dissolved Oxygen!
[GSM] Sending SMS to: +639XXXXXXXXX
[GSM] Message: CRITICAL ALERT: OxyFeeder - Low Dissolved Oxygen...
```

**Remember to revert the change after testing!**

---

## Phase 6: Verify All Systems

### Final Checklist

| Feature | Test | Status |
|---------|------|--------|
| BLE Connection | App connects to "OxyFeeder" | [ ] Pass |
| Live Sensor Data | Dashboard shows updating values | [ ] Pass |
| Manual Feed | Motor runs for specified duration | [ ] Pass |
| Non-blocking Feed | Sensors update during feeding | [ ] Pass |
| Feed Duration | 10s command = 10s motor run | [ ] Pass |
| SMS Phone Number | Number saves and persists | [ ] Pass |
| Test SMS | SMS received on phone | [ ] Pass |
| Feeding Schedule | Auto-feed at scheduled time | [ ] Pass |
| Low DO Alert | SMS sent when DO < 4.0 | [ ] Pass |
| Gate Servo | Opens and closes correctly | [ ] Pass |
| Buzzer | Beeps after feeding complete | [ ] Pass |

---

## Troubleshooting

### Arduino not detected
- Try different USB cable
- Install CH340 driver: https://sparks.gogo.co.nz/ch340.html

### ESP32 not detected
- Try different USB cable
- Install CP210x driver: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers

### No data from Arduino to ESP32
- Check Logic Level Shifter connections
- Verify TX1→RX2 and RX1→TX2 (crossed)
- Check both boards share common GND

### App not connecting via BLE
- Restart app
- Turn Bluetooth off/on
- Check ESP32 Serial Monitor for "Advertising as 'OxyFeeder'"

### Motor not running
- Check L298N connections (IN1=10, IN2=11, ENA=12)
- Verify 12V power to motor driver
- Check motor wires

### Servo not moving
- Check servo connected to Pin 6
- Verify 5V power to servo

### SMS not sending
- Check SIM card installed in SIM800L
- Verify SIM has credit/data
- Check antenna connected
- Try AT commands manually via Serial3

---

## Quick Reference

### COM Port Settings
| Device | Baud Rate |
|--------|-----------|
| Arduino Mega | 9600 |
| ESP32 | 115200 |

### Pin Assignments
| Component | Arduino Pin |
|-----------|------------|
| Motor IN1 | 10 |
| Motor IN2 | 11 |
| Motor ENA | 12 |
| Servo | 6 |
| Buzzer | 7 |
| DO Sensor | A1 |
| Voltage Sensor | A0 |
| Load Cell DT | 8 |
| Load Cell SCK | 9 |
| ESP32 TX | 18 (TX1) |
| ESP32 RX | 19 (RX1) |
| GSM TX | 14 (TX3) |
| GSM RX | 15 (RX3) |

### BLE UUIDs
| Purpose | UUID |
|---------|------|
| Service | 0000abcd-0000-1000-8000-00805f9b34fb |
| Data Characteristic | 0000abce-0000-1000-8000-00805f9b34fb |
| Command Characteristic | 0000abcf-0000-1000-8000-00805f9b34fb |

---

## Next Steps

After all tests pass:
1. Disconnect from laptop
2. Connect to solar power system
3. Deploy at pond location
4. Calibrate sensors with real water
5. Fine-tune feeding schedules

**Congratulations! Your OxyFeeder is ready for deployment!**
