# Phase 4: Final Hardware Assembly Guide

## The Simple Approach

**Don't think about building a whole robot.** Focus on ONE mission at a time.

| Mission | Goal | Time |
|---------|------|------|
| **Mission 1** | Mechanical Setup (Drilling & Mounting) | ~1-2 hours |
| **Mission 2** | Power Backbone (The "Veins") | ~30 min |
| **Mission 3** | Sensors (The "Nerves") | ~30 min |
| **Mission 4** | Actuators & Camera (The "Muscles" & "Eyes") | ~30 min |

---

## Materials Checklist

Before starting, gather everything:

### Enclosure & Mounting
- [ ] Waterproof enclosure (IP65 or better)
- [ ] Cable glands (various sizes)
- [ ] Standoffs, screws, double-sided tape/velcro
- [ ] Drill with appropriate bits
- [ ] Cable zip ties

### Power System
- [ ] Solar Panel
- [ ] MPPT Charge Controller
- [ ] 12V Lead Acid Battery
- [ ] Master Switch
- [ ] 12V Fuse Box
- [ ] Fuses: 5A (x3), 10A (x1)
- [ ] DC-DC Buck Converter (12V to 5V)
- [ ] Wires: 14-16 AWG (power), 22 AWG (signals)

### Wiring Accessories
- [ ] Wire Ferrules Kit
- [ ] Ring Terminals
- [ ] JST Connector Set
- [ ] Heat Shrink Tubing

### Electronics
- [ ] Arduino Mega 2560
- [ ] ESP32 DevKit (with Screw Terminal Extender)
- [ ] ESP32-CAM module
- [ ] Logic Level Shifter (on perfboard)
- [ ] L298N Motor Driver
- [ ] 12V DC Gear Motor
- [ ] Servo Motor

### Sensors
- [ ] DO Sensor (DFRobot Gravity Analog)
- [ ] HX711 + Load Cell
- [ ] DS3231 RTC Module
- [ ] Voltage Sensor Module
- [ ] TFT LCD Display

### Protection Components
- [ ] 1000uF Capacitor
- [ ] 0.1uF Ceramic Capacitors
- [ ] 10k Resistors (spare)
- [ ] Flyback Diodes

---

# MISSION 1: Mechanical Setup

**Goal:** Get everything attached to the box. NO WIRING YET.

## Step 1: Empty the Box
Open your waterproof enclosure. It should be empty.

## Step 2: Plan the Floor

