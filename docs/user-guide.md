# OxyFeeder App - User Guide (Alpha Test)

A complete guide for alpha testers on how to operate the OxyFeeder system.

---

## Quick Start (Read This First!)

### What You Need
- Android phone with Bluetooth enabled
- OxyFeeder hardware powered on (master switch ON)
- The OxyFeeder APK installed on your phone
- WiFi network (home router or phone hotspot) for camera feature

### Power On
1. Turn on the **master switch**
2. Wait **15 seconds** for the system to fully boot (startup delay prevents false triggers)
3. Open the OxyFeeder app
4. Allow Bluetooth and Location permissions when prompted
5. App automatically scans for the OxyFeeder device via Bluetooth
6. **"CONNECTED"** (green) appears at top right = ready to use

> If it shows **"SCANNING..."** (yellow), make sure the hardware is powered on and you're within 10 meters.

### System Overview
The OxyFeeder has **two wireless connections**:
- **Bluetooth** — connects app to Arduino/ESP32 for sensor data and commands (automatic)
- **WiFi** — connects app to ESP32-CAM for live video only (requires same WiFi network)

Both work at the same time on your phone — no conflict.

---

## Tab 1 — Dashboard

The main screen showing all live data at a glance.

### Connection Banner
- **"Main Pond - System Active"** (green) = Device connected and sending data
- **"Searching..."** (yellow) = Not connected yet. Tap refresh to retry

### Alert Banner
- A **red banner** appears when any reading crosses your safety threshold
- Example: "Low Feed Level: 15%" or "Low Dissolved Oxygen: 3.2 mg/L"
- Configure thresholds in **Settings > Safety Thresholds**

### Sensor Cards

3 cards showing live data:

#### Dissolved Oxygen (DO)
- Current oxygen level in pond water (mg/L)
- **OPTIMAL** (green) = 6.0+ mg/L — fish are healthy
- **WARNING** (yellow) = 4.0 to 5.9 mg/L — getting low
- **CRITICAL** (red) = below 4.0 mg/L — danger
- *Note: DO sensor is currently uncalibrated — readings may not be accurate yet*

#### Feed Level
- How much feed is left in the hopper (%)
- Measured by **ultrasonic sensor** (HC-SR04) mounted at the top of the hopper
- Detects how far the feed surface is from the sensor
- **SUFFICIENT** (green) = above 20%
- **REFILL NEEDED** (red) = 20% or below

#### Battery
- Battery/power level of the system (%)
- Measured by voltage sensor reading the 12V battery
- **HEALTHY** (green) = above 20%
- **LOW POWER** (red) = 20% or below

#### Tapping a Sensor Card
Tap any card to see a detail panel with:
- Current reading and status
- What it measures
- How it works
- Alert threshold

### Feed Now Button
- Tap **"FEED NOW"** to manually dispense feed
- Shows **"FEEDING..."** while processing
- Confirms success or failure with a message
- Must be connected (green status) to work

**What happens when Feed Now is triggered:**
1. Servo opens to 30 degrees for **0.15 seconds** then closes back to 0 degrees — releases a small amount of feed from the hopper
2. DC motor spins for the set duration to push feed out
3. Buzzer beeps to confirm feeding completed

> The servo is detached (inactive) when not feeding to prevent random jitter. It only activates during Feed Now or scheduled feeds.

---

## Tab 2 — Sensors

Detailed diagnostics and component status.

### Component Status

| Component | What It Shows |
|-----------|---------------|
| BLE Connection | Connected or No signal |
| DO Sensor | Current reading or -- |
| Ultrasonic Sensor | Feed level % or -- |
| Voltage Sensor | Battery % or -- |
| RTC | Active or -- |
| Servo | Ready |
| Motor | Ready |
| SIM800L (SMS) | Standby |

- **Green "OK"** = component is working
- **Red "N/A"** = no data from this component
- **ONLINE** badge = device sending live data
- **OFFLINE** badge = no data received

