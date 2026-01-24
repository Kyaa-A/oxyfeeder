/*
  ================================================================================
  OxyFeeder Firmware - FINAL VERSION (Real Sensors + LCD + Motor Control)
  ================================================================================

  Target: Arduino Mega 2560

  This is the COMPLETE firmware with all real sensor integrations.
  Currently using SIMULATED data - uncomment the real sensor code when hardware
  is connected.

  ================================================================================
  REQUIRED LIBRARIES (Install via Arduino IDE Library Manager):
  ================================================================================
  - HX711 Arduino Library (by Bogdan Necula)
  - RTClib (by Adafruit)
  - Adafruit GFX Library
  - Adafruit ILI9341 (for TFT LCD)
  - Servo (built-in)

  ================================================================================
  PIN ASSIGNMENTS:
  ================================================================================

  COMMUNICATION:
  - Serial1 TX (Pin 18) -> ESP32 RX (via level shifter)
  - Serial1 RX (Pin 19) -> ESP32 TX (via level shifter)

  SENSORS:
  - A0: Voltage Sensor (battery monitoring)
  - A1: DO Sensor (dissolved oxygen)
  - Pin 8: HX711 DT (load cell data)
  - Pin 9: HX711 SCK (load cell clock)
  - Pin 20 (SDA): RTC DS3231 + LCD (I2C)
  - Pin 21 (SCL): RTC DS3231 + LCD (I2C)

  ACTUATORS:
  - Pin 6: Servo (gate control)
  - Pin 10: Motor Driver IN1
  - Pin 11: Motor Driver IN2
  - Pin 12: Motor Driver ENA (PWM speed control)

  LCD (SPI):
  - Pin 38: DC
  - Pin 40: CS
  - Pin 42: RST
  - Pin 51: MOSI
  - Pin 52: SCK

  ================================================================================
*/

// ============================================================================
// LIBRARY INCLUDES
// ============================================================================

#include <SPI.h>
#include <Servo.h>

// Uncomment these when you have the libraries installed and hardware connected:
// #include <HX711.h>
// #include <RTClib.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_ILI9341.h>

// ============================================================================
// PIN DEFINITIONS
// ============================================================================

// Analog Sensors
#define PIN_VOLTAGE_SENSOR  A0    // Battery voltage divider
#define PIN_DO_SENSOR       A1    // Dissolved oxygen sensor

// HX711 Load Cell
#define PIN_HX711_DT        8     // Data pin
#define PIN_HX711_SCK       9     // Clock pin

// Motor Driver (L298N)
#define PIN_MOTOR_IN1       10    // Direction 1
#define PIN_MOTOR_IN2       11    // Direction 2
#define PIN_MOTOR_ENA       12    // Enable/PWM speed

// Servo
#define PIN_SERVO           6     // Servo signal

// LCD (ILI9341 SPI)
#define PIN_TFT_CS          40    // Chip select
#define PIN_TFT_DC          38    // Data/Command
#define PIN_TFT_RST         42    // Reset

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

// Voltage Sensor Calibration
// Adjust these based on your voltage divider resistors
const float VOLTAGE_DIVIDER_RATIO = 5.0;  // (R1+R2)/R2 ratio
const float VREF = 5.0;                    // Arduino reference voltage
const float BATTERY_MAX_VOLTAGE = 12.6;    // Fully charged 3S LiPo
const float BATTERY_MIN_VOLTAGE = 10.5;    // Minimum safe voltage

// DO Sensor Calibration
// These values need to be calibrated with known DO solutions
const float DO_CALIBRATION_OFFSET = 0.0;   // Offset adjustment
const float DO_CALIBRATION_SCALE = 1.0;    // Scale factor

// Load Cell Calibration
const float LOADCELL_CALIBRATION_FACTOR = 2280.0;  // Adjust after calibration
const float HOPPER_EMPTY_WEIGHT = 0.0;             // Tare weight (grams)
const float HOPPER_FULL_WEIGHT = 5000.0;           // Full hopper weight (grams)

// Motor Settings
const int MOTOR_SPEED = 200;          // PWM value 0-255
const int FEED_DURATION_MS = 5000;    // Default feeding duration

// Servo Settings
const int SERVO_CLOSED_ANGLE = 0;     // Gate closed position
const int SERVO_OPEN_ANGLE = 90;      // Gate open position

