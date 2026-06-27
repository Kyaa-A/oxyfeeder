# OxyFeeder: Sensor and Module Reference Sheet

| Name | Function | Type | Voltage | Interface | Placement | Notes |
|------|-----------|------|----------|------------|-----------|-------|
| DO Sensor | Measures dissolved oxygen | Galvanic Probe | 3.3–5.5V | Analog | Submerged ~30cm | Requires calibration & clean membrane |
| Load Cell + HX711 | Feed weight measurement | Strain Gauge | 2.6–5.5V | Digital | Under feed hopper | Calibrate tare before operation |
| DS3231 RTC | Real-time scheduling | Clock Module | 3.3–5.5V | I2C | Main enclosure | Has CR2032 backup battery |
| Voltage Sensor | Battery monitoring | Resistive Divider | 3.3–5V | Analog | Across battery | Calibrate with multimeter |
| ESP32-CAM | Visual monitoring | Camera Module | 5V | UART/WiFi | Facing pond | Needs waterproof housing |
| pH Sensor (optional) | Water acidity monitoring | ISE Probe | 5V | Analog | Near DO probe | Future expansion |
| DS18B20 Temp Sensor (optional) | Water temperature | Digital | 3–5.5V | 1-Wire | Submerged | Helps correlate DO changes |
