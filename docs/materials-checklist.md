# OxyFeeder: Complete Materials Checklist

This document lists all materials needed for the OxyFeeder project, organized by development phase. Check items off as you verify you have them.

---

## 📦 Phase 2: Communication Hub ("The Brain")

**Status:** 🔄 CURRENT PHASE - Gather these items NOW

### Microcontrollers
- [ ] **Arduino Mega 2560** (with pre-attached Female Headers)
- [ ] **ESP32 Development Board** (with pre-attached Male Pins)
- [ ] **ESP32 Screw Terminal Extender Board** (breakout board with screw terminals)

### Bridge Components
- [ ] **Logic Level Shifter Module** (Bi-directional, 4-channel minimum)
  - Must support 5V ↔ 3.3V conversion
  - HV and LV sides clearly marked
- [ ] **Prototyping PCB (Perfboard)** - Small piece (5cm × 7cm is sufficient)
- [ ] **Male Header Pins** - For soldering wires to plug into Arduino female headers

### Wiring & Power (Phase 2 Testing)
- [ ] **22 AWG Hook-up Wire** (both solid core and stranded)
  - Solid core: Best for perfboard soldering
  - Stranded: Better for connections to screw terminals
  - Recommended colors: Red, Black, Yellow, Green (for organization)
- [ ] **USB Cable for Arduino** (Type-B Male to Type-A)
- [ ] **USB Cable for ESP32** (Micro-USB or USB-C, depending on your ESP32 model)

### Tools Required (Phase 2)
- [ ] **Soldering Iron** (temperature-controlled recommended, 300-350°C)
- [ ] **Solder** (60/40 or 63/37 tin/lead, or lead-free)
- [ ] **Wire Strippers/Cutters**
- [ ] **Small Flathead Screwdriver** (for ESP32 Extender screw terminals)
- [ ] **Multimeter** (CRITICAL - verify 5V and 3.3V before connecting)
- [ ] **Heat Shrink Tubing** (various sizes, for insulating soldered connections)
- [ ] **Helping Hands/PCB Holder** (optional but very helpful for soldering)

---

## 📦 Phase 4: Full System Assembly

**Status:** ⏳ FUTURE - Keep these stored safely until Hub is tested

### Power System

#### Energy Generation & Storage
- [ ] **Solar Panel** - 50W, 12V monocrystalline
- [ ] **Solar Panel Mounting Brackets** (adjustable tilt)
- [ ] **MPPT Solar Charge Controller** - 10A minimum
- [ ] **12V Lead-Acid Battery** - 20Ah capacity
  - Alternative: 12V LiFePO4 (longer lifespan, higher cost)

#### Power Distribution & Safety
- [ ] **Inline Fuse Holder** - For main battery protection
- [ ] **Fuses** - 5A and 10A (automotive blade type)
- [ ] **12V Fuse Box** - Multi-output distribution (2+ circuits)
  - Must support individual fuse protection per output
- [ ] **Master Power Switch** - Rated for 12V, 5A minimum
- [ ] **MC4 Connectors** - For solar panel wiring (usually included with panel)
- [ ] **XT60 Connectors** - For battery connections (optional but recommended)

#### Power Wiring
- [ ] **14 AWG Wire** - For battery to MPPT, MPPT to fuse box (Red & Black)
- [ ] **16 AWG Wire** - For fuse box to components (Red & Black)
- [ ] **Ring Terminals** - For secure battery connections
- [ ] **Wire Ferrules** - For screw terminal connections (optional but cleaner)

#### Voltage Regulation (Optional)
- [ ] **DC-DC Buck Converter Module** (12V → 5V, 3A)
  - **Note:** Only needed if:
    - Your servo draws >500mA continuously
    - You want to reduce heat on Arduino's onboard regulator
  - **Current Design:** Arduino's onboard regulator handles all 5V distribution

### Sensors ("The Senses")

- [ ] **DFRobot Gravity Analog Dissolved Oxygen Sensor**
  - [ ] DO Probe (membrane-type)
  - [ ] Calibration solutions (if not included)
  - Connects to: Arduino Pin A1

- [ ] **Load Cell** (5kg-20kg capacity for feed hopper)
  - [ ] **HX711 Load Cell Amplifier Module**
  - Connects to: Arduino Pin 8 (DT), Pin 9 (SCK)

- [ ] **DS3231 Real-Time Clock Module** (I2C interface)
  - Includes coin cell battery for timekeeping
  - Connects to: Arduino Pin 20 (SDA), Pin 21 (SCL)

- [ ] **Voltage Sensor Module** (0-25V DC measurement range)
  - For battery voltage monitoring
  - Connects to: Arduino Pin A0

