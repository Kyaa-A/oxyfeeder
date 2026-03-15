# OxyFeeder App - User Guide

A complete guide on how to use the OxyFeeder mobile app to monitor your fishpond and automate feeding.

---

## Getting Started

### What You Need
- Android phone with Bluetooth enabled
- OxyFeeder hardware (Arduino + ESP32) powered on and nearby
- The OxyFeeder app installed on your phone

### First Launch
1. Open the OxyFeeder app
2. Allow Bluetooth and Location permissions when prompted
3. The app will automatically scan for your OxyFeeder device
4. Once connected, you'll see **"CONNECTED"** in green at the top right

> If it shows **"SCANNING..."** in yellow, make sure your OxyFeeder hardware is powered on and within Bluetooth range (about 10 meters).

---

## Dashboard

The Dashboard is your main screen. It shows everything at a glance.

### Connection Banner
- **"Main Pond - System Active"** (green) = Your device is connected and sending data
- **"Searching..."** (yellow) = Not connected yet. Tap the refresh button to retry

### Alert Banner
- A **red banner** appears below the connection card when any sensor reading crosses your safety threshold
- Example: "Low Feed Level: 15%" or "Low Dissolved Oxygen: 3.2 mg/L"
- You can configure these thresholds in **Settings > Safety Thresholds**
- To turn off alerts, disable **Notifications** in Settings

### Sensor Cards

There are 3 sensor cards showing live data from your fishpond:

#### Dissolved Oxygen (DO)
- Shows the current oxygen level in your pond water (in mg/L)
- **OPTIMAL** (green) = 6.0 mg/L or above - fish are healthy
- **WARNING** (yellow) = 4.0 to 5.9 mg/L - oxygen is getting low
- **CRITICAL** (red) = below 4.0 mg/L - fish are in danger
- Tap the card to see more details

#### Feed Level
- Shows how much feed is left in the hopper (in %)
- **SUFFICIENT** (green) = above 20% - enough feed
- **REFILL NEEDED** (red) = 20% or below - time to refill
- Tap the card to see more details

#### Battery
- Shows the battery/power level of the system (in %)
- **HEALTHY** (green) = above 20%
- **LOW POWER** (red) = 20% or below
- Tap the card to see more details

#### Tapping a Sensor Card
When you tap any sensor card, a detail panel slides up showing:
- The current reading and status
- **What it measures** - a simple explanation of the sensor
- **How it works** - how the sensor collects data
- **Alert threshold** - the current threshold you've set in Settings, and how to change it

### Feed Now Button
- Tap **"FEED NOW"** to manually dispense feed immediately
- The button will show **"FEEDING..."** while the command is being sent
- A message will confirm if it was sent successfully or failed
- Make sure your device is connected before pressing

---

## Sensors

The Sensors tab shows detailed diagnostics and history.

### Component Status
A list of all hardware components and whether they're working:

| Component | What It Shows |
|-----------|---------------|
| BLE Connection | Connected or No signal |
| DO Sensor | Current reading or -- |
| Load Cell | Feed level % or -- |
| Voltage Sensor | Battery % or -- |
| RTC | Active or -- |
| Servo | Ready |
| Motor | Ready |
| SIM800L (SMS) | Standby |

- **Green "OK"** = component is working
- **Red "N/A"** = no data from this component
- **ONLINE** badge (top right) = device is sending live data
- **OFFLINE** badge = no data being received

### DO History Chart
- Shows a graph of dissolved oxygen readings over time
- The teal line tracks how oxygen levels change
- Useful for spotting trends (e.g., oxygen drops at night)
- Shows the total number of readings collected

---

## Settings

The Settings tab is where you configure everything.

### Feeding Automation

This is where you set up automatic feeding schedules so the machine feeds your fish even when you're not around.

#### How to Add a Feeding Schedule
1. Tap the **+ button** (bottom right corner)
2. A dialog will appear with these options:
   - **Feeding Time** - Set the hour, minute, and AM/PM
   - **Duration** - How long to dispense feed (1, 2, 3, 5, 10, 15, 20, or 30 seconds)
   - **Enable** - Toggle on/off
3. Tap **"ADD"** to save the schedule

#### Managing Schedules
- **Toggle switch** (right side) - Turn a schedule on or off without deleting it
- **Trash icon** - Delete a schedule permanently
- Schedules are saved on your phone and synced to the ESP32 device automatically
- The machine will feed at the scheduled times even if your phone is disconnected (as long as the schedules were synced while connected)

#### Tips for Feeding Schedules
- Start with 2-3 schedules per day (e.g., 6:00 AM, 12:00 PM, 6:00 PM)
- Use shorter durations (3-5 seconds) for small ponds
- Use longer durations (10-20 seconds) for larger ponds or more fish
- You can have multiple schedules - they all run independently

