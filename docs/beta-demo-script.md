# OxyFeeder — Beta Video Demo Script

**Format:** Step-by-step input-process-output for each function
**Total Length:** ~4-5 minutes
**Style:** Technical walkthrough, concise narration

Each function is one scene. Record separately, edit together.

---

## SCENE 1 — System Startup

**Function:** Power on and system boot

| | |
|---|---|
| **Input** | User flips master switch to ON |
| **Process** | System boots, initializes Arduino Mega, ESP32, sensors, LCD, SIM800L. 15-second startup delay. |
| **Output** | LCD displays sensor readings. System ready. |

### Script
> "The system is powered on by flipping the master switch. After a 15-second boot sequence, the LCD displays live sensor readings, indicating the system is ready."

### Shot
- Close-up of master switch flipping ON
- Time-skip 15 seconds
- LCD lighting up with values

---

## SCENE 2 — Bluetooth Connection

**Function:** Connect mobile app to machine

| | |
|---|---|
| **Input** | User opens OxyFeeder app |
| **Process** | App scans for BLE device named "OxyFeeder", auto-pairs |
| **Output** | Green "CONNECTED" status appears in app |

### Script
> "Opening the app initiates a Bluetooth scan. The app automatically detects the OxyFeeder device and establishes connection within seconds."

### Shot
- Phone screen recording: app opens
- Scanning animation
- CONNECTED status appears

---

## SCENE 3 — Sensor Reading (Dissolved Oxygen)

**Function:** Read dissolved oxygen level

| | |
|---|---|
| **Input** | DO sensor on Arduino pin A1 |
| **Process** | Arduino reads analog voltage, converts to mg/L using calibration |
| **Output** | Value sent via BLE to app, displayed on DO card |

### Script
> "The dissolved oxygen sensor on analog pin A1 reads voltage from the probe in the water. Arduino converts this to milligrams per liter and sends the value to the app for display."

### Shot
- Quick shot of DO probe in water
- App dashboard showing DO card with value
- Color indicator (green/yellow/red)

---

## SCENE 4 — Sensor Reading (Feed Level)

**Function:** Measure feed level in hopper

| | |
|---|---|
| **Input** | HC-SR04 ultrasonic sensor mounted on the side of the hopper |
| **Process** | Sends ultrasonic pulse, measures echo time, calculates distance to feed surface, maps to percentage |
| **Output** | Feed level displayed in app as percentage |

### Script
> "An ultrasonic sensor mounted on the side of the hopper measures distance to the feed surface. This distance is converted to a percentage and displayed in the app."

### Shot
- Side of hopper showing ultrasonic sensor
- App showing Feed Level card

---

## SCENE 5 — Sensor Reading (Battery)

**Function:** Monitor battery level

| | |
|---|---|
| **Input** | Voltage sensor on Arduino pin A2 |
| **Process** | Reads 12V battery voltage, maps to percentage |
| **Output** | Battery percentage displayed in app |

### Script
> "The voltage sensor on analog pin A2 monitors the 12V battery. The reading is converted to a percentage to indicate remaining power."

### Shot
- Battery and voltage sensor
- App Battery card

---

## SCENE 6 — Manual Feeding (Feed Now)

**Function:** Trigger immediate feeding from app

| | |
|---|---|
| **Input** | User taps "FEED NOW" button on app |
| **Process** | App sends command via BLE → ESP32 → GPIO pulse → Arduino Pin 17. Arduino opens servo gate, runs DC motor for set duration, beeps buzzer. |
| **Output** | Feed dispensed. Confirmation beep. Success message in app. |

### Script
> "Tapping FEED NOW sends a command via Bluetooth. The ESP32 pulses a GPIO pin to Arduino, which opens the servo gate, runs the dispensing motor, and sounds a confirmation beep."

### Shot
- Tap FEED NOW on phone
- Cut to machine: servo opening, motor spinning, feed dropping
- Buzzer beep sound

---

## SCENE 7 — Automated Feeding Schedule

**Function:** Set and execute scheduled feeding

| | |
|---|---|
| **Input** | User sets time, duration, enable toggle in app |
| **Process** | Schedule synced via BLE to ESP32 memory. ESP32 compares current RTC time every minute. When match → pulses Feed pin → Arduino dispenses feed. |
| **Output** | Machine feeds automatically at scheduled time, even without phone connected |