### DO History Chart
- Graph of dissolved oxygen readings over time
- Useful for spotting trends (oxygen drops at night, etc.)

---

## Tab 3 — Settings

### Feeding Automation

Set up automatic feeding schedules.

#### How to Add a Schedule
1. Tap the **+ button** (bottom right)
2. Set the options:
   - **Feeding Time** — hour, minute, AM/PM
   - **Duration** — 1, 2, 3, 5, 10, 15, 20, or 30 seconds
   - **Enable** — toggle on/off
3. Tap **"ADD"**

#### Managing Schedules
- **Toggle switch** — turn schedule on/off without deleting
- **Trash icon** — delete permanently
- Schedules sync to the ESP32 automatically
- Machine feeds at scheduled times even if phone is disconnected (as long as schedules were synced while connected)

#### Tips
- Start with 2-3 schedules per day (e.g., 6:00 AM, 12:00 PM, 6:00 PM)
- Shorter durations (3-5 sec) for small ponds
- Longer durations (10-20 sec) for larger ponds

---

### Safety Thresholds

Sliders that control when the app warns you about dangerous conditions. Same values are synced to the machine for SMS alerts.

| Threshold | Range | Default |
|-----------|-------|---------|
| Min Dissolved Oxygen | 2.0 - 8.0 mg/L | 4.0 mg/L |
| Low Feed Warning | 0% - 100% | 10% |
| Low Battery Warning | 0% - 100% | 10% |

> The machine's buzzer has its own hardcoded safety thresholds as a backup — it will beep even if your phone is not connected.

---

### Alert System (SMS)

The machine can text you when something goes wrong, even when you're far away.

#### Setup
1. Type phone number (format: `09XXXXXXXXX`)
2. Tap **"SAVE"**
3. Tap **"TEST SMS"** to verify

#### Important
- SMS is sent by the SIM800L module inside the machine (not your phone)
- SIM card must have load/credits
- SIM slot is loose — if SMS stops working, check if SIM card shifted (use paper shim behind SIM to keep it pressed)
- SMS has a cooldown to prevent spam
- Default number: `09550717546` (can be changed in the app)

---

### Connection

- **CONNECTED** (green) = actively receiving data
- **SEARCHING** (yellow) = looking for device
- **Disconnect** — manually disconnect
- **Retry Connection** — force reconnect

---

### Live Camera

View a live video stream from the ESP32-CAM.

#### First Time Setup (One Time Only)
1. Power on the system (master switch ON)
2. The ESP32-CAM creates a hotspot: **OxyFeeder-CAM** (password: `oxyfeeder123`)
3. On your phone, go to WiFi settings and connect to `OxyFeeder-CAM`
4. Turn off mobile data temporarily
5. Open Chrome and go to `192.168.4.1`
6. Click **"Configure WiFi"**
7. Select your WiFi network from the list (e.g., your home router or phone hotspot)
8. Enter the WiFi password and tap Save
9. Camera restarts and connects to the selected WiFi
10. Reconnect your phone back to the same WiFi
11. Open the app > Live Camera — stream loads automatically

> After first setup, the camera remembers the WiFi and connects automatically every time you power on. No need to repeat these steps unless you change WiFi.

#### Changing Camera WiFi Later
If you move to a new location or want to switch WiFi:
1. Open Chrome on your phone
2. Go to `http://oxyfeeder-cam.local/wifi`
3. Enter new WiFi name and password
4. Camera restarts on the new network

If the camera can't connect to any saved WiFi, it automatically creates the `OxyFeeder-CAM` hotspot again — just repeat the first time setup steps.

#### AUTO SCAN
- If the stream doesn't load automatically, tap the **settings icon** (top right) in Live Camera
- Tap **"AUTO SCAN FOR CAMERA"**
- Wait ~10 seconds — it scans all devices on your WiFi network
- When found, the IP auto-fills
- Tap **"SAVE & CONNECT"**

