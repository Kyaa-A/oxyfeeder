# OxyFeeder — Beta Video Demo Script

**Total Video Length:** ~6-8 minutes (final edited)
**Format:** Scene-by-scene shoot list with narration script

Each scene can be recorded separately and edited together. Re-record any scene that does not look good. Final video edited in sequence.

---

## Pre-Production Checklist

Before you start recording:

- [ ] Charge phone and battery fully
- [ ] SIM card has load
- [ ] Phone number set in app
- [ ] At least one feeding schedule already set
- [ ] WiFi available for camera scene
- [ ] Test every feature once to confirm it works
- [ ] Clean lens of phone camera (for filming)
- [ ] Find quiet location for audio recording
- [ ] Good lighting where machine will be filmed
- [ ] Tripod or stable surface for phone (or have someone hold steady)
- [ ] Backup phone or camera in case primary fails

---

## SCENE 1 — Hook / Problem Intro

**Duration:** 15-20 seconds
**Shot:** Wide shot of a fishpond OR close-up shots of fish, then transition to the machine
**Audio:** Voice-over narration (or live to camera)

### Script

> "In aquaculture, two problems cause major losses: inconsistent feeding wastes money and stunts fish growth, while sudden drops in water quality can kill an entire pond overnight. Fishpond owners cannot watch their ponds 24 hours a day. That is the problem we set out to solve."

### Visual Tips
- Use stock footage of fishponds if you do not have your own
- Quick cuts to build energy
- End on a shot of your machine

---

## SCENE 2 — Solution Introduction

**Duration:** 15 seconds
**Shot:** Hero shot of the machine on a clean background, then close-ups of components
**Audio:** Confident voice-over

### Script

> "Meet OxyFeeder. An automated fish feeding and water quality monitoring system designed for small to medium fishponds. Powered by solar energy, controlled from your phone, and protected by automatic SMS alerts."

### Visual Tips
- Slow camera pan around the machine
- Show the solar panel briefly
- Cut to phone screen showing app logo

---

## SCENE 3 — Hardware Overview (Outside)

**Duration:** 20 seconds
**Shot:** Close-up of the enclosure lid, showing LCD, master switch, and buzzer
**Audio:** Voice-over

### Script

> "On the outside of the enclosure, you have everything you need at a glance: an LCD displays live sensor readings, the master switch controls system power, and a buzzer sounds when any reading reaches critical levels."

### Visual Tips
- Camera moves slowly across each component
- Zoom in on the LCD showing readings
- Show finger flipping the master switch on
- Wait for system to boot, show LCD lighting up

---

## SCENE 4 — Hardware Overview (Inside)

**Duration:** 25 seconds
**Shot:** Open the enclosure, camera looks inside
**Audio:** Voice-over

### Script

> "Inside, the system is organized into two sections. The lower section handles power: an MPPT solar charge controller manages charging, a 12V battery stores energy, a fuse box distributes power safely, and a buck converter steps voltage down to 5V for the electronics. The upper section is the brain: an Arduino Mega reads sensors and controls motors, an ESP32 handles Bluetooth communication with the app, a SIM800L module sends SMS alerts, and an RTC module keeps time for scheduled feedings."

### Visual Tips
- Open the lid slowly to reveal the inside
- Point at each component as it is mentioned (use finger or pointer)
- Steady camera, not shaky
- Good lighting inside the box

---

## SCENE 5 — Power On

**Duration:** 10 seconds
**Shot:** Close-up of the master switch, then pan to the LCD
**Audio:** Voice-over

### Script

> "Turning on the master switch initiates a 15-second boot sequence. Once ready, the LCD displays live readings for dissolved oxygen, feed level, and battery percentage."

### Visual Tips
- Capture the click of the switch
- Wait for the LCD to light up
- Use a quick time-skip (fast-forward edit) for the 15-second wait
- End on LCD showing actual values

---

## SCENE 6 — App Connection

**Duration:** 15 seconds
**Shot:** Phone screen recording, then over-the-shoulder shot
**Audio:** Voice-over

### Script

> "Opening the OxyFeeder app, the phone automatically scans for the device via Bluetooth. Within seconds, the connection is established. The green CONNECTED indicator confirms we are receiving live data."

### Visual Tips
- Use phone screen recording feature for clean footage
- Show the app opening, scanning animation, then CONNECTED status
- Zoom in on the green CONNECTED label