// Timing
const unsigned long SENSOR_READ_INTERVAL = 5000;   // Read sensors every 5 seconds
const unsigned long LCD_UPDATE_INTERVAL = 1000;    // Update LCD every 1 second
const unsigned long SERIAL_SEND_INTERVAL = 5000;   // Send to ESP32 every 5 seconds

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

Servo gateServo;

// Uncomment when hardware is connected:
// HX711 loadCell;
// RTC_DS3231 rtc;
// Adafruit_ILI9341 tft = Adafruit_ILI9341(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);

// ============================================================================
// GLOBAL VARIABLES - Current System State
// ============================================================================

float currentDO = 0.0;              // Dissolved Oxygen (mg/L)
int currentFeedLevel = 0;           // Feed level (%)
int currentBattery = 0;             // Battery level (%)
float currentVoltage = 0.0;         // Raw battery voltage

// Timing variables
unsigned long lastSensorRead = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastSerialSend = 0;

// Feeding schedule (simple version - can be expanded with RTC)
bool feedingScheduleEnabled = false;
int feedingHour = 8;    // Default: 8 AM
int feedingMinute = 0;

// ============================================================================
// SENSOR READING FUNCTIONS
// ============================================================================

/*
 * Read Battery Voltage
 * Uses a voltage divider to measure 12V battery with 5V Arduino
 *
 * WIRING:
 * Battery (+) --> R1 (e.g., 30K) --> A0 --> R2 (e.g., 7.5K) --> GND
 *
 * With R1=30K, R2=7.5K: ratio = (30+7.5)/7.5 = 5.0
 * 12V battery reads as 2.4V on A0
 */
float readBatteryVoltage_REAL() {
  int rawADC = analogRead(PIN_VOLTAGE_SENSOR);
  float voltage = (rawADC / 1023.0) * VREF * VOLTAGE_DIVIDER_RATIO;
  return voltage;
}

int voltageToPercent(float voltage) {
  // Map voltage to percentage (linear approximation)
  float percent = ((voltage - BATTERY_MIN_VOLTAGE) /
                   (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)) * 100.0;
  return constrain((int)percent, 0, 100);
}

/*
 * Read Dissolved Oxygen Sensor
 *
 * For DFRobot Gravity DO Sensor:
 * - Output: 0-3V corresponding to 0-20 mg/L
 * - Needs temperature compensation for accuracy
 *
 * WIRING:
 * DO Sensor Signal --> A1
 * DO Sensor VCC --> 5V
 * DO Sensor GND --> GND
 */
float readDO_REAL() {
  int rawADC = analogRead(PIN_DO_SENSOR);

  // Convert to voltage (0-5V range)
  float voltage = (rawADC / 1023.0) * VREF;

  // Convert voltage to DO (mg/L)
  // Typical: 0V = 0 mg/L, 3V = 20 mg/L (adjust based on your sensor)
  float doValue = (voltage / 3.0) * 20.0;

  // Apply calibration
  doValue = (doValue * DO_CALIBRATION_SCALE) + DO_CALIBRATION_OFFSET;

  return constrain(doValue, 0.0, 20.0);
}

/*
 * Read Feed Level from Load Cell
 *
 * HX711 WIRING:
 * HX711 DT  --> Pin 8
 * HX711 SCK --> Pin 9
 * HX711 VCC --> 5V
 * HX711 GND --> GND
 *
 * Load Cell (4 wires) to HX711:
 * Red (E+) --> E+
 * Black (E-) --> E-
 * White (A-) --> A-
 * Green (A+) --> A+
 */
float readFeedWeight_REAL() {
  // Uncomment when HX711 is connected:
  /*
  if (loadCell.is_ready()) {
    float weight = loadCell.get_units(5);  // Average of 5 readings
    return weight;
  }
  */
  return 0.0;
}

int weightToPercent(float weight) {
  float percent = ((weight - HOPPER_EMPTY_WEIGHT) /
                   (HOPPER_FULL_WEIGHT - HOPPER_EMPTY_WEIGHT)) * 100.0;
  return constrain((int)percent, 0, 100);
}

// ============================================================================
// SIMULATED SENSOR FUNCTIONS (Use these until real hardware is connected)
// ============================================================================

float readDO_SIMULATED() {
  static float value = 7.2;
  value += random(-10, 11) / 100.0;  // ±0.1 variation
  value = constrain(value, 5.0, 9.0);
  return value;
}

int readFeedLevel_SIMULATED() {
  static int value = 75;
  if (random(0, 100) < 3) {  // 3% chance to decrease
    value = constrain(value - 1, 0, 100);
  }
  return value;
}