- [ ] **I2C LCD Display** (16×2 or 20×4 character display)
  - **Type:** I2C interface (uses PCF8574 I2C expander)
  - **NOT** TFT/SPI display (different pin requirements)
  - Connects to: Arduino Pin 20 (SDA), Pin 21 (SCL) - shared with RTC

**Note:** RTC and LCD share the I2C bus (SDA/SCL pins). This is normal and supported.

### Actuators ("The Muscles")

- [ ] **12V DC Gear Motor** (with gearbox, 100-300 RPM)
- [ ] **Feed Spinner Plate** (attaches to motor shaft)
- [ ] **L298N Motor Driver Module** (or equivalent H-bridge)
  - Connects to: Arduino Pin 10 (IN1), Pin 11 (IN2), Pin 12 (ENA)
  - Power: 12V from fuse box

- [ ] **Servo Motor** (SG90 or MG90S, 5V operation)
  - For actuator control (hopper gate, etc.)
  - Connects to: Arduino Pin 6 (PWM signal)
  - Power: Arduino 5V pin (if <500mA draw)

#### Motor Protection Components
- [ ] **Flyback Diodes** (1N4007 or similar, 2-4 pieces)
  - Protects motor driver from back-EMF
- [ ] **1000µF Electrolytic Capacitor** (16V or 25V)
  - Smooths power supply noise from motor
- [ ] **Optocouplers** (PC817 or similar, optional)
  - Isolates Arduino from motor driver for extra protection

### Housing & Physical Assembly

- [ ] **Waterproof Enclosure** (IP65 or higher rating)
  - Size: Large enough for all components (~30cm × 20cm × 15cm minimum)
  - Material: ABS plastic or aluminum
- [ ] **Cable Glands** (PG7, PG9, or PG11 sizes)
  - Waterproof cable entry/exit points
  - Need at least 4: Solar, Motor, DO Probe, Antenna
- [ ] **Mounting Hardware**
  - [ ] M3/M4 Screws and Standoffs (for PCB mounting)
  - [ ] Cable Zip Ties (UV-resistant for outdoor use)
  - [ ] Double-sided foam tape (for temporary mounting)
  - [ ] Sensor Mounting Brackets (for DO probe, LCD)
- [ ] **Silicone Sealant** (for additional waterproofing around glands)

### Connectivity & UI (Optional/Future Features)

#### Phase 3 (Current Blueprint)
- [x] ESP32 Bluetooth Low Energy (BLE) - **Already included in Phase 2**
- [x] Flutter Mobile App - **Already developed**

#### Future Phases (Not in Current Blueprint)
- [ ] **GSM Module** (SIM800L or SIM7600)
  - **Status:** Future feature (not in Phase 1-6)
  - For SMS/cellular alerts when BLE out of range
  - Requires: SIM card, antenna, additional power budget

- [ ] **ESP32-CAM Module**
  - **Status:** Future feature
  - For visual monitoring of feed hopper/pond
  - Requires: SD card, different power/mounting considerations

- [ ] **Buzzer** (Active or Passive, 5V)
  - **Status:** Optional local alerts
  - For audible alerts at pond site

**Note:** GSM and Camera features are **not required** for core functionality. BLE + mobile app provides all necessary monitoring and control.

---

## 🛠️ Complete Tool List

### Essential (Required)
- [ ] **Soldering Iron** (temperature-controlled, 300-400°C range)
- [ ] **Solder** (0.8mm-1.0mm diameter)
- [ ] **Wire Strippers/Cutters** (20-14 AWG range)
- [ ] **Screwdriver Set** (flathead and Phillips, various sizes)
- [ ] **Multimeter** (voltage, continuity, resistance)

### Highly Recommended
- [ ] **Heat Gun** or **Lighter** (for heat shrink tubing)
- [ ] **Helping Hands/PCB Holder** (makes soldering much easier)
- [ ] **Desoldering Pump** or **Wick** (for fixing mistakes)
- [ ] **Wire Ferrule Crimping Tool** (if using ferrules)
- [ ] **Label Maker** or **Masking Tape + Marker** (for wire identification)

### For Phase 4 (Physical Assembly)
- [ ] **Electric Drill** + **Drill Bits** (for enclosure holes)
- [ ] **Step Drill Bit** or **Hole Saw Set** (for cable gland holes)
- [ ] **Wrench Set** (for battery terminals, mounting nuts)
- [ ] **Hot Glue Gun** (for strain relief on cables)

---

## 📋 Verification Checklist by Phase

### ✅ Before Starting Phase 2B (Current)
**Can you answer YES to all of these?**
- [ ] I have the Arduino Mega with Female Headers
- [ ] I have the ESP32 with Screw Terminal Extender
- [ ] I have the Logic Level Shifter module
- [ ] I have perfboard, solder, and a soldering iron
- [ ] I have 22 AWG wire in multiple colors
- [ ] I have both USB cables for programming
- [ ] I have a multimeter to verify voltages