Place these components inside and move them around until they fit nicely:
- Battery (keep at bottom - it's heavy)
- MPPT Controller
- Fuse Box
- Buck Converter
- Arduino + ESP32 perfboard
- L298N Motor Driver

```
┌─────────────────────────────────────────┐
│           ENCLOSURE LAYOUT              │
│                                         │
│  ┌──────────┐  ┌──────────┐            │
│  │ Arduino  │  │  ESP32   │            │
│  │  Mega    │  │  BLE     │            │
│  └──────────┘  └──────────┘            │
│                                         │
│  ┌──────────┐  ┌────────────────┐      │
│  │  L298N   │  │   Fuse Box     │      │
│  │  Motor   │  │                │      │
│  └──────────┘  └────────────────┘      │
│                                         │
│  ┌──────────┐  ┌────────────────┐      │
│  │  Buck    │  │     MPPT       │      │
│  │Converter │  │   Controller   │      │
│  └──────────┘  └────────────────┘      │
│                                         │
│  ┌──────────────────────────────┐      │
│  │         BATTERY (bottom)     │      │
│  └──────────────────────────────┘      │
│                                         │
│  [Master Switch]   [LCD Window]         │
└─────────────────────────────────────────┘
```

## Step 3: Drill Holes

Mark where wires need to come in:
- Solar panel cables
- Motor wires
- DO sensor probe
- Load cell wires

Drill those holes. Install **cable glands** to seal them.

## Step 4: Mount Components

Mount components securely:
- **Battery**: Must be secured (screws or straps)
- **Arduino/ESP32**: Standoffs (10mm) to elevate
- **Lighter modules**: Double-sided tape or velcro OK
- **Fuse Box**: Screws, easy access for fuse replacement

---

# STOP. MISSION 1 COMPLETE. TAKE A BREAK.

---

# MISSION 2: Power Backbone

**Goal:** Get power to Fuse Box, Arduino, Motor Driver, and Buck Converter. NO SENSORS YET.

## Power Flow Diagram

```
SOLAR PANEL
     │
     ▼
┌─────────────┐
│    MPPT     │
│ Controller  │
└──┬──────┬───┘
   │      │
   ▼      ▼
BATTERY   LOAD OUTPUT
           │
           ▼
      ┌─────────┐
      │  FUSE   │ (15A main)
      └────┬────┘
           │
           ▼
      ┌─────────┐
      │ MASTER  │
      │ SWITCH  │
      └────┬────┘
           │
           ▼
┌──────────────────────────────────┐
│         12V FUSE BOX             │
│  ┌────┐ ┌────┐ ┌────┐ ┌────┐   │
│  │ 5A │ │10A │ │ 5A │ │ 5A │   │
│  └──┬─┘ └──┬─┘ └──┬─┘ └──┬─┘   │
└─────┼──────┼──────┼──────┼─────┘
      │      │      │      │
      ▼      ▼      ▼      ▼
   Arduino  Motor  Buck   Spare
    Vin    Driver  Conv.
                    │
                    ▼
              5V Rail ──→ Servo + ESP32-CAM
```

## Wiring Tips: Use Your Accessories!

- **Ferrules**: Crimp onto every wire going into MPPT, Fuse Box, or screw terminals
- **Ring Terminals**: Use on battery bolt connections
- **Heat Shrink**: Cover any soldered joints

## Step 1: Battery to MPPT Controller

Crimp **ring terminals** onto battery wires:
```
Battery (+) ────→ MPPT "BATT+" terminal
Battery (-) ────→ MPPT "BATT-" terminal
```

**Do this FIRST** - most MPPT controllers require battery before solar.

## Step 2: Solar to MPPT Controller

Pass solar wires through cable gland. Crimp **ferrules**:
```
Solar (+) ────→ MPPT "PV+" terminal
Solar (-) ────→ MPPT "PV-" terminal
```

## Step 3: MPPT Load to Fuse Box

Crimp **ferrules** on these wires:
```
MPPT "LOAD+" ──→ 15A Fuse ──→ Master Switch ──→ Fuse Box (+)
MPPT "LOAD-" ──→ Fuse Box Ground Bus (-)
```

## Step 4: Power the Arduino

1. Insert **5A fuse** into Fuse Box Slot 1
2. Crimp **ferrules** and wire:
```
Fuse Box Slot 1 Output ────→ Arduino "Vin" pin
Fuse Box Ground        ────→ Arduino "GND" pin
```

## Step 5: Power the Motor Driver

1. Insert **10A fuse** into Fuse Box Slot 2
2. Wire it:
```
Fuse Box Slot 2 Output ────→ L298N "+12V" terminal
Fuse Box Ground        ────→ L298N "GND" terminal
```

## Step 6: Power the Buck Converter

1. Insert **5A fuse** into Fuse Box Slot 3
2. Wire it:
```
Fuse Box Slot 3 Output ────→ Buck Converter "IN+"
Fuse Box Ground        ────→ Buck Converter "IN-"
```

3. **IMPORTANT: Adjust Buck Converter to 5V!**
   - Use multimeter on output terminals
   - Turn the potentiometer until it reads exactly **5.0V**

This 5V rail powers the Servo and ESP32-CAM (they need more current than Arduino can provide).

---

# STOP. MISSION 2 COMPLETE.

## TEST NOW:
1. Turn on Master Switch
2. Does Arduino LED light up?
3. Does Motor Driver LED light up?
4. Does Buck Converter output show 5V on multimeter?
5. If YES → Turn off and continue
6. If NO → Check connections with multimeter

---

# MISSION 3: Sensors (The "Nerves")

**Goal:** Connect sensors to Arduino. One group at a time.

**Pro Tips:**
- Use **JST connectors** for wires leaving the box (DO probe, load cell) so you can unplug later
- Solder **0.1uF ceramic capacitors** across VCC and GND of sensors to reduce noise

## Group A: The I2C Pair (Pins 20 & 21)

These share the same pins:

### RTC Module (DS3231):
```
RTC VCC ────→ Arduino 5V
RTC GND ────→ Arduino GND
RTC SDA ────→ Arduino Pin 20 (SDA)
RTC SCL ────→ Arduino Pin 21 (SCL)
```

### LCD Display:
```
LCD VCC ────→ Arduino 5V
LCD GND ────→ Arduino GND
LCD SDA ────→ Arduino Pin 20 (SDA)  ← same as RTC
LCD SCL ────→ Arduino Pin 21 (SCL)  ← same as RTC
```

**Note:** Both devices share SDA/SCL - this is normal for I2C.

---

## Group B: The Analog Pair (Pins A0 & A1)

### Voltage Sensor (Battery Monitor):
```
Voltage Sensor VCC    ────→ Arduino 5V
Voltage Sensor GND    ────→ Arduino GND
Voltage Sensor Signal ────→ Arduino A0

Voltage Sensor (+) input ──→ Battery + (or MPPT Load+)
Voltage Sensor (-) input ──→ Battery - (Ground)
```

### DO Sensor (Dissolved Oxygen):

**Optional:** Solder a **0.1uF ceramic capacitor** across VCC and GND to filter noise.

```
DO Sensor VCC    ────→ Arduino 5V
DO Sensor GND    ────→ Arduino GND
DO Sensor Signal ────→ Arduino A1
```

Use **JST connector** on DO probe cable for easy disconnect!

---

## Group C: The Load Cell (Pins 8 & 9)

### HX711 Amplifier:

**Optional:** Solder a **0.1uF ceramic capacitor** across VCC and GND.

```
HX711 VCC ────→ Arduino 5V
HX711 GND ────→ Arduino GND
HX711 DT  ────→ Arduino Pin 8
HX711 SCK ────→ Arduino Pin 9
```

Load cell wires to HX711:
```
HX711 E+ ────→ Load Cell Red
HX711 E- ────→ Load Cell Black
HX711 A+ ────→ Load Cell White
HX711 A- ────→ Load Cell Green
```
*(Wire colors may vary - check your load cell datasheet)*

---

# STOP. MISSION 3 COMPLETE.

All input devices (sensors) are now connected!

---

# MISSION 4: Actuators & Camera

**Goal:** Connect the things that move or see.

## DC Motor (Feeding Spinner)

### Motor Driver Logic Wires:
```
L298N IN1 ────→ Arduino Pin 10
L298N IN2 ────→ Arduino Pin 11
L298N ENA ────→ Arduino Pin 12 (PWM speed control)
```

### Motor Power Wires:
```
Motor Wire 1 ────→ L298N OUT1
Motor Wire 2 ────→ L298N OUT2
```

### Motor Protection:
Install **flyback diodes** across motor terminals to prevent voltage spikes:
```
Diode 1: Striped end (cathode) → Motor terminal connected to OUT1
         Plain end (anode)     → Motor terminal connected to OUT2

Diode 2: Striped end (cathode) → Motor terminal connected to OUT2
         Plain end (anode)     → Motor terminal connected to OUT1
```
*(Forms an "X" pattern across motor terminals)*

---

## Servo Motor (Gate/Shaker)

**IMPORTANT:** Power servo from Buck Converter, NOT Arduino 5V pin!

### Power (from Buck Converter 5V output):
```
Servo VCC (Red)         ────→ Buck Converter OUT+ (5V)
Servo GND (Brown/Black) ────→ Buck Converter OUT- (GND)
```

### Capacitor for Power Smoothing:
Install **1000uF capacitor** across Buck Converter output:
```
Capacitor (+) leg ────→ Buck Converter OUT+ (5V)
Capacitor (-) leg ────→ Buck Converter OUT- (GND)
```
*This prevents voltage drops when servo moves suddenly.*

### Signal (to Arduino):
```
Servo Signal (Orange/Yellow) ────→ Arduino Pin 6 (PWM)
```

Use **JST connector** on servo wires for easy disconnect!

---

## ESP32-CAM (Power Only!)

Power from Buck Converter (same 5V rail as Servo):
```
ESP32-CAM 5V  ────→ Buck Converter OUT+ (5V)
ESP32-CAM GND ────→ Buck Converter OUT- (GND)
```

**That's it!** The camera communicates via WiFi - no data wires needed.

---

### IMPORTANT: Flash ESP32-CAM BEFORE mounting!

1. **Connect FTDI programmer:**
```
FTDI 5V  ────→ ESP32-CAM 5V
FTDI GND ────→ ESP32-CAM GND
FTDI TX  ────→ ESP32-CAM U0R
FTDI RX  ────→ ESP32-CAM U0T
GND      ────→ ESP32-CAM IO0  ← REQUIRED FOR UPLOAD
```

2. **Update WiFi credentials** in `firmware/camera_server/camera_server.ino`:
```cpp
const char* WIFI_SSID = "YourWiFiName";
const char* WIFI_PASSWORD = "YourPassword";
```

3. **Upload in Arduino IDE:**
   - Board: "AI Thinker ESP32-CAM"
   - Click Upload

4. **After upload:**
   - Disconnect IO0 from GND
   - Press RESET button
   - Open Serial Monitor (115200 baud)
   - Look for IP address
   - Test video stream in browser: `http://[IP_ADDRESS]/stream`

5. **Mount in waterproof housing** aimed at pond

---

# MISSION 5: Final Cable Management

**Goal:** Clean up and protect all connections.

1. **Heat Shrink**: Apply to all soldered wire joints
2. **Zip Ties**: Bundle loose wires together
3. **Check**: Ensure no loose wires can touch circuit boards
4. **Seal**: Apply silicone around cable glands if needed

---

# YOU ARE DONE!

## Quick Reference: Arduino Pin Summary

| Pin | Connected To |
|-----|-------------|
| Vin | 12V from Fuse Box |
| 5V | Powers sensors, RTC, LCD |
| GND | Common ground |
| A0 | Voltage Sensor |
| A1 | DO Sensor |
| D6 | Servo Signal |
| D8 | HX711 DT |
| D9 | HX711 SCK |
| D10 | L298N IN1 |
| D11 | L298N IN2 |
| D12 | L298N ENA |
| D18 (TX1) | → Logic Shifter → ESP32 RX2 |
| D19 (RX1) | → Logic Shifter → ESP32 TX2 |
| D20 (SDA) | RTC + LCD (shared I2C) |
| D21 (SCL) | RTC + LCD (shared I2C) |

## Where Your Small Parts Go

| Component | Where It Goes |
|-----------|---------------|
| **Wire Ferrules** | Every screw terminal (MPPT, Fuse Box, Buck Converter, ESP32 Extender) |
| **Ring Terminals** | Battery bolt connections |
| **JST Connectors** | DO sensor probe, Load cell, Servo, Solar panel (easy disconnect) |
| **Heat Shrink** | All soldered wire joints |
| **1000uF Capacitor** | Across Buck Converter output (+ to 5V, - to GND) |
| **0.1uF Ceramic Caps** | Across sensor VCC/GND pins (noise filter) |
| **10k Resistors** | I2C pull-ups if needed (usually not required) |
| **Flyback Diodes** | Across DC motor terminals (spike protection) |
| **Zip Ties** | Bundle wires neatly inside enclosure |

## Fuse Box Configuration

| Slot | Fuse | Powers |
|------|------|--------|
| 1 | 5A | Arduino Vin |
| 2 | 10A | L298N Motor Driver (12V) |
| 3 | 5A | Buck Converter → Servo + ESP32-CAM |
| 4 | 5A | Spare |

---

## Troubleshooting

### No Power?
1. Master switch ON?
2. Check main 15A fuse
3. Check individual fuses in fuse box
4. Measure battery voltage with multimeter

### Arduino Not Starting?
1. Check Vin (should be 7-12V)
2. Try USB power to isolate issue

### Motor Not Spinning?
1. Check 12V at motor driver
2. Verify control pins (10, 11, 12)
3. Test motor directly with 12V
4. Check flyback diodes orientation

### Servo Not Moving?
1. Check Buck Converter output (should be 5V)
2. Verify signal wire on Pin 6
3. Check 1000uF capacitor polarity (+ to 5V)

### ESP32-CAM No Video?
1. Correct WiFi credentials?
2. Stable 5V from Buck Converter?
3. Correct IP address?

---

## Next Steps

**Phase 5:** Update Arduino firmware to read real sensors (replace mock data)

**Phase 6:** Calibrate sensors and deploy at pond