#### Camera Tips
- Phone and camera MUST be on the **same WiFi network**
- Bluetooth (for sensor data) and WiFi (for camera) work at the same time
- The stream may have some lag — this is normal for a budget ESP32-CAM
- If using phone hotspot as WiFi: you need 2 phones (1 for hotspot, 1 for app) or use a router instead

---

### Event History
- Log of past events: connections, disconnections, alerts, feed commands
- Check what happened while you were away

---

## Alert System — 3 Layers of Protection

| Layer | Where | How | When It Works |
|-------|-------|-----|---------------|
| **App Banner** | Phone screen | Red banner on Dashboard | When phone is connected via Bluetooth |
| **SMS Alert** | Text message | SIM800L sends SMS | Always (even without phone nearby) |
| **Buzzer** | Machine beeps | Arduino buzzer | Always (hardcoded backup) |

---

## Hardware Notes for Testers

### Power
- System runs on 12V battery charged by solar panel via MPPT controller
- Master switch turns everything on/off
- Arduino powers the ESP32 BLE bridge via 5V connection

### Feed Mechanism
- **Hopper** holds the fish feed
- **Ultrasonic sensor** (HC-SR04) at the top measures feed level by distance
- **Servo** opens a gate to release feed (30 degrees for 0.15 seconds)
- **DC motor** spins to push feed out through the dispenser
- Feeding is a blocking operation — sensors pause briefly during dispensing

### Known Limitations (Alpha)
- **DO sensor** is uncalibrated — readings may be inaccurate (needs electrolyte solution)
- **Camera lag** — ESP32-CAM is a budget module, slight delay is normal
- **SMS number** — currently hardcoded, changing via app may require Arduino restart
- **USB conflict** — if Arduino USB is connected to laptop, LCD may show white screen and Pin 1 data conflicts. System works fine on main power only.
- **Buzzer** — current buzzer is 12V, waiting for 5V replacement

### Sensor Ranges
| Sensor | What | Range | Pins |
|--------|------|-------|------|
| Ultrasonic HC-SR04 | Feed level | 5cm (full) to 37cm (empty) | TRIG=13, ECHO=48 |
| Voltage Sensor | Battery | 0-25V | A2 |
| DO Sensor | Dissolved Oxygen | 0-20 mg/L | A1 |
| RTC DS3231 | Time/Schedules | -- | SDA=20, SCL=21 |

---

## Troubleshooting

### App shows "SCANNING..." and won't connect
- Make sure hardware is powered on (master switch ON)
- Bluetooth enabled on your phone
- Within 10 meters of the device
- Go to Settings > Connection > Retry

### Sensor values show 0 or don't update
- Check that ESP32 and Arduino are both powered
- Data path: Arduino → ESP32 → Bluetooth → Phone
- If one link is broken, values won't update

### Feed Now doesn't work
- Make sure app shows "CONNECTED" (green)
- Wait for "Feed command sent!" confirmation
- If it says "Failed to send", check Bluetooth connection

### Feed level jumps to 0% then back
- Occasional ultrasonic misreads — firmware uses median of 3 readings to filter this
- If it persists, check sensor is pointed straight down with no obstructions

### SMS alerts not arriving
- SIM card must be inserted with load/credits
- SIM slot is loose — use paper shim behind SIM card to ensure contact
- Use "TEST SMS" button to verify
- Phone number format: `09XXXXXXXXX`

### Live Camera not loading
- Phone and camera must be on the same WiFi
- Try AUTO SCAN
- If that fails, connect to `OxyFeeder-CAM` hotspot and enter `192.168.4.1`
- Restart ESP32-CAM by turning master switch off and on

### Servo moves randomly
- Should not happen with latest firmware (servo detaches when idle)
- If it does, reflash Arduino with the latest firmware