---

## SCENE 7 — Dashboard Walkthrough

**Duration:** 30 seconds
**Shot:** Phone screen recording of the dashboard
**Audio:** Voice-over

### Script

> "The Dashboard is the main screen, showing three sensor cards. The Dissolved Oxygen card displays the current oxygen level in the pond, with color indicators for optimal, warning, and critical conditions. The Feed Level card shows how much feed remains in the hopper, measured by an ultrasonic sensor. The Battery card shows the power level of the 12V system. Tapping any card reveals more details about what it measures, how it works, and the configured alert threshold."

### Visual Tips
- Slow scroll showing all three cards
- Tap each card briefly to show the detail panel
- Highlight the alert banner if visible

---

## SCENE 8 — Feed Now Demonstration

**Duration:** 20 seconds
**Shot:** Split screen — phone tapping FEED NOW on one side, the machine actuating on the other
**Audio:** Voice-over and machine sounds

### Script

> "The FEED NOW button allows manual feeding anytime. When tapped, the servo opens the hopper gate, releasing feed. Then the DC motor spins, pushing the feed out the dispenser. A confirmation beep signals the action is complete."

### Visual Tips
- Pre-position camera to capture the feed dispenser
- Tap FEED NOW clearly visible on phone
- Show the servo moving, the motor spinning, and the feed dropping
- Capture the buzzer beep sound
- This shot may need multiple takes for good audio

---

## SCENE 9 — Sensors Tab

**Duration:** 15 seconds
**Shot:** Phone screen recording
**Audio:** Voice-over

### Script

> "The Sensors tab provides detailed diagnostics. Each component shows its current status. Green OK means the component is working, red N/A means no data is being received. Below the status grid, a chart shows dissolved oxygen trends over time, useful for spotting patterns like nightly oxygen drops."

### Visual Tips
- Scroll slowly through the status grid
- Tap to expand the DO history chart
- Show the data points

---

## SCENE 10 — Feeding Schedule Setup

**Duration:** 25 seconds
**Shot:** Phone screen recording, showing the Settings tab
**Audio:** Voice-over

### Script

> "In the Settings tab, we can configure automated feeding schedules. Tapping the plus button opens a dialog. Here we set the feeding time, the duration in seconds, and toggle the schedule on. After tapping ADD, the schedule is synced to the machine instantly. From now on, the machine will feed at that scheduled time every day, even if my phone is disconnected, because the schedule is stored in the machine's memory."

### Visual Tips
- Slow, deliberate taps so viewer can follow
- Show the schedule appearing in the list after adding
- Show the toggle switch and trash icon for managing schedules

---

## SCENE 11 — Safety Thresholds

**Duration:** 20 seconds
**Shot:** Phone screen recording
**Audio:** Voice-over

### Script

> "Below the schedules are Safety Thresholds. These sliders let us define when the system triggers alerts. For example, lowering the Low Feed Warning slider to 20% means the app will show a red alert banner, and the machine will send an SMS, whenever feed drops below 20%. The same logic applies for low battery and low dissolved oxygen."

### Visual Tips
- Show fingers dragging the sliders
- Highlight the threshold values changing
- Quick cut showing a red alert banner on the dashboard

---

## SCENE 12 — SMS Alert Demo

**Duration:** 30 seconds
**Shot:** Phone screen showing the TEST SMS button, then cut to the SMS notification arriving
**Audio:** Voice-over

### Script

> "One of the most important features is the SMS alert system. The SIM800L module inside the machine can send text messages directly, without needing the user's phone to be nearby. To demonstrate, I will tap the TEST SMS button. The message is sent from the machine to my registered phone number."

**[Pause for SMS to arrive — usually 10-30 seconds. Edit out the wait in the video.]**

> "Here is the SMS arriving on my phone. This means even when I am far away from the pond, I will be alerted when feed runs low, the battery is dying, or oxygen levels become dangerous. The SMS is independent of Bluetooth and works wherever there is cellular signal."

### Visual Tips
- Show finger tapping TEST SMS
- Time-skip the wait
- Cut to the SMS notification popping up on the phone screen
- Zoom in on the SMS content

---

## SCENE 13 — Live Camera Demo

**Duration:** 25 seconds
**Shot:** Phone screen showing the Live Camera screen, then the live feed
**Audio:** Voice-over

### Script