**If YES to all:** You're ready to build the Bridge! Proceed with Phase 2B assembly.

### ✅ Before Starting Phase 2C (Testing)
- [ ] Communication Hub physically assembled
- [ ] Both firmwares (Arduino + ESP32) compiled successfully in Arduino IDE
- [ ] No short circuits verified with multimeter (continuity test)
- [ ] All solder joints are clean and shiny (no cold joints)

### ✅ Before Starting Phase 4 (Full Assembly)
- [ ] Phase 2C complete (Hub tested, BLE verified with phone)
- [ ] Phase 3 complete (App connects to real hardware)
- [ ] All Phase 4 components gathered and verified
- [ ] Waterproof enclosure tested (sprayed with water to verify sealing)
- [ ] Solar panel and battery verified with multimeter (correct voltages)

---

## 💡 Purchasing Notes

### Budget-Friendly Alternatives
- **Perfboard:** Can substitute with prototyping breadboard for initial testing, but solder to perfboard for permanent installation
- **Heat Shrink Tubing:** Electrical tape works in a pinch but is less durable
- **Ferrules:** Can use tinned wire ends instead (less professional but functional)
- **LiFePO4 Battery:** Lead-acid is cheaper upfront; upgrade to LiFePO4 later if budget allows

### Where to Buy
- **Microcontrollers/Sensors:** AliExpress, Amazon, Adafruit, SparkFun, DFRobot
- **Solar/Battery Components:** Specialized solar suppliers, Amazon, local hardware stores
- **Enclosures:** IP-rated boxes from electrical supply stores or Amazon
- **Wire/Connectors:** Electrical supply stores often cheaper than hobby shops for bulk

### Quality Matters
**Don't cheap out on:**
- Voltage sensors (inaccurate readings = unreliable battery monitoring)
- MPPT controller (cheap ones can damage battery or underperform)
- Waterproof enclosure (failure = system death)
- Fuses and fuse holders (safety critical)

**Can be budget-friendly:**
- Perfboard, wire, connectors
- Servo motor
- Prototyping supplies

---

## 🔍 Troubleshooting Materials Issues

**"My Logic Level Shifter has 8 channels but I only need 4"**
- That's fine! Use 4 channels, leave others unconnected. More channels = more flexibility for future expansion.

**"My ESP32 Extender doesn't have enough screw terminals"**
- You may need to share GND terminals (this is safe - all grounds connect together)
- Consider a separate screw terminal block for additional connections

**"I only have stranded wire, not solid core"**
- For perfboard: Tin the wire ends heavily with solder to make them rigid
- Alternatively: Use male header pins soldered to perfboard, then plug stranded wires in

**"My battery is 18Ah, not 20Ah"**
- Close enough! Anything 15-25Ah works fine. Adjust autonomy calculations accordingly.

**"I have a buck converter but the blueprint says I don't need it"**
- You can use it! Options:
  1. Don't use it (Arduino regulator handles everything)
  2. Use it for servo power if servo draws >500mA
  3. Use it to reduce heat on Arduino regulator (powers ESP32 from buck instead)

---

## 📊 Materials Summary

| **Phase** | **Components** | **Estimated Cost** | **Ready?** |
|-----------|----------------|-------------------|------------|
| Phase 2 (Current) | 3 boards, shifter, perfboard, wire | $30-50 USD | ☐ |
| Phase 4 (Power) | Solar, battery, MPPT, fuse box | $150-250 USD | ☐ |
| Phase 4 (Sensors) | DO, RTC, HX711, voltage, LCD | $80-120 USD | ☐ |
| Phase 4 (Actuators) | Motor, driver, servo | $30-50 USD | ☐ |
| Phase 4 (Housing) | Enclosure, glands, mounting | $40-80 USD | ☐ |
| **Total Estimated** | | **$330-550 USD** | |

**Note:** Costs vary by region and supplier. This is a ballpark estimate.

---

## ✅ Final Verification

You mentioned you have **everything**. Based on this checklist, verify one more time:

**For Phase 2 (NOW):**
- [x] All microcontrollers? (Arduino + ESP32 + Extender)
- [x] Logic Level Shifter?
- [x] Perfboard and soldering supplies?
- [x] Wires and USB cables?

**If all checked:** You're 100% ready to proceed with Phase 2B - Building the Bridge! 🎉

**For Phase 4 (LATER):**
- Store all components in a labeled box
- Keep moisture-sensitive items (battery, sensors) in dry location
- Test solar panel and battery voltages periodically if stored long-term

---

*Last Updated: 2025-11-22*
*Aligned with: OxyFeeder Master Blueprint (Parts 1-6)*