int readBattery_SIMULATED() {
  static int value = 85;
  value += random(-1, 2);  // -1, 0, or +1
  value = constrain(value, 20, 100);
  return value;
}

// ============================================================================
// ACTUATOR CONTROL FUNCTIONS
// ============================================================================

/*
 * Control the Feeder Motor
 *
 * L298N WIRING:
 * IN1 --> Pin 10
 * IN2 --> Pin 11
 * ENA --> Pin 12
 * 12V --> Fuse Box (Motor Fuse)
 * GND --> Fuse Box GND
 *
 * Motor terminals --> OUT1, OUT2
 *
 * Direction:
 * IN1=HIGH, IN2=LOW  = Forward
 * IN1=LOW, IN2=HIGH  = Reverse
 * IN1=LOW, IN2=LOW   = Stop
 */
void motorForward(int speed) {
  digitalWrite(PIN_MOTOR_IN1, HIGH);
  digitalWrite(PIN_MOTOR_IN2, LOW);
  analogWrite(PIN_MOTOR_ENA, speed);
  Serial.println("MOTOR: Running forward");
}

void motorReverse(int speed) {
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, HIGH);
  analogWrite(PIN_MOTOR_ENA, speed);
  Serial.println("MOTOR: Running reverse");
}

void motorStop() {
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, LOW);
  analogWrite(PIN_MOTOR_ENA, 0);
  Serial.println("MOTOR: Stopped");
}

/*
 * Control the Gate Servo
 *
 * SERVO WIRING:
 * Signal (Orange/Yellow) --> Pin 6
 * VCC (Red) --> Buck Converter 5V output
 * GND (Brown/Black) --> Buck Converter GND
 *
 * IMPORTANT: Power servo from Buck Converter, NOT Arduino 5V pin!
 * Add 1000uF capacitor across Buck Converter output for stability.
 */
void openGate() {
  gateServo.write(SERVO_OPEN_ANGLE);
  Serial.println("GATE: Opened");
}

void closeGate() {
  gateServo.write(SERVO_CLOSED_ANGLE);
  Serial.println("GATE: Closed");
}

/*
 * Complete Feeding Sequence
 * 1. Open gate
 * 2. Run motor for specified duration
 * 3. Stop motor
 * 4. Close gate
 */
void dispenseFeed(int durationMs) {
  Serial.println("=== FEEDING SEQUENCE START ===");

  // Step 1: Open gate
  openGate();
  delay(500);  // Wait for gate to open

  // Step 2: Run motor
  motorForward(MOTOR_SPEED);
  delay(durationMs);

  // Step 3: Stop motor
  motorStop();
  delay(500);  // Wait for motor to stop

  // Step 4: Close gate
  closeGate();

  Serial.println("=== FEEDING SEQUENCE COMPLETE ===");
}

// ============================================================================
// LCD DISPLAY FUNCTIONS
// ============================================================================

/*
 * Initialize LCD Display
 *
 * LCD WIRING (4.0" ILI9341 480x320):
 * VCC  --> 5V
 * GND  --> GND
 * CS   --> Pin 40
 * RST  --> Pin 42
 * DC   --> Pin 38
 * MOSI --> Pin 51
 * SCK  --> Pin 52
 * LED  --> 3.3V (or 5V through resistor)
 */
void initLCD() {
  // Uncomment when LCD is connected:
  /*
  tft.begin();
  tft.setRotation(1);  // Landscape mode
  tft.fillScreen(ILI9341_BLACK);

  // Draw title
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(3);
  tft.setCursor(10, 10);
  tft.println("OxyFeeder");

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 50);
  tft.println("Initializing...");
  */

  Serial.println("LCD: Initialized (or simulated)");
}

void updateLCD() {
  // Uncomment when LCD is connected:
  /*
  // Clear data area (keep title)
  tft.fillRect(0, 80, 320, 160, ILI9341_BLACK);

  tft.setTextSize(2);

  // Dissolved Oxygen
  tft.setCursor(10, 90);
  tft.setTextColor(ILI9341_CYAN);
  tft.print("DO: ");
  tft.setTextColor(ILI9341_WHITE);
  tft.print(currentDO, 1);
  tft.println(" mg/L");

  // Feed Level
  tft.setCursor(10, 130);
  tft.setTextColor(ILI9341_YELLOW);
  tft.print("Feed: ");
  tft.setTextColor(ILI9341_WHITE);
  tft.print(currentFeedLevel);
  tft.println(" %");

  // Battery
  tft.setCursor(10, 170);
  if (currentBattery > 30) {
    tft.setTextColor(ILI9341_GREEN);
  } else {
    tft.setTextColor(ILI9341_RED);
  }
  tft.print("Battery: ");
  tft.setTextColor(ILI9341_WHITE);
  tft.print(currentBattery);
  tft.println(" %");

  // Status bar
  tft.setCursor(10, 220);
  tft.setTextColor(ILI9341_DARKGREY);
  tft.setTextSize(1);
  tft.print("Last update: ");
  tft.print(millis() / 1000);
  tft.println(" sec");
  */
}