> "The system also includes a live camera feature. An ESP32-CAM module mounted near the pond streams video over WiFi. The app automatically discovers the camera on the local network. Within a few seconds, the live feed appears, giving us real-time visual monitoring of the pond. This is useful for verifying fish behavior, water clarity, or detecting unusual activity."

### Visual Tips
- Show the camera mounted in position (if possible)
- Screen recording of the live stream loading
- Wave a hand in front of the camera to show real-time response
- Quick cut between camera view and pond shot to show it is live

---

## SCENE 14 — Three-Layer Alert System

**Duration:** 15 seconds
**Shot:** Quick cuts showing all three alert types
**Audio:** Voice-over

### Script

> "Alerts work on three layers for maximum reliability. First, when connected, the app shows a red banner on the dashboard. Second, the SIM800L sends an SMS to the user's phone, working anywhere with signal. Third, the buzzer inside the machine sounds an alarm, audible at the pond itself. Even if one layer fails, the others still notify the user."

### Visual Tips
- Quick cuts: app banner, SMS notification, buzzer sounding
- Title cards or overlays labeling each layer

---

## SCENE 15 — Closing Summary

**Duration:** 20-30 seconds
**Shot:** Wide hero shot of the machine working in its environment, fading to logo or team name
**Audio:** Voice-over

### Script

> "OxyFeeder combines automated feeding, real-time monitoring, three-layer alerts, and remote viewing into one solar-powered system. It is designed for fishpond owners who want to protect their livestock and reduce manual labor. After completing alpha testing, we are now refining the system through beta testing for real-world deployment.
>
> Thank you for watching."

### Visual Tips
- End with a strong, clean shot
- Optional: add team names, school logo, or project info as text overlay
- Hold the final frame for a beat before cutting to black

---

## Recommended Editing Flow

1. **Record all scenes** out of order if needed (record outdoor scenes when weather is good, indoor scenes anytime)
2. **Sort clips** by scene number
3. **Cut and trim** each clip to its intended duration
4. **Add voice-over** if narrating after filming (record audio separately for cleaner sound)
5. **Add transitions** between scenes (simple cuts work well, avoid overusing fancy effects)
6. **Add background music** at low volume (royalty-free music recommended)
7. **Add text overlays** for component names, feature highlights, key numbers
8. **Add intro and outro** with team and project credits
9. **Color grade** if needed for consistent look
10. **Export** in 1080p or higher

---

## Total Scene Breakdown

| Scene | Title | Duration |
|-------|-------|----------|
| 1 | Hook / Problem | 15-20s |
| 2 | Solution Intro | 15s |
| 3 | Hardware Outside | 20s |
| 4 | Hardware Inside | 25s |
| 5 | Power On | 10s |
| 6 | App Connection | 15s |
| 7 | Dashboard | 30s |
| 8 | Feed Now Demo | 20s |
| 9 | Sensors Tab | 15s |
| 10 | Feeding Schedule | 25s |
| 11 | Safety Thresholds | 20s |
| 12 | SMS Alert Demo | 30s |
| 13 | Live Camera | 25s |
| 14 | 3-Layer Alerts | 15s |
| 15 | Closing | 20-30s |
| | **TOTAL** | **~6-7 min** |

---

## Recording Tips

### Audio
- Record voice-over in a quiet room
- Speak slowly and clearly
- Read the script naturally, not robotically
- Re-record any sentence that sounds wrong
- Keep audio levels consistent across scenes

### Video
- Use horizontal (landscape) orientation
- Stabilize the camera (tripod or steady hands)
- Avoid backlit shots (window behind the subject)
- Use natural daylight when possible
- Phone screen recording: use the built-in screen recorder for highest quality
- For machine close-ups: use macro mode if your phone has it

### Filming Order Recommendation
1. Outdoor scenes first (depend on weather and light)
2. Hardware scenes when machine is fully assembled and clean
3. App screen recordings — these are the easiest, save for last
4. Voice-over recording in a quiet evening

### Common Mistakes to Avoid
- Do not film in portrait orientation
- Do not zoom in and out rapidly
- Do not let the camera shake
- Do not narrate over machine sounds you want to capture
- Do not record where ambient noise is loud (fans, traffic, people talking)
- Do not rush — let scenes breathe for a moment before cutting

---

Good luck with the beta video. Each scene is independent, so you can re-record any part that does not turn out well without redoing everything.
