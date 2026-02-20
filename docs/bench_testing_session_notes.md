# OxyFeeder Bench Testing Session Notes
**Date:** February 20, 2026
**Purpose:** Diagnose and test all sensors after 12V incident damage

---

## Executive Summary

The Arduino Mega suffered damage from a previous 12V incident that affected multiple pins. Through systematic testing, we identified damaged pins and found working alternatives. Most sensors are now functional except the RTC module which needs replacement.

---

## Hardware Damage Assessment

### Damaged Pins (Do NOT Use)

| Pin | Type | Symptom | Notes |
|-----|------|---------|-------|
| **A0** | Analog | Reads ~1.7V instead of expected 2.4V | Internal leakage to ground |
| **Pin 8** | Digital | Shows 0 with INPUT_PULLUP | Shorted internally |
| **Pin 9** | Digital | Shows 0 with INPUT_PULLUP | Shorted internally |
| **Pin 21** | Digital/I2C SCL | Variable readings | May have issues |

### Healthy Pins (Confirmed Working)

| Pin | Type | Test Result | Now Used For |
|-----|------|-------------|--------------|
| **A1** | Analog | Responds to input | DO Sensor |
| **A2** | Analog | Reads correctly | Voltage Sensor (12.3V confirmed) |
| **Pin 10** | Digital | Shows 1 with INPUT_PULLUP | HX711 DT |
| **Pin 11** | Digital | Shows 1 with INPUT_PULLUP | HX711 SCK |
| **Pin 20** | Digital/I2C SDA | Shows 1 with INPUT_PULLUP | RTC SDA (when module replaced) |
| **Pin 21** | Digital/I2C SCL | Shows 1 with INPUT_PULLUP | RTC SCL (when module replaced) |
| **Pin 22** | Digital | Shows 1 with INPUT_PULLUP | Available |
| **Pin 23** | Digital | Shows 1 with INPUT_PULLUP | Available |
| **Pin 24** | Digital | Shows 1 with INPUT_PULLUP | Available |

---

## Sensor Test Results

### 1. Voltage Sensor (25V Module)
- **Status:** ✅ WORKING
- **Pin Used:** A2 (moved from damaged A0)
- **Test Results:**
  - Raw ADC: ~502-530
  - S-Pin Voltage: ~2.45V
  - Calculated Battery: **12.3V - 13.0V**
- **Wiring:**
  - S (Signal) → A2
  - + (middle) → Not connected
  - - (GND) → GND

### 2. Load Cell / HX711
- **Status:** ✅ WORKING
- **Pins Used:** DT=Pin 10, SCK=Pin 11 (moved from damaged Pin 8/9)
- **Test Results:**
  - HX711 detected successfully
  - Raw readings: ~132,000 baseline
  - Responds to pressure: jumps to ~135,000-140,000
  - Returns to baseline when released
- **Calibration Factor:** ~2.0 (approximate, from iPhone 12 @ 164g test)
- **Wiring:**
  - HX711 VCC → 5V
  - HX711 GND → GND
  - HX711 DT → Pin 10
  - HX711 SCK → Pin 11
  - Load Cell Red → E+
  - Load Cell Black → E-
  - Load Cell White → A-
  - Load Cell Green → A+

### 3. RTC DS3231
- **Status:** ❌ MODULE DAMAGED (needs replacement)
- **Arduino Pins:** Pin 20/21 are HEALTHY
- **Issue:** RTC module itself is damaged - pulls I2C lines low when connected
- **Evidence:**
  - Pin 20/21 show 1|1 when nothing connected (healthy)
  - I2C scanner finds no devices even with direct Arduino connection
  - Module pulls pins to 0 when connected
- **Solution:** Order replacement DS3231 module from MakerLab

### 4. DO Sensor (Dissolved Oxygen)
- **Status:** ⏳ PENDING TEST
- **Pin:** A1
- **Wiring:**
  - Signal → A1
  - VCC → 5V
  - GND → GND

---

## Pin Mapping Summary (Final Configuration)