// ============================================================================
// RTC FUNCTIONS
// ============================================================================

/*
 * Initialize RTC
 *
 * DS3231 WIRING (I2C):
 * SDA --> Pin 20
 * SCL --> Pin 21
 * VCC --> 5V
 * GND --> GND
 */
void initRTC() {
  // Uncomment when RTC is connected:
  /*
  if (!rtc.begin()) {
    Serial.println("RTC: Could not find DS3231!");
    return;
  }

  if (rtc.lostPower()) {
    Serial.println("RTC: Lost power, setting time to compile time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println("RTC: Initialized");
  */
}

void checkFeedingSchedule() {
  // Uncomment when RTC is connected:
  /*
  if (!feedingScheduleEnabled) return;

  DateTime now = rtc.now();

  // Check if it's feeding time
  if (now.hour() == feedingHour && now.minute() == feedingMinute) {
    static int lastFeedDay = -1;

    // Only feed once per day at scheduled time
    if (now.day() != lastFeedDay) {
      lastFeedDay = now.day();
      Serial.println("SCHEDULE: Feeding time!");
      dispenseFeed(FEED_DURATION_MS);
    }
  }
  */
}

// ============================================================================
// COMMUNICATION FUNCTIONS
// ============================================================================

/*
 * Send data to ESP32 via Serial1
 * JSON format: {"do": 7.2, "feed": 75, "battery": 85}
 */
void sendDataToESP32() {
  Serial1.print("{");
  Serial1.print("\"do\": ");
  Serial1.print(currentDO, 1);
  Serial1.print(", \"feed\": ");
  Serial1.print(currentFeedLevel);
  Serial1.print(", \"battery\": ");
  Serial1.print(currentBattery);
  Serial1.println("}");

  // Also print to USB Serial for debugging
  Serial.print("Sent to ESP32: {do: ");
  Serial.print(currentDO, 1);
  Serial.print(", feed: ");
  Serial.print(currentFeedLevel);
  Serial.print(", battery: ");
  Serial.print(currentBattery);
  Serial.println("}");
}

/*
 * Check for commands from ESP32
 * Future: Implement command parsing for remote feed triggers, etc.
 */
