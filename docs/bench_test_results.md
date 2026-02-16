# OxyFeeder Bench Test Results

**Date:** 2026-02-11
**Test Type:** System Diagnostic (Bench Test)
**Firmware Used:** `firmware/bench_test_diagnostic/bench_test_diagnostic.ino`

---

## Summary

| Component | Status | Notes |
|-----------|--------|-------|
| DC Motor (L298N) | ✅ WORKING | Runs/stops as programmed (2s on, 10s off) |
| Servo Motor | ✅ WORKING | Opens (90°) / Closes (0°) every 5 seconds |
| Buzzer | ✅ WORKING | Beeps on startup and actions |
| Arduino Mega | ✅ WORKING | Code running, Serial communication OK |
| Serial Monitor | ✅ WORKING | 9600 baud, readable output |
| Voltage Sensor | ❌ DAMAGED | Needs replacement |
| Load Cell (HX711) | ❓ UNTESTED | Shows 8388607 (max value error) |
| RTC (DS3231) | ❓ UNTESTED | Shows wrong date (21/00/2015) |
| TFT LCD (ILI9341) | ❓ UNTESTED | Not confirmed yet |
| ESP32 Bridge | ❓ UNTESTED | Not confirmed yet |
| GSM Module | ❓ UNTESTED | Not confirmed yet |
| DO Sensor | ⏸️ SKIPPED | Intentionally excluded from this test |

---

## Detailed Findings

### 1. Voltage Sensor Module - DAMAGED

**Problem:** Module reads 0.16V at S pin when input is 12V (should be ~2.4V)

**Diagnosis:**
- Screw terminal (+)(-) receives 12V correctly ✅
- VCC/GND powered by 5V rail correctly ✅
- S pin output: 0.16V (expected ~2.4V) ❌
- Internal voltage divider circuit is damaged

**Root Cause:** Previous incident where 12V was accidentally injected into the 5V rail, damaging the module's internal circuit.

**Solution:** Replace the voltage sensor module
- Part: "25V Voltage Sensor Module Arduino"
- Estimated cost: ₱30-80
- Available: Lazada, Shopee, local electronics stores

**Wiring Reference (for new module):**
```
Module Layout:
┌─────────────────────────────────┐
│   SIDE 1:          SIDE 2:     │
│   ┌─────┬─────┐    ┌───┬───┬───┐
│   │ VCC │ GND │    │ S │ + │ - │
│   └─────┴─────┘    └───┴───┴───┘
└─────────────────────────────────┘

Connections:
- VCC → 5V Rail (perfboard)
- GND → GND Rail (perfboard)
- S   → Arduino A0
- (+) → Fuse Box Slot 1 (12V)
- (-) → Fuse Box GND
```

---

### 2. Load Cell (HX711) - ERROR

**Problem:** Reading shows 8388607 (maximum raw value)

**Possible Causes:**
1. Wiring issue (DT/SCK pins)
2. Load cell not connected to HX711 board
3. HX711 not getting power
4. Module damaged from previous incident

**Current Wiring:**
- DT → Pin 8
- SCK → Pin 9
- VCC → 5V
- GND → GND

**Next Steps:** Verify wiring and test with multimeter

---

### 3. RTC (DS3231) - WRONG TIME

**Problem:** Shows 21/00/2015 21:00:00 (invalid date)

**Possible Causes:**
1. RTC lost power (coin battery dead/missing)
2. RTC never been set
3. Module not communicating via I2C

**Current Wiring:**
- SDA → Pin 20
- SCL → Pin 21
- VCC → 5V (or 3.3V)
- GND → GND

**Next Steps:**
- Check/replace CR2032 coin battery
- Re-upload code to set time to compile time

---

## Power System Status

**Configuration:**
```
Solar Panel → MPPT Controller → Battery
                    ↓
            Master Switch (ON)
                    ↓
              Fuse Box
                    ↓
    ┌───────────────┴───────────────┐
    ↓                               ↓
Arduino VIN (12V)           Buck Converter
    ↓                               ↓
ESP32 (via 5V)              Servo, GSM, ESP32-CAM
```

**Status:** Power system is working correctly
- Fuse Box Slot 1 outputs 12V ✅
- Arduino receives power ✅
- Buck converter provides 5V for peripherals ✅

---

## Serial Monitor Output (Sample)

```
========================================
   OxyFeeder BENCH TEST DIAGNOSTIC
   (DO Sensor EXCLUDED from this test)
========================================

[OK] Buzzer initialized
[OK] Motor driver initialized
[OK] Servo initialized (position: 0)
[OK] Load Cell (HX711) detected
[OK] Load Cell tared
[OK] RTC (DS3231) detected
[OK] TFT Display initialized
[OK] Sent startup message to ESP32 (Serial1)
[OK] Sent AT command to GSM (Serial3)

========================================
         STARTING MAIN TEST LOOP
========================================

----------------------------------------
Test Cycle: 380

VOLTAGE:    4.94 V       ← Wrong (should be ~12V)
DO SENSOR:  (SKIPPED - Not connected)
WEIGHT:     8388607.00 units  ← Error (max value)
TIME:       21/00/2015 21:00:00  ← Wrong date
SERVO:      OPEN (90°)   ← Working
MOTOR:      STOPPED      ← Working
----------------------------------------
```

---

## Action Items

### Immediate (Before Next Test)
1. [ ] **Order replacement Voltage Sensor Module** (25V Arduino)
2. [ ] Check Load Cell (HX711) wiring
3. [ ] Check/replace RTC coin battery (CR2032)

### Next Testing Session
4. [ ] Test Load Cell with proper wiring verification
5. [ ] Test RTC after battery check/replacement
6. [ ] Test TFT LCD display
7. [ ] Test ESP32 Bridge communication
8. [ ] Test GSM Module (AT commands)
9. [ ] Test DO Sensor (after other components verified)

### Hardware to Purchase
| Item | Quantity | Est. Price | Priority |
|------|----------|------------|----------|
| Voltage Sensor Module (25V) | 1 | ₱30-80 | HIGH |
| CR2032 Battery (for RTC) | 1 | ₱20-50 | MEDIUM |

---

## Files Related to This Test

- **Diagnostic Firmware:** `firmware/bench_test_diagnostic/bench_test_diagnostic.ino`
- **Main Firmware (future):** `firmware/oxyfeeder_firmware/oxyfeeder_firmware.ino`
- **Wiring Reference:** See CLAUDE.md → Hardware Architecture

---

## Notes

- DO Sensor was intentionally excluded from this bench test to protect it until power system is verified stable
- The previous incident (12V injected into 5V rail) damaged the voltage sensor module and potentially other components
- Motor and Servo confirmed working - core actuators are OK
- Arduino Mega replacement is functioning correctly

---

*Document created during bench testing session on 2026-02-11*
