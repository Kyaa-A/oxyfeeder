# OxyFeeder: Hardware Architecture Overview

## 1. Subsystems

### MCU (Communication Hub - "The Brain")
**Arduino Mega 2560** (with Female Headers):
- Main controller for sensors and actuators
- Reads all sensors (DO, voltage, load cell, RTC)
- Controls motor driver and servo
- Sends JSON data via Serial1 to ESP32
- Powers ESP32 via 5V connection

**ESP32** (with Screw Terminal Extender):
- BLE bridge for wireless communication
- Receives JSON from Arduino via Serial2
- Broadcasts data to mobile app via Bluetooth Low Energy
- Powered by Arduino 5V → VIN on extender

**Logic Level Shifter** ("Bridge" board on perfboard):
- Converts 5V (Arduino) ↔ 3.3V (ESP32) signals
- HV side connects to Arduino via female headers
- LV side connects to ESP32 Extender via screw terminals
- Ensures safe serial communication between MCUs

### Power
- 50W Solar Panel → MPPT Controller → 12V 20Ah Battery → MPPT Load Output → Fuse → Master Switch → 12V Fuse Box
- Fuse Box distributes power to:
  - Arduino Vin (which then powers ESP32)
  - Motor Driver 12V input

### Sensors
**Pin Mappings:**
- DO Sensor (DFRobot Gravity): Analog A1
- Voltage Sensor (Battery monitor): Analog A0
- RTC (DS3231): SDA → Pin 20, SCL → Pin 21 (I2C)
- Load Cell + HX711: DT → Pin 8, SCK → Pin 9
- LCD Display: SDA → Pin 20, SCL → Pin 21 (shared I2C bus)

### Actuators
**Pin Mappings:**
- Motor Driver (L298N): IN1 → Pin 10, IN2 → Pin 11, ENA → Pin 12
- Servo: PWM → Pin 6

### Connectivity
- ESP32 Bluetooth Low Energy (BLE)
- Service UUID: `0000abcd-0000-1000-8000-00805f9b34fb`
- Characteristic UUID: `0000abce-0000-1000-8000-00805f9b34fb`
- Device Name: "OxyFeeder"

---

## 2. Signal Flow

**Power:**
```
Solar Panel → MPPT Controller → Battery
              MPPT Load → Fuse → Switch → 12V Fuse Box
                                          ├→ Arduino Vin (also powers ESP32)
                                          └→ Motor Driver 12V
```

**Data:**
```
Sensors → Arduino Mega (reads & processes)
          ↓ Serial1 (TX1/RX1)
Logic Level Shifter (5V ↔ 3.3V)
          ↓ Serial2 (RX2/TX2)
ESP32 (receives JSON)
          ↓ BLE notifications
Mobile App (Flutter)
```

**Control:**
```
Arduino → Motor Driver → 12V Gear Motor (feed dispensing)
Arduino → Servo (actuator control)
```

---

## 3. MCU Integration ("Bridge Method")

### Physical Connection
**Arduino Mega → Logic Level Shifter (HV side):**
- 5V → HV
- GND → GND
- Pin 18 (TX1) → HV1
- Pin 19 (RX1) → HV2

**Logic Level Shifter (LV side) → ESP32 Extender:**
- LV → 3V3 terminal (screw)
- GND → GND terminal (screw)
- LV1 → Pin 17 (RX2) terminal (screw)
- LV2 → Pin 16 (TX2) terminal (screw)

**Power Connection:**
- Arduino 5V → ESP32 Extender VIN terminal (screw)

### Communication Protocol
- **Baud Rate:** 9600 (both Serial1 on Arduino and Serial2 on ESP32)
- **Data Format:** JSON string
- **Example:** `{"do": 6.5, "feed": 75, "battery": 82}`
- **Direction:** Unidirectional (Arduino → ESP32 only)

### Current Implementation (Phase 2)
- Arduino sends **simulated** sensor data
- ESP32 receives via Serial2 and broadcasts via BLE
- Mobile app uses `MockBluetoothService` for development

### Future Implementation (Phase 5)
- Arduino reads **real** sensors using libraries:
  - DFRobot_OxygenSensor (DO sensor)
  - RTClib (DS3231 RTC)
  - HX711 (Load cell)
  - Adafruit_GFX (LCD display)
- Mobile app uses `RealBluetoothService` with `flutter_blue_plus` package
