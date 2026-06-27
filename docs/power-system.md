# OxyFeeder: Solar Energy & Power Regulation System

## 1. Architecture

**Complete Power Flow:**
```
Solar Panel (50W)
    ↓
MPPT Charge Controller (manages charging)
    ↓
12V 20Ah Battery (main storage)
    ↓
MPPT Load Output (regulated 12V)
    ↓
Inline Fuse Holder
    ↓
Master Power Switch
    ↓
12V Fuse Box (distribution with individual fuse protection)
    ├→ Output 1: Arduino Vin (12V → internal regulator → 5V logic)
    │            └→ Arduino 5V pin → ESP32 VIN (powers ESP32)
    └→ Output 2: Motor Driver 12V input
```

**Key Design Principle:** Arduino acts as the power hub, receiving 12V and distributing 5V to ESP32. No separate buck converters needed - Arduino's onboard regulator handles voltage conversion.

---

## 2. Components

### Energy Generation & Storage
- **Solar Panel:** 50W monocrystalline — high efficiency, field-deployable
- **MPPT Controller:** 10A capacity, maximizes charge efficiency (especially on cloudy days)
- **Battery:** 12V 20Ah lead-acid or LiFePO4 (provides ~48h autonomy without sun)

### Power Distribution
- **MPPT Load Output:** Regulated 12V output for system loads
- **Inline Fuse Holder:** Primary protection between battery and load (recommended: 5A-10A fuse)
- **Master Power Switch:** Manual on/off control for entire system
- **12V Fuse Box:** Multi-output distribution with individual fuse protection per circuit
  - Output 1 fuse: 2A (for Arduino + ESP32)
  - Output 2 fuse: 5A (for Motor Driver)

### Monitoring
- **Voltage Sensor:** Connected to Arduino pin A0, monitors battery voltage
  - Alert threshold: < 11.5V (low battery warning)
  - Critical shutdown: < 11.0V (prevent battery damage)

### Voltage Regulation
- **Arduino Internal Regulator:** Converts 12V (Vin) → 5V (for logic and ESP32)
- **ESP32 Internal Regulator:** Converts 5V (VIN on extender) → 3.3V (for ESP32 logic)
- No external buck converters required in current design

---

## 3. Safety & Protection

### Electrical Protection
- **Inline fuse:** Primary protection between battery and all loads
- **Individual fuses:** Each circuit in fuse box has dedicated protection
- **Reverse polarity protection:** Built into MPPT controller
- **Low-voltage disconnect:** MPPT controller prevents deep discharge
- **Wire gauging:** 14-16 AWG for main power lines, 22 AWG for signal/logic

### System Monitoring
- Battery voltage continuously monitored via Arduino
- App displays battery percentage in real-time
- Automatic alerts when battery drops below safe threshold
- Optional: Future implementation of sleep mode to conserve power

### Physical Protection
- Waterproof enclosure with proper cable glands
- All connections sealed against moisture
- Fuse box mounted inside enclosure for weather protection

---

## 4. Power Budget (Estimated)

**Continuous Draw:**
- Arduino Mega: ~100mA @ 5V = 0.5W
- ESP32 (active BLE): ~160mA @ 3.3V = 0.53W
- Sensors (DO, RTC, Load Cell): ~50mA @ 5V = 0.25W
- LCD Display: ~20mA @ 5V = 0.1W
- **Total Idle:** ~1.4W (~120mAh @ 12V)

**Peak Draw (during feeding):**
- Motor Driver + Gear Motor: ~2A @ 12V = 24W (for ~10 seconds)
- Servo: ~500mA @ 5V = 2.5W (brief actuation)

**Daily Energy:**
- Idle (23.5 hrs): 1.4W × 23.5 = 33Wh
- Feeding (3 cycles × 10 sec): negligible
- **Total:** ~35-40Wh/day

**Battery Capacity:** 12V × 20Ah = 240Wh (~6 days autonomy, 2-3 days recommended minimum)

**Solar Input:** 50W × 4-5 peak sun hours = 200-250Wh/day (sufficient with margin)

---

## 5. Field Deployment & Maintenance

### Installation
- **Panel Tilt:** Set to local latitude angle for optimal year-round performance
- **Panel Orientation:** Face true south (northern hemisphere) or true north (southern hemisphere)
- **Mounting:** Secure against wind, ensure no shading from trees/structures

### Regular Maintenance
- **Monthly:** Visual inspection of all connections and cable glands
- **Quarterly:** Clean solar panel surface (remove dust, bird droppings, debris)
- **Semi-annually:** Check battery terminals for corrosion, verify voltage under load
- **Annually:** Inspect all wiring for UV damage or rodent activity

### Battery Care
- Maintain temperature range: 10-30°C (50-86°F) for optimal lifespan
- Monitor voltage via app - replace battery if unable to hold charge
- For lead-acid: Check water level if applicable (sealed batteries exempt)
- For LiFePO4: Expect 2000+ charge cycles vs ~500 for lead-acid

---

## 6. Troubleshooting

**Battery not charging:**
- Check solar panel voltage at MPPT input (should be >14V in sun)
- Verify MPPT controller is functioning (check indicator LEDs)
- Inspect fuse between battery and MPPT

**System not powering on:**
- Check master switch position
- Verify battery voltage with multimeter (should be >11V)
- Check fuse box - replace blown fuses

**Intermittent power:**
- Inspect all screw terminals for tight connections
- Check Arduino Vin voltage (should be stable 12V)
- Verify ESP32 VIN connection to Arduino 5V