void checkIncomingCommands() {
  if (Serial1.available()) {
    String command = Serial1.readStringUntil('\n');
    command.trim();

    Serial.print("Received command: ");
    Serial.println(command);

    // Parse commands
    if (command == "FEED") {
      dispenseFeed(FEED_DURATION_MS);
    } else if (command == "GATE_OPEN") {
      openGate();
    } else if (command == "GATE_CLOSE") {
      closeGate();
    } else if (command == "STATUS") {
      sendDataToESP32();
    }
  }
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  // Initialize Serial communications
  Serial.begin(9600);   // USB Serial for debugging
  Serial1.begin(9600);  // Hardware Serial to ESP32

  while (!Serial) {
    ; // Wait for Serial to connect
  }

  Serial.println();
  Serial.println("================================================");
  Serial.println("    OxyFeeder Firmware - FINAL VERSION          ");
  Serial.println("================================================");
  Serial.println();

  // Initialize random seed for simulated data
  randomSeed(analogRead(A2));  // Use unused analog pin

  // Initialize Motor Driver pins
  pinMode(PIN_MOTOR_IN1, OUTPUT);
  pinMode(PIN_MOTOR_IN2, OUTPUT);
  pinMode(PIN_MOTOR_ENA, OUTPUT);
  motorStop();  // Ensure motor is off at startup
  Serial.println("Motor driver: Initialized");

  // Initialize Servo
  gateServo.attach(PIN_SERVO);
  closeGate();  // Ensure gate is closed at startup
  Serial.println("Servo: Initialized");

  // Initialize Load Cell (uncomment when connected)
  /*
  loadCell.begin(PIN_HX711_DT, PIN_HX711_SCK);
  loadCell.set_scale(LOADCELL_CALIBRATION_FACTOR);
  loadCell.tare();  // Reset to zero
  Serial.println("Load cell: Initialized");
  */

  // Initialize RTC (uncomment when connected)
  // initRTC();

  // Initialize LCD (uncomment when connected)
  // initLCD();

  Serial.println();
  Serial.println("Setup complete!");
  Serial.println("Using SIMULATED sensor data");
  Serial.println("Sending data to ESP32 every 5 seconds");
  Serial.println("================================================");
  Serial.println();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  unsigned long currentMillis = millis();

  // -------------------------------------------------------------------------
  // READ SENSORS (every SENSOR_READ_INTERVAL)
  // -------------------------------------------------------------------------
  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = currentMillis;

    // === USE SIMULATED DATA (current) ===
    currentDO = readDO_SIMULATED();
    currentFeedLevel = readFeedLevel_SIMULATED();
    currentBattery = readBattery_SIMULATED();

    // === USE REAL SENSORS (uncomment when hardware is connected) ===
    /*
    currentDO = readDO_REAL();
    currentVoltage = readBatteryVoltage_REAL();
    currentBattery = voltageToPercent(currentVoltage);
    float feedWeight = readFeedWeight_REAL();
    currentFeedLevel = weightToPercent(feedWeight);
    */
  }

  // -------------------------------------------------------------------------
  // UPDATE LCD (every LCD_UPDATE_INTERVAL)
  // -------------------------------------------------------------------------
  if (currentMillis - lastLCDUpdate >= LCD_UPDATE_INTERVAL) {
    lastLCDUpdate = currentMillis;
    updateLCD();
  }

  // -------------------------------------------------------------------------
  // SEND DATA TO ESP32 (every SERIAL_SEND_INTERVAL)
  // -------------------------------------------------------------------------
  if (currentMillis - lastSerialSend >= SERIAL_SEND_INTERVAL) {
    lastSerialSend = currentMillis;
    sendDataToESP32();
  }

  // -------------------------------------------------------------------------
  // CHECK FEEDING SCHEDULE (RTC-based)
  // -------------------------------------------------------------------------
  checkFeedingSchedule();

  // -------------------------------------------------------------------------
  // CHECK FOR INCOMING COMMANDS FROM ESP32
  // -------------------------------------------------------------------------
  checkIncomingCommands();

  // Small delay to prevent CPU hogging
  delay(10);
}

// ============================================================================
// CALIBRATION FUNCTIONS (Run these separately to calibrate sensors)
// ============================================================================

/*
 * To calibrate the load cell:
 * 1. Upload this code with CALIBRATION_MODE defined
 * 2. Open Serial Monitor
 * 3. Place a known weight on the scale
 * 4. Adjust LOADCELL_CALIBRATION_FACTOR until reading matches known weight
 *
 * Uncomment the function below and call it from setup() to calibrate:
 */

/*
void calibrateLoadCell() {
  Serial.println("=== LOAD CELL CALIBRATION MODE ===");
  Serial.println("Remove all weight from scale and wait...");
  delay(5000);

  loadCell.tare();
  Serial.println("Tare complete. Place known weight on scale.");

  while (true) {
    if (loadCell.is_ready()) {
      float reading = loadCell.get_units(10);
      Serial.print("Reading: ");
      Serial.print(reading);
      Serial.println(" grams");
      Serial.println("Adjust LOADCELL_CALIBRATION_FACTOR if incorrect");
    }
    delay(1000);
  }
}
*/

/*
 * To calibrate the DO sensor:
 * 1. Prepare a known DO solution (or use air-saturated water ~8.3 mg/L at 25°C)
 * 2. Record the analog reading
 * 3. Adjust DO_CALIBRATION_OFFSET and DO_CALIBRATION_SCALE
 */

/*
void calibrateDOSensor() {
  Serial.println("=== DO SENSOR CALIBRATION MODE ===");
  Serial.println("Place probe in calibration solution...");

  while (true) {
    int rawADC = analogRead(PIN_DO_SENSOR);
    float voltage = (rawADC / 1023.0) * VREF;
    float doValue = (voltage / 3.0) * 20.0;

    Serial.print("Raw ADC: ");
    Serial.print(rawADC);
    Serial.print(" | Voltage: ");
    Serial.print(voltage, 3);
    Serial.print("V | DO: ");
    Serial.print(doValue, 2);
    Serial.println(" mg/L");

    delay(1000);
  }
}
*/
