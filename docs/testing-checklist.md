# OxyFeeder - Hardware Testing Checklist

**Date Created:** January 24, 2026  
**Status:** Ready for Hardware Testing

---

## ✅ Pre-Testing Setup

### 1. Arduino Mega Setup
- [ ] Fix Arduino Mega connection (try different USB cable or install CH340 driver)
- [ ] Upload `oxyfeeder_firmware.ino` to Arduino Mega
- [ ] Note: Use Arduino IDE with these libraries installed:
  - RTClib
  - HX711
  - Servo
  - Adafruit_ILI9341
  - Adafruit_GFX
  - SoftwareSerial

### 2. ESP32 Setup
- [ ] Upload `esp32_communicator.ino` to ESP32
- [ ] Verify ESP32 powers on (LED should blink)

### 3. Physical Connections
- [ ] Connect Arduino Mega Serial1 (TX1=Pin 18, RX1=Pin 19) to ESP32 Serial2 (RX2=Pin 16, TX2=Pin 17)
- [ ] Use logic level shifter (5V <-> 3.3V) between Arduino and ESP32
- [ ] Power ESP32 from Arduino's 5V output
- [ ] Connect all sensors to Arduino:
  - DO Sensor → A1
  - Voltage Sensor → A0
  - HX711 (Load Cell) → D8 (DT), D9 (SCK)
  - DS3231 RTC → I2C (SDA/SCL)
  - SIM800L → Serial3 (TX3=Pin 14, RX3=Pin 15)
  - TFT Display → SPI (CS=D40, DC=D38)
  - Motor Driver → D10, D11, D12
  - Servo → D6
  - Buzzer → D7

---

## 📱 Flutter App Testing

### Dashboard Screen
- [ ] App launches without errors
- [ ] Connection status shows "SCANNING..." when disconnected
- [ ] Connection status shows "CONNECTED" when ESP32 is found
- [ ] Sensor cards display real data from Arduino
- [ ] "FEED NOW" button triggers manual feed
- [ ] Loading spinner appears while command is sending
- [ ] Success/Error feedback appears in SnackBar

### Sensors Screen
- [ ] Live readings update every 2 seconds
- [ ] DO history chart populates with data
- [ ] "LIVE" indicator shows when receiving data

### Settings Screen
- [ ] Feeding schedules persist after app restart
- [ ] Thresholds persist after app restart
- [ ] Phone number input works
- [ ] "SAVE" button saves phone to Arduino (check Arduino Serial Monitor)
- [ ] "TEST SMS" sends test message
- [ ] Connection section shows dynamic status
- [ ] Disconnect/Retry buttons work

### History Screen
- [ ] Events are logged (manual feed, SMS sent)
- [ ] Export CSV works
- [ ] Clear all works

### Camera Screen (if ESP32-CAM connected)
- [ ] Can enter stream URL
- [ ] Connection status updates

### About Screen
- [ ] Shows correct version info
- [ ] Shows device connection status
- [ ] Credits show "Developed by Team"

---

## 🔧 Arduino Firmware Testing

### Serial Monitor Output
- [ ] Shows "SYSTEM READY" on boot
- [ ] Displays sensor readings every 2 seconds
- [ ] Shows "[CMD] Received:" when app sends command

### Commands to Test (via App or Serial)
| Command | Expected Result |
|---------|-----------------|
| `PHONE:+639XXXXXXXXX` | Saves phone, returns `{"cmd":"PHONE","status":"OK"}` |
| `FEED:5` | Runs feeding sequence, returns `{"cmd":"FEED","status":"OK"}` |
| `TEST_SMS:1` | Sends test SMS, returns `{"cmd":"TEST_SMS","status":"OK"}` |
| `GET_PHONE:1` | Returns `{"cmd":"GET_PHONE","phone":"..."}` |
| `CLEAR_SCHEDULES:` | Clears schedules, returns OK |
| `SCHEDULE:08:00 AM,10,1` | Adds schedule, returns OK |

### Feeding Schedule Test
- [ ] Add schedule from app
- [ ] Verify Arduino receives and stores it
- [ ] Wait for scheduled time → feeding should trigger
- [ ] Clear all schedules → no auto feeding

### Safety Alert Test
- [ ] Simulate low DO (< 4.0 mg/L) → SMS alert should send
- [ ] Verify SMS is received on phone
- [ ] Check 5-minute cooldown works (no spam)

---

## 📡 BLE Communication

### Identifiers
- **Device Name:** OxyFeeder
- **Service UUID:** `0000abcd-0000-1000-8000-00805f9b34fb`
- **Data Characteristic:** `0000abce-...` (NOTIFY)
- **Command Characteristic:** `0000abcf-...` (WRITE)

### Data Format (Arduino → App)
```json
{"do": 7.5, "feed": 85, "battery": 92}
```

### Command Format (App → Arduino)
```
CMD:VALUE
```

---

## 🐛 Known Issues / Workarounds

1. **Arduino Mega not detected:** Try installing CH340 driver or use different USB cable
2. **ESP32 not advertising:** Reset ESP32, check Serial Monitor for errors
3. **App crashes on connect:** Ensure Bluetooth/Location permissions are granted
4. **SMS not sending:** Verify SIM800L has SIM card with credit, check signal

---

## 📋 Final Checklist Before Demo

- [ ] Arduino firmware uploaded and running
- [ ] ESP32 firmware uploaded and BLE advertising
- [ ] All sensors connected and reading correctly
- [ ] GSM module working (test SMS received)
- [ ] App installed on Android phone
- [ ] App connects to device successfully
- [ ] All screens navigate without errors
- [ ] Commands from app work (Feed, SMS, Settings)

---

**Once Arduino is fixed, follow this checklist step by step!**

Good luck! 🚀