### Safety Thresholds

These sliders control when the app warns you about dangerous conditions. The same values are synced to the ESP32 machine for SMS alerts.

#### Min Dissolved Oxygen
- **What it does**: Sets the minimum safe oxygen level
- **Range**: 2.0 to 8.0 mg/L
- **Default**: 4.0 mg/L
- **When to change**: If you want earlier warnings, set it higher (e.g., 5.0). Most fish need at least 4.0 mg/L to survive
- When the reading drops below this value, you'll see an alert banner on the Dashboard and the machine will send an SMS

#### Low Feed Warning
- **What it does**: Sets when to warn you that feed is running low
- **Range**: 0% to 100%
- **Default**: 20%
- **When to change**: Set higher (e.g., 30-40%) if you want more time to refill before it runs out

#### Low Battery Warning
- **What it does**: Sets when to warn you about low power
- **Range**: 0% to 100%
- **Default**: 25%
- **When to change**: Set higher if you want earlier power warnings, especially during rainy/cloudy days when solar charging is low

> All threshold changes are automatically saved and synced to the ESP32. The machine's buzzer also has its own hardcoded safety thresholds as a backup in case your phone is not connected.

### Alert System (SMS)

Set up SMS alerts so the machine can text you when something goes wrong, even when you're far from the pond.

#### How to Set Up SMS Alerts
1. Type your phone number in the field (format: +639XXXXXXXXX)
2. Tap **"SAVE"** to send the number to the machine
3. Tap **"TEST SMS"** to receive a test message and confirm it works

#### Important Notes
- The SIM800L module in the machine sends the SMS (not your phone)
- Make sure the SIM card in the machine has load/credits
- SMS alerts have a 5-minute cooldown to prevent spam
- The machine sends SMS when any threshold is crossed (DO, feed, or battery)

### Connection

#### Device Status
- Shows whether your phone is connected to the OxyFeeder via Bluetooth
- **CONNECTED** (green) = actively receiving data
- **SEARCHING** (yellow) = looking for the device

#### Disconnect / Retry
- Tap **"Disconnect"** to manually disconnect from the device
- Tap **"Retry Connection"** if the app can't find the device
- The app will automatically reconnect when the device is in range

### More

#### Event History
- View a log of past events: connections, disconnections, alerts, feed commands
- Useful for checking what happened while you were away

#### Live Camera
- View a live video stream from the ESP32-CAM pointed at your fishpond
- **How to set up**:
  1. Make sure the ESP32-CAM is powered on and connected to your WiFi
  2. Open the camera screen in the app
  3. Enter the camera's IP address (shown on the ESP32-CAM's serial monitor after boot)
  4. The live stream will appear
- The camera connects via WiFi, not Bluetooth

#### About
- Shows app version and credits

---

## How the Alert System Works

OxyFeeder has a **3-layer alert system** to keep your fish safe:

| Layer | Where | How | When It Works |
|-------|-------|-----|---------------|
| **App Banner** | Your phone screen | Red banner on Dashboard | When phone is connected |
| **SMS Alert** | Text message to your phone | ESP32 triggers SIM800L | Always (even without phone nearby) |
| **Buzzer** | Beeps on the machine | Arduino buzzer | Always (hardcoded safety backup) |

- **App alerts** use the thresholds you set in Settings
- **SMS alerts** use the same thresholds (synced from app to ESP32)
- **Buzzer** uses hardcoded critical values (DO < 4.0, Feed < 20%, Battery < 25%) as an emergency backup that always works regardless of phone connection

---

## Troubleshooting

### App shows "SCANNING..." and won't connect
- Make sure the OxyFeeder hardware is powered on
- Check that Bluetooth is enabled on your phone
- Make sure you're within 10 meters of the device
- Try tapping the connection badge or going to Settings > Connection > Retry

### Sensor values show 0 or don't update
- Check that the ESP32 and Arduino are both powered and connected to each other
- The data link goes: Arduino -> ESP32 -> Bluetooth -> Your Phone
- If one link is broken, values won't update

### Feed Now doesn't work
- Make sure the app shows "CONNECTED"
- Wait for the "Feed command sent!" confirmation
- If it says "Failed to send", check your Bluetooth connection

### SMS alerts not arriving
- Make sure a SIM card with credits is inserted in the SIM800L module
- Use the "TEST SMS" button to verify
- Check that the phone number is saved correctly (with country code, e.g., +63)
- SMS has a 5-minute cooldown between alerts

### Live Camera not loading
- The camera uses WiFi, not Bluetooth
- Make sure the ESP32-CAM is connected to the same WiFi network
- Check the IP address is correct
- Try restarting the ESP32-CAM module