### Script
> "In the settings, the user creates a feeding schedule by setting the time, duration, and enabling it. The schedule is stored on the machine. At the scheduled time, the system automatically dispenses feed without needing phone connection."

### Shot
- Setting schedule in app (tap +, fill form, ADD)
- Schedule appears in list
- Optional: show machine feeding at scheduled time

---

## SCENE 8 — Automatic SMS Alert (Low Feed)

**Function:** Auto-send SMS when feed level drops below threshold

| | |
|---|---|
| **Input** | Feed level drops below 10% (detected by ultrasonic sensor) |
| **Process** | Arduino detects low feed condition. SIM800L sends SMS to registered phone number. 1-hour cooldown applied to prevent spam. |
| **Output** | SMS arrives on user's phone with low feed warning |

### Script
> "When the feed level drops below 10 percent, the system automatically sends an SMS alert to the user's registered phone number. A 1-hour cooldown is applied between alerts to prevent message spam."

### Shot
- Show feed level dropping (or empty hopper)
- Time-skip
- SMS notification appearing on phone screen
- Show SMS content: "OxyFeeder ALERT: Low feed level! Please refill the hopper."

---

## SCENE 9 — Live Camera Stream

**Function:** Stream live video from pond

| | |
|---|---|
| **Input** | User opens "Live Camera" in app |
| **Process** | ESP32-CAM connects to saved WiFi network. If no WiFi available, the camera creates its own access point. App auto-discovers the camera and connects to the stream. |
| **Output** | Live video displayed in app |

### Script
> "The ESP32-CAM module connects to the saved WiFi network. If no WiFi is available, the camera automatically creates its own access point so the user can still connect. The app discovers the camera and displays the live feed."

### Shot
- Tap Live Camera
- Stream loads
- Wave hand in front of camera to confirm real-time

---

## SCENE 10 — Diagnostic View (Sensors Tab)

**Function:** Check status of each component

| | |
|---|---|
| **Input** | User opens Sensors tab |
| **Process** | App displays current status of each component based on data stream |
| **Output** | Status grid showing OK/N/A for each component |

### Script
> "The Sensors tab provides a diagnostic overview. Each component shows green OK when working and red N/A when no data is received. This helps users identify issues quickly."

### Shot
- Tap Sensors tab
- Scroll through component status grid

---

## SCENE 11 — Closing

**Function:** Summary

### Script
> "OxyFeeder combines automated feeding, real-time monitoring, automatic SMS alerts, and live camera in one solar-powered system. Designed for small to medium fishponds. Currently in beta testing."

### Shot
- Hero shot of machine
- Fade to project credits

---

## Scene Summary Table

| Scene | Function | Duration |
|-------|----------|----------|
| 1 | System Startup | 15s |
| 2 | Bluetooth Connection | 10s |
| 3 | Read DO Sensor | 15s |
| 4 | Read Feed Level | 15s |
| 5 | Read Battery | 10s |
| 6 | Manual Feeding | 20s |
| 7 | Scheduled Feeding | 25s |
| 8 | Auto SMS Alert | 25s |
| 9 | Live Camera | 20s |
| 10 | Sensors Diagnostic | 15s |
| 11 | Closing | 20s |
| | **TOTAL** | **~3-4 min** |

---

## Pre-Production Checklist

- [ ] Battery fully charged (>12.5V)
- [ ] SIM card has load
- [ ] Phone number set in app
- [ ] At least one schedule set
- [ ] WiFi available (or test the AP fallback)
- [ ] Test every function once before recording
- [ ] Phone fully charged
- [ ] Quiet location for voice-over
- [ ] Good lighting for hardware shots
- [ ] Tripod or steady surface

---

## Recording Tips

- Use horizontal (landscape) orientation
- Stable camera (tripod or steady hands)
- Record voice-over separately in a quiet room
- Use phone's built-in screen recorder for app shots
- Re-record any scene that doesn't look right — they are independent
- Add text overlays showing "INPUT", "PROCESS", "OUTPUT" labels in post-production for academic clarity