```
OXYFEEDER PIN ASSIGNMENTS (Post-Damage)
========================================

ANALOG PINS:
  A0  - DAMAGED (do not use)
  A1  - DO Sensor Signal
  A2  - Voltage Sensor Signal

DIGITAL PINS:
  Pin 4   - Motor IN1 (unchanged)
  Pin 5   - Motor IN2 (unchanged)
  Pin 6   - Servo PWM (unchanged)
  Pin 7   - Buzzer (unchanged)
  Pin 8   - DAMAGED (do not use)
  Pin 9   - DAMAGED (do not use)
  Pin 10  - HX711 DT (moved from Pin 8)
  Pin 11  - HX711 SCK (moved from Pin 9)
  Pin 12  - Motor ENA (unchanged)

I2C PINS:
  Pin 20 (SDA) - RTC DS3231 (healthy, awaiting new module)
  Pin 21 (SCL) - RTC DS3231 (healthy, awaiting new module)

SPI/LCD PINS:
  Pin 40  - LCD CS (unchanged)
  Pin 38  - LCD DC (unchanged)
  Pin 51  - LCD MOSI (unchanged)
  Pin 52  - LCD SCK (unchanged)

SERIAL:
  Serial1 (TX1=18, RX1=19) - ESP32 Communication
```

---

## Diagnostic Test Files Created

| File | Purpose |
|------|---------|
| `firmware/voltage_sensor_test/voltage_sensor_test.ino` | Test A0/A2 pins |
| `firmware/loadcell_test/loadcell_test.ino` | Test HX711 on various pins |
| `firmware/rtc_test/rtc_test.ino` | I2C scanner and pin health check |
| `firmware/all_sensors_test/all_sensors_test.ino` | Combined sensor test |
| `firmware/bench_test_diagnostic/bench_test_diagnostic.ino` | Comprehensive diagnostic |

---

## Testing Procedures Used

### Pin Health Check Code
```cpp
void setup() {
  Serial.begin(115200);
  pinMode(PIN_NUMBER, INPUT_PULLUP);
}
void loop() {
  Serial.println(digitalRead(PIN_NUMBER));
  delay(500);
}
```
- **Result = 1:** Pin is healthy
- **Result = 0:** Pin is damaged (shorted to ground)

### I2C Scanner Code
Used standard I2C scanner to detect devices at addresses 0x00-0x7F.
- DS3231 RTC expected at 0x68
- DS3231 EEPROM expected at 0x57

### Voltage Sensor Calculation
```cpp
int rawValue = analogRead(A2);
float sPinVoltage = rawValue * (5.0 / 1023.0);
float batteryVoltage = sPinVoltage * 5.0;  // 5:1 voltage divider
```

---

## Extender Board Notes

- Initial testing through extender board showed issues
- Direct connection to Arduino Mega pins (bypassing extender) confirmed Arduino pins are healthy
- Extender board may have shorts or connection issues
- **Recommendation:** Test extender board separately or replace if needed

---

## Next Steps

1. **DO Sensor Test** - Connect and verify voltage readings on A1
2. **Order Replacement RTC** - DS3231 module from MakerLab
3. **LCD Test** - Verify SPI communication still works
4. **Update Main Firmware** - Integrate new pin assignments
5. **ESP32 Communication Test** - Verify Serial1 still works
6. **Full Integration Test** - All sensors + BLE communication

---

## Shopping List (Replacement Parts)

| Item | Quantity | Notes |
|------|----------|-------|
| DS3231 RTC Module | 1 | Current module damaged |
| Arduino Mega (backup) | 1 | Optional - current has multiple damaged pins |

---

## Lessons Learned

1. **12V incidents cause cascading damage** - Multiple pins can be affected
2. **Always test pins with INPUT_PULLUP** - Quick way to identify shorted pins
3. **Arduino Mega has pin redundancy** - Many alternative pins available
4. **I2C scanner is essential** - Helps identify module vs wiring issues
5. **Test direct connection first** - Bypass extenders/shields when debugging

---

## Git Commits for This Session

- `e1c517f` - feat(firmware): add diagnostic test sketches for bench testing

---

## Contact/Reference

- AI Assistant: Claude (Anthropic)
- Development Tool: Claude Code CLI
- AI Studio: Google AI Studio (secondary reference)
