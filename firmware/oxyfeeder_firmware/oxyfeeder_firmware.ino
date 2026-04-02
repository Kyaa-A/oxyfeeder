/*
  =============================================================================
  OxyFeeder Firmware - Phase 5: REAL HARDWARE INTEGRATION
  =============================================================================
  
  Target: Arduino Mega 2560
  Status: PRODUCTION FIRMWARE - Controls real sensors and actuators
  
  Hardware Configuration:
  -----------------------
  COMMUNICATION:
    - Serial1 (TX1=18, RX1=19) → ESP32 BLE Bridge (JSON output)
    - Serial3 (TX3=14, RX3=15) → SIM800L GSM Module (SMS alerts)
  
  SENSORS (Updated Feb 2026 - after 12V incident damage):
    - Dissolved Oxygen: DFRobot Analog on A1
    - Voltage Sensor: 0-25V Module on A2 (A0 DAMAGED)
    - Ultrasonic: HC-SR04 on TRIG=13, ECHO=48 (replaced HX711 load cell)
    - RTC: DS3231 on I2C (SDA=20, SCL=21) - MODULE DAMAGED, awaiting replacement

  ACTUATORS:
    - DC Motor (Spinner): L298N on IN1=4, IN2=5, ENA=12
    - Servo (Gate): Pin 6
    - Buzzer: Pin 7
  
  DISPLAY:
    - TFT LCD: ST7796S 4.0" 480x320 via SPI (CS=40, DC=38, RST=3.3V)
  
  =============================================================================
*/

// ============================================================================
// LIBRARIES
// ============================================================================

#include <Wire.h>
#include <SPI.h>
#include <Servo.h>
#include <RTClib.h>
// #include <HX711.h>  // LOAD CELL - uncomment if switching back to load cell

#include <Arduino_GFX_Library.h>
#include <EEPROM.h>

// ============================================================================
// ALPHA TESTING MODE - Disable LCD to prevent white screen issues
// ============================================================================
#define ALPHA_MODE false  // Set to false when LCD is working

// Color Definitions (RGB565 format)
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define CYAN    0x07FF
#define YELLOW  0xFFE0
#define ORANGE  0xFD20
#define NAVY    0x000F
#define DARKGREY 0x7BEF

// ============================================================================
// PIN DEFINITIONS
// ============================================================================

// Sensors (UPDATED after bench testing - Feb 2026)
#define DO_SENSOR_PIN       A1    // Dissolved Oxygen (Analog)
#define VOLTAGE_SENSOR_PIN  A2    // Battery Voltage (A0 DAMAGED - moved to A2)
// ULTRASONIC (current)
#define ULTRASONIC_TRIG_PIN 13    // HC-SR04 Trigger
#define ULTRASONIC_ECHO_PIN 48    // HC-SR04 Echo

// LOAD CELL (commented out - uncomment if switching back)
// #define HX711_DT_PIN        10    // Load Cell Data (Pin 8 DAMAGED - moved to 10)
// #define HX711_SCK_PIN       11    // Load Cell Clock (Pin 9 DAMAGED - moved to 11)

// Actuators
#define MOTOR_IN1           4     // L298N Input 1
#define MOTOR_IN2           5     // L298N Input 2
#define MOTOR_ENA           12    // L298N Enable A (PWM)
#define SERVO_PIN           6     // Servo PWM
#define BUZZER_PIN          7     // Buzzer

// Display (SPI)
#define TFT_CS              40    // Chip Select
#define TFT_DC              38    // Data/Command
#define TFT_RST             -1    // Not using reset pin (RST tied to 3.3V)

// ============================================================================
// CONFIGURATION - ADJUST THESE VALUES
// ============================================================================

// Feeding Schedule - Dynamic (can be updated from app)
#define MAX_SCHEDULES 10
struct FeedingSchedule {
  int hour;       // 24-hour format
  int minute;
  int duration;   // seconds
  bool enabled;
};

FeedingSchedule schedules[MAX_SCHEDULES];
int scheduleCount = 0;

// Default feeding schedule (used if no schedules from app)
const int DEFAULT_FEED_HOUR_1 = 8;    // First feeding at 08:00
const int DEFAULT_FEED_MIN_1 = 0;
const int DEFAULT_FEED_HOUR_2 = 17;   // Second feeding at 17:00
const int DEFAULT_FEED_MIN_2 = 0;

// Feeding duration
const int FEED_DURATION_SECONDS = 5;

// Servo positions
const int SERVO_OPEN_ANGLE = 30;
const int SERVO_CLOSED_ANGLE = 0;

// ULTRASONIC calibration - ADJUST THESE to match your hopper
const int HOPPER_EMPTY_CM = 30;  // Distance (cm) when hopper is empty
const int HOPPER_FULL_CM  = 5;   // Distance (cm) when hopper is full

// LOAD CELL calibration (commented out - uncomment if switching back)
// float calibration_factor = -7050.0;
// const float EMPTY_HOPPER_KG = 0.0;
// const float FULL_HOPPER_KG = 5.0;

// Dissolved Oxygen calibration
// DFRobot DO sensor: V = 0-3V corresponds to 0-20 mg/L
const float DO_VOLTAGE_REF = 5.0;
const float DO_MAX_VALUE = 20.0;     // Max mg/L at max voltage

// Safety thresholds
const float DO_CRITICAL_THRESHOLD = 4.0;  // mg/L - trigger alarm below this
const int LOW_FEED_THRESHOLD = 10;        // % - trigger warning below this
const int LOW_BATTERY_THRESHOLD = 10;     // % - trigger warning below this

// Voltage sensor calibration
// 0-25V module with voltage divider ratio of 5:1
const float VOLTAGE_RATIO = 4.57;  // Calibrated: actual/measured = 11.7/12.8
const float BATTERY_FULL_VOLTAGE = 14.4;  // 12V battery fully charged
const float BATTERY_EMPTY_VOLTAGE = 11.0; // 12V battery empty

// Update intervals
const unsigned long SENSOR_UPDATE_INTERVAL = 2000;   // 2 seconds
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000;  // 1 second
const unsigned long JSON_SEND_INTERVAL = 2000;       // 2 seconds
const unsigned long HEARTBEAT_INTERVAL = 10000;      // 10 seconds - ALIVE indicator

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

RTC_DS3231 rtc;
// HX711 scale;  // LOAD CELL - uncomment if switching back
Servo feedGate;

// TFT Display - ST7796S 4.0" 480x320
// Hardware SPI: MOSI=51, SCK=52 (fixed on Mega)
#if !ALPHA_MODE
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);
Arduino_GFX *tft = new Arduino_ST7796(bus, TFT_RST, 1 /* rotation */, true /* IPS */);
#endif

// ============================================================================
// GLOBAL VARIABLES - Current Sensor Readings
// ============================================================================

float currentDissolvedOxygen = 0.0;   // mg/L
int currentFeedLevel = 0;              // percentage 0-100
int currentBatteryPercent = 0;         // percentage 0-100
float currentBatteryVoltage = 0.0;     // volts

DateTime currentTime;
bool rtcAvailable = false;

// Timing variables
unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastJsonSend = 0;
unsigned long lastHeartbeat = 0;
unsigned long lastWarningPrint = 0;
unsigned long lastBuzzerAlert = 0;
const unsigned long BUZZER_ALERT_INTERVAL = 30000;  // Buzzer every 30 seconds during alert
bool lastFeedingDone = false;  // Prevents repeated feeding in same minute

// SMS cooldown to prevent spam
unsigned long lastSmsSent = 0;
const unsigned long SMS_COOLDOWN = 10000;  // 10 seconds (testing mode)

// SMS Phone Number (stored in EEPROM)
#define EEPROM_PHONE_ADDR 0       // EEPROM starting address for phone number
#define PHONE_NUMBER_LENGTH 15    // Max length of phone number
char smsPhoneNumber[PHONE_NUMBER_LENGTH + 1] = "09550717546";  // Default number (local format)

// Command buffer for receiving from ESP32
String commandBuffer = "";
const int MAX_COMMAND_LENGTH = 50;

// Flag: once app syncs schedules, don't use defaults anymore
bool appSchedulesSynced = false;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

void initSensors();
void initActuators();
void initDisplay();
void initGSM();

float readDissolvedOxygen();
float readBatteryVoltage();
int readBatteryPercent();
long readUltrasonicCM();
int readFeedLevel();

void dispenseFeed(int seconds);
void openGate();
void closeGate();
void triggerAlarm();
void stopAlarm();

void drawStaticUI();
void updateDisplay();
void sendJsonToESP32();
void sendSMS(const char* message);

void checkFeedingSchedule();
void runFeedingSequence();
void checkSafetyAlerts();

// Phone number and command handling
void loadPhoneNumberFromEEPROM();
void savePhoneNumberToEEPROM();
void processIncomingCommands();
void handleCommand(String command);

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  // Initialize Serial ports
  Serial.begin(9600);       // USB Debug
  Serial1.begin(9600);      // ESP32 Data Output (TX1=Pin18 → ESP32 RX)
  pinMode(17, INPUT); // Feed command from ESP32 GPIO2 (external 10K pull-down installed)
  pinMode(3, INPUT);  // SMS command from ESP32 GPIO13 (external 10K pull-down installed)
  Serial3.begin(9600);      // GSM Module (SIM800L)
  
  while (!Serial) {
    ; // Wait for USB Serial (needed for some boards)
  }
  
  Serial.println(F(""));
  Serial.println(F("=============================================="));
  Serial.println(F("  OxyFeeder Firmware v2.0 - PRODUCTION BUILD  "));
  Serial.println(F("=============================================="));
  Serial.println(F(""));

  // DEBUG: Send test string on Serial1 to verify Pin 18 TX works
  Serial1.println(F("{\"test\":\"serial1_alive\"}"));
  Serial.println(F("[DEBUG] Sent test JSON on Serial1 (Pin 18)"));
  
  // Initialize all subsystems
  loadPhoneNumberFromEEPROM();  // Load saved phone number
  initActuators();
  initSensors();
  initDisplay();
  initGSM();
  
  // Initial sensor read
  delay(1000);
  currentDissolvedOxygen = readDissolvedOxygen();
  currentBatteryVoltage = readBatteryVoltage();
  currentBatteryPercent = readBatteryPercent();
  currentFeedLevel = readFeedLevel();
  
  Serial.println(F(""));
  Serial.println(F("=============================================="));
  Serial.println(F("        SYSTEM READY - Entering Main Loop     "));
  Serial.println(F("=============================================="));
  Serial.println(F(""));
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  unsigned long now = millis();
  
  // Update RTC time
  if (rtcAvailable) {
    currentTime = rtc.now();
  }
  
  // 1. READ SENSORS (every 2 seconds)
  if (now - lastSensorRead >= SENSOR_UPDATE_INTERVAL) {
    lastSensorRead = now;
    
    currentDissolvedOxygen = readDissolvedOxygen();
    currentBatteryVoltage = readBatteryVoltage();
    currentBatteryPercent = readBatteryPercent();
    currentWeight = readWeight();
    currentFeedLevel = readFeedLevel();
    
    // Debug output
    Serial.print(F("DO: "));
    Serial.print(currentDissolvedOxygen, 1);
    Serial.print(F(" mg/L | Feed: "));
    Serial.print(currentFeedLevel);
    Serial.print(F("% | Battery: "));
    Serial.print(currentBatteryPercent);
    Serial.print(F("% ("));
    Serial.print(currentBatteryVoltage, 1);
    Serial.println(F("V)"));
  }
  
  // 2. UPDATE DISPLAY (every 1 second) - Only if LCD enabled
#if !ALPHA_MODE
  if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdate = now;
    updateDisplay();
  }
#endif
  
  // 3. SEND JSON TO ESP32 (every 2 seconds)
  if (now - lastJsonSend >= JSON_SEND_INTERVAL) {
    lastJsonSend = now;
    sendJsonToESP32();
  }
  
  // 4. CHECK FEEDING SCHEDULE
  checkFeedingSchedule();

  // 5. CHECK SAFETY ALERTS
  checkSafetyAlerts();

  // 6. PROCESS INCOMING COMMANDS FROM APP
  processIncomingCommands();

  // 7. HEARTBEAT - ALIVE INDICATOR (every 10 seconds)
  if (now - lastHeartbeat >= HEARTBEAT_INTERVAL) {
    lastHeartbeat = now;
    Serial.println(F("[HEARTBEAT] ALIVE - System running OK"));

    // Quick beep to confirm system is alive (comment out if annoying)
    // tone(BUZZER_PIN, 1000, 50);  // 50ms beep at 1kHz
  }

  // Small delay to prevent overwhelming the system
  delay(10);
}

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

void initSensors() {
  Serial.println(F("[INIT] Initializing sensors..."));
  
  // Initialize I2C for RTC
  Wire.begin();
  
  // Initialize RTC (DS3231)
  Serial.print(F("  - RTC DS3231: "));
  if (rtc.begin()) {
    rtcAvailable = true;
    
    // Only set RTC if it lost power (battery died)
    if (rtc.lostPower()) {
      Serial.println(F("RTC lost power, setting to compile time!"));
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    
    currentTime = rtc.now();
    Serial.print(F("OK - Time: "));
    Serial.print(currentTime.hour());
    Serial.print(F(":"));
    if (currentTime.minute() < 10) Serial.print(F("0"));
    Serial.print(currentTime.minute());
    Serial.print(F(":"));
    if (currentTime.second() < 10) Serial.print(F("0"));
    Serial.println(currentTime.second());
  } else {
    rtcAvailable = false;
    Serial.println(F("FAILED! Check wiring."));
  }
  
  // Initialize Ultrasonic Sensor (HC-SR04)
  Serial.print(F("  - Ultrasonic HC-SR04: "));
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delay(50);
  long testDist = readUltrasonicCM();
  if (testDist > 0 && testDist < 400) {
    Serial.print(F("OK - Distance: "));
    Serial.print(testDist);
    Serial.println(F(" cm"));
  } else {
    Serial.println(F("WARNING - Check TRIG=13, ECHO=48"));
  }

  // Dummy block to keep structure (replaces old HX711 loop)
  if (false) {
    Serial.print(F("OK - Tared (attempt "));
    Serial.print(1);
      Serial.println(F(")"));
      break;
    }
  }
  }
  
  // Analog pins (no special init needed)
  Serial.println(F("  - DO Sensor (A1): OK"));
  Serial.println(F("  - Voltage Sensor (A2): OK"));
}

void initActuators() {
  Serial.println(F("[INIT] Initializing actuators..."));
  
  // Motor driver pins
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_ENA, OUTPUT);
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_ENA, LOW);  // Fully disable motor driver
  Serial.println(F("  - DC Motor (L298N): OK"));
  
  // Servo
  feedGate.attach(SERVO_PIN);
  closeGate();  // Start with gate closed
  Serial.println(F("  - Servo Gate: OK (Closed)"));
  
  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println(F("  - Buzzer: OK"));
}

void initDisplay() {
#if ALPHA_MODE
  // Alpha Mode: LCD disabled, use buzzer feedback instead
  Serial.println(F("[INIT] TFT display DISABLED (Alpha Mode)"));
  Serial.println(F("  - Using buzzer for feedback"));

  // Startup beep: 2 short beeps = system starting
  tone(BUZZER_PIN, 1000, 150);
  delay(200);
  tone(BUZZER_PIN, 1500, 150);
  delay(200);
  Serial.println(F("  - Startup beep: OK"));
#else
  Serial.println(F("[INIT] Initializing TFT display..."));

  if (!tft->begin()) {
    Serial.println(F("  - TFT ST7796S: FAILED!"));
    return;
  }

  tft->fillScreen(BLACK);

  // Draw startup screen
  tft->setTextColor(CYAN);
  tft->setTextSize(4);
  tft->setCursor(120, 80);
  tft->println(F("OxyFeeder"));

  tft->setTextSize(2);
  tft->setTextColor(WHITE);
  tft->setCursor(140, 140);
  tft->println(F("Initializing..."));

  tft->setTextSize(1);
  tft->setTextColor(GREEN);
  tft->setCursor(160, 200);
  tft->println(F("Production Build v2.0"));

  delay(2000);
  tft->fillScreen(BLACK);

  // Draw static UI elements
  drawStaticUI();

  Serial.println(F("  - TFT ST7796S: OK (480x320)"));
#endif
}

void initGSM() {
  Serial.println(F("[INIT] Initializing GSM module..."));
  
  // Wait for GSM module to boot
  delay(1000);
  
  // Send AT command to check if module is responding
  Serial3.println(F("AT"));
  delay(500);
  
  // Set SMS to text mode
  Serial3.println(F("AT+CMGF=1"));
  delay(500);
  
  Serial.println(F("  - SIM800L GSM: OK (Text Mode)"));
}

// ============================================================================
// SENSOR READING FUNCTIONS
// ============================================================================

float readDissolvedOxygen() {
  // Read analog value from DFRobot DO sensor
  int rawValue = analogRead(DO_SENSOR_PIN);
  
  // Convert to voltage
  float voltage = (rawValue / 1023.0) * DO_VOLTAGE_REF;
  
  // Convert voltage to DO value (mg/L)
  // DFRobot sensor typically outputs 0-3V for 0-20 mg/L
  // Adjust this calibration based on your specific sensor
  float doValue = (voltage / 3.0) * DO_MAX_VALUE;
  
  // Clamp to valid range
  doValue = constrain(doValue, 0.0, 20.0);
  
  return doValue;
}

float readBatteryVoltage() {
  // Read analog value from voltage sensor
  int rawValue = analogRead(VOLTAGE_SENSOR_PIN);
  
  // Convert to voltage at the Arduino pin
  float pinVoltage = (rawValue / 1023.0) * 5.0;
  
  // Apply voltage divider ratio to get actual battery voltage
  float batteryVoltage = pinVoltage * VOLTAGE_RATIO;
  
  return batteryVoltage;
}

int readBatteryPercent() {
  float voltage = currentBatteryVoltage;
  
  // Map voltage to percentage
  // 11.0V = 0%, 14.4V = 100%
  int percent = map(voltage * 100, BATTERY_EMPTY_VOLTAGE * 100, 
                    BATTERY_FULL_VOLTAGE * 100, 0, 100);
  
  // Clamp to 0-100%
  percent = constrain(percent, 0, 100);
  
  return percent;
}

// ---- ULTRASONIC (current) ----
long readUltrasonicCM() {
  // Send 10us pulse to TRIG
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  // Read echo duration (timeout 30ms = ~500cm max)
  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;  // Timeout = no reading

  // Convert to cm: speed of sound = 343m/s
  return duration / 58;
}

int readFeedLevel() {
  // FULL hopper = short distance (HOPPER_FULL_CM)
  // EMPTY hopper = long distance (HOPPER_EMPTY_CM)
  long distanceCM = readUltrasonicCM();

  if (distanceCM <= 0) return currentFeedLevel;  // Keep last reading on error

  int percent = map(distanceCM, HOPPER_EMPTY_CM, HOPPER_FULL_CM, 0, 100);
  return constrain(percent, 0, 100);
}

// ---- LOAD CELL (commented out - uncomment if switching back) ----
// float readWeight() {
//   if (!scaleAvailable) return 0.0;
//   if (scale.is_ready()) {
//     float weight = scale.get_units(5);
//     if (weight < 0) weight = 0;
//     return weight;
//   }
//   return 0.0;
// }
//
// int readFeedLevel() {  // LOAD CELL version
//   float weight = currentWeight;
//   int percent = map(weight * 100, EMPTY_HOPPER_KG * 100, FULL_HOPPER_KG * 100, 0, 100);
//   return constrain(percent, 0, 100);
// }

// ============================================================================
// ACTUATOR CONTROL FUNCTIONS
// ============================================================================

void dispenseFeed(int seconds) {
  static bool isFeeding = false;
  if (isFeeding) {
    Serial.println(F("[ACTUATOR] Already feeding, ignoring request"));
    return;
  }
  isFeeding = true;

  Serial.print(F("[ACTUATOR] Starting feeding sequence for "));
  Serial.print(seconds);
  Serial.println(F(" seconds..."));

  // BLOCKING feeding sequence (servo needs uninterrupted PWM)
  // Step 1: Open servo briefly (100ms) then close
  Serial.println(F("[FEEDING] Gate opening..."));
  feedGate.attach(SERVO_PIN);
  feedGate.write(SERVO_OPEN_ANGLE);
  delay(150);  // Open for 150ms only
  feedGate.write(SERVO_CLOSED_ANGLE);
  delay(500);  // Wait for servo to fully close

  // Step 2: Spin DC motor for the set duration
  Serial.println(F("[FEEDING] Dispensing feed..."));
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_ENA, 255);
  delay((unsigned long)seconds * 1000UL);

  // Stop motor - fully disable
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_ENA, LOW);

  Serial.println(F("[FEEDING] Complete!"));
  triggerAlarm();  // Beep to confirm
  isFeeding = false;
}

void openGate() {
  Serial.println(F("[ACTUATOR] Opening gate..."));
  feedGate.write(SERVO_OPEN_ANGLE);
  delay(500);  // Wait for servo to reach position
}

void closeGate() {
  Serial.println(F("[ACTUATOR] Closing gate..."));
  feedGate.write(SERVO_CLOSED_ANGLE);
  delay(500);  // Wait for servo to reach position
}

void triggerAlarm() {
  Serial.println(F("[ALARM] Buzzer ON"));
  
  // Beep pattern: 3 short beeps
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}

void stopAlarm() {
  digitalWrite(BUZZER_PIN, LOW);
}

// ============================================================================
// DISPLAY FUNCTIONS - ST7796S 480x320
// ============================================================================

#if !ALPHA_MODE
void drawStaticUI() {
  // Header bar
  tft->fillRect(0, 0, 480, 50, NAVY);
  tft->setTextColor(WHITE);
  tft->setTextSize(3);
  tft->setCursor(120, 12);
  tft->print(F("OXYFEEDER SYSTEM"));

  // Separator line
  tft->drawFastHLine(0, 50, 480, WHITE);

  // Sensor box borders
  tft->drawRect(10, 100, 145, 90, CYAN);    // DO box
  tft->drawRect(165, 100, 145, 90, YELLOW); // Feed box
  tft->drawRect(320, 100, 150, 90, GREEN);  // Battery box

  // Sensor labels
  tft->setTextSize(2);
  tft->setTextColor(CYAN);
  tft->setCursor(20, 105);
  tft->print(F("DO Level"));

  tft->setTextColor(YELLOW);
  tft->setCursor(175, 105);
  tft->print(F("Feed Level"));

  tft->setTextColor(GREEN);
  tft->setCursor(335, 105);
  tft->print(F("Battery"));

  // Status bar background
  tft->fillRect(0, 280, 480, 40, DARKGREY);
}

void updateDisplay() {
  // Only update value areas (prevents flicker)

  // Time display area (below header)
  tft->fillRect(150, 55, 180, 35, BLACK);
  tft->setTextColor(CYAN);
  tft->setTextSize(3);
  tft->setCursor(155, 60);

  if (rtcAvailable) {
    if (currentTime.hour() < 10) tft->print(F("0"));
    tft->print(currentTime.hour());
    tft->print(F(":"));
    if (currentTime.minute() < 10) tft->print(F("0"));
    tft->print(currentTime.minute());
    tft->print(F(":"));
    if (currentTime.second() < 10) tft->print(F("0"));
    tft->print(currentTime.second());
  } else {
    tft->print(F("--:--:--"));
  }

  // DO Value (inside box)
  tft->fillRect(15, 130, 135, 55, BLACK);
  tft->setTextSize(3);
  tft->setCursor(25, 140);
  if (currentDissolvedOxygen < DO_CRITICAL_THRESHOLD) {
    tft->setTextColor(RED);
  } else {
    tft->setTextColor(GREEN);
  }
  tft->print(currentDissolvedOxygen, 1);
  tft->setTextSize(2);
  tft->setCursor(25, 168);
  tft->print(F("mg/L"));

  // Feed Level Value (inside box)
  tft->fillRect(170, 130, 135, 55, BLACK);
  tft->setTextSize(3);
  tft->setCursor(185, 140);
  if (currentFeedLevel < LOW_FEED_THRESHOLD) {
    tft->setTextColor(ORANGE);
  } else {
    tft->setTextColor(GREEN);
  }
  tft->print(currentFeedLevel);
  tft->print(F("%"));

  // Battery Value (inside box)
  tft->fillRect(325, 130, 140, 55, BLACK);
  tft->setTextSize(3);
  tft->setCursor(335, 140);
  if (currentBatteryPercent < LOW_BATTERY_THRESHOLD) {
    tft->setTextColor(RED);
  } else {
    tft->setTextColor(GREEN);
  }
  tft->print(currentBatteryPercent);
  tft->print(F("%"));
  tft->setTextSize(2);
  tft->setCursor(335, 168);
  tft->print(currentBatteryVoltage, 1);
  tft->print(F("V"));

  // Status section
  tft->fillRect(10, 200, 220, 70, BLACK);
  tft->setTextSize(2);
  tft->setTextColor(YELLOW);
  tft->setCursor(15, 205);
  tft->print(F("System Status:"));
  tft->setTextSize(2);
  tft->setCursor(15, 230);
  tft->setTextColor(GREEN);
  tft->print(F("RUNNING"));

  // BLE Status
  tft->fillRect(250, 200, 220, 70, BLACK);
  tft->setTextSize(2);
  tft->setTextColor(YELLOW);
  tft->setCursor(255, 205);
  tft->print(F("Communication:"));
  tft->setCursor(255, 230);
  tft->setTextColor(CYAN);
  tft->print(F("BLE ACTIVE"));

  // Footer - Next feeding time
  tft->fillRect(10, 285, 460, 30, DARKGREY);
  tft->setTextSize(2);
  tft->setTextColor(WHITE);
  tft->setCursor(15, 292);
  tft->print(F("Schedule: Managed by App"));
}
#endif  // !ALPHA_MODE

// ============================================================================
// COMMUNICATION FUNCTIONS
// ============================================================================

void sendJsonToESP32() {
  // Build JSON string and send on BOTH Serial (Pin 1) and Serial1 (Pin 18)
  // Pin 1 goes direct wire to ESP32 D26 (logic shifter on Pin 18 failed)
  // ESP32 filters for lines starting with '{' to ignore debug text

  // Send on Serial (Pin 1) - this is the working path to ESP32
  Serial.print(F("{\"do\": "));
  Serial.print(currentDissolvedOxygen, 1);
  Serial.print(F(", \"feed\": "));
  Serial.print(currentFeedLevel);
  Serial.print(F(", \"battery\": "));
  Serial.print(currentBatteryPercent);
  Serial.println(F("}"));

  // Also send on Serial1 (Pin 18) in case it works later
  Serial1.print(F("{\"do\": "));
  Serial1.print(currentDissolvedOxygen, 1);
  Serial1.print(F(", \"feed\": "));
  Serial1.print(currentFeedLevel);
  Serial1.print(F(", \"battery\": "));
  Serial1.print(currentBatteryPercent);
  Serial1.println(F("}"));
}

void sendSMS(const char* message) {
  unsigned long now = millis();
  
  // Check cooldown to prevent SMS spam
  if (now - lastSmsSent < SMS_COOLDOWN && lastSmsSent != 0) {
    Serial.println(F("[GSM] SMS cooldown active, skipping..."));
    return;
  }
  
  Serial.print(F("[GSM] Sending SMS to: "));
  Serial.println(smsPhoneNumber);
  Serial.print(F("[GSM] Message: "));
  Serial.println(message);
  
  // Clear any leftover data in SIM800L buffer
  while (Serial3.available()) Serial3.read();

  // Check SIM PIN status
  Serial3.println(F("AT+CPIN?"));
  delay(1000);
  String pinResponse = "";
  while (Serial3.available()) {
    char c = Serial3.read();
    pinResponse += c;
  }
  Serial.print(F("[GSM] PIN status: "));
  Serial.println(pinResponse);
  if (pinResponse.indexOf("READY") == -1) {
    Serial.println(F("[GSM] ERROR: SIM not ready (PIN locked or not inserted)"));
    return;
  }

  // Check network registration
  Serial3.println(F("AT+CREG?"));
  delay(1000);
  String regResponse = "";
  while (Serial3.available()) {
    char c = Serial3.read();
    regResponse += c;
  }
  Serial.print(F("[GSM] Network registration: "));
  Serial.println(regResponse);
  // +CREG: 0,1 = registered home, +CREG: 0,5 = roaming
  if (regResponse.indexOf(",1") == -1 && regResponse.indexOf(",5") == -1) {
    Serial.println(F("[GSM] ERROR: Not registered to network"));
    return;
  }

  // Set SMS to text mode
  Serial3.println(F("AT+CMGF=1"));
  delay(500);
  while (Serial3.available()) Serial3.read();  // flush response

  // Set recipient phone number
  Serial3.print(F("AT+CMGS=\""));
  Serial3.print(smsPhoneNumber);
  Serial3.println(F("\""));

  // Wait for '>' prompt from SIM800L
  unsigned long promptStart = millis();
  bool gotPrompt = false;
  while (millis() - promptStart < 5000) {
    if (Serial3.available()) {
      char c = Serial3.read();
      Serial.write(c);  // print raw response to USB for debugging
      if (c == '>') {
        gotPrompt = true;
        break;
      }
    }
  }

  if (!gotPrompt) {
    Serial.println(F("[GSM] ERROR: No '>' prompt from SIM800L"));
    return;
  }

  // Send message text
  Serial3.print(message);
  delay(100);
  Serial3.write(26);  // Ctrl+Z to send

  // Wait for +CMGS: response (up to 10 seconds)
  unsigned long sendStart = millis();
  String response = "";
  bool smsSent = false;
  while (millis() - sendStart < 10000) {
    if (Serial3.available()) {
      char c = Serial3.read();
      Serial.write(c);  // print raw response
      response += c;
      if (response.indexOf("+CMGS:") >= 0) {
        smsSent = true;
        break;
      }
      if (response.indexOf("ERROR") >= 0) {
        break;
      }
    }
  }

  if (smsSent) {
    lastSmsSent = now;
    Serial.println(F("[GSM] SMS sent!"));
  } else {
    Serial.println(F("[GSM] SMS FAILED! Check SIM/signal."));
    Serial.print(F("[GSM] Response: "));
    Serial.println(response);
  }
}

// ============================================================================
// SCHEDULING & SAFETY FUNCTIONS
// ============================================================================

void checkFeedingSchedule() {
  if (!rtcAvailable) return;
  
  int hour = currentTime.hour();
  int minute = currentTime.minute();
  int second = currentTime.second();
  
  bool shouldFeed = false;
  
  // All scheduling is handled by ESP32 (synced from app via BLE)
  // ESP32 pulses GPIO pin 17 at scheduled times
  // Arduino just responds to the GPIO pulse in processIncomingCommands()
  
  if (shouldFeed && !lastFeedingDone) {
    Serial.println(F(""));
    Serial.println(F("========================================"));
    Serial.println(F("    SCHEDULED FEEDING TIME!"));
    Serial.println(F("========================================"));
    
    runFeedingSequence();
    lastFeedingDone = true;
  }
  
  // Reset the flag after the feeding minute passes
  if (second >= 30) {
    lastFeedingDone = false;
  }
}

void runFeedingSequence() {
  dispenseFeed(FEED_DURATION_SECONDS);
}

void checkSafetyAlerts() {
  // NOTE: SMS alerts are now handled by ESP32 (synced thresholds from app).
  // Arduino only handles buzzer here as a safety fallback with hardcoded thresholds.
  unsigned long nowAlert = millis();
  bool anyAlert = false;

  // Check for critical low dissolved oxygen
  if (currentDissolvedOxygen < DO_CRITICAL_THRESHOLD && currentDissolvedOxygen > 0) {
    anyAlert = true;
    if (nowAlert - lastWarningPrint >= 10000) {
      Serial.println(F("[ALERT] CRITICAL: Low Dissolved Oxygen!"));
    }
  }

  // Check for low feed level
  if (currentFeedLevel < LOW_FEED_THRESHOLD && currentFeedLevel >= 0) {
    anyAlert = true;
    if (nowAlert - lastWarningPrint >= 10000) {
      Serial.println(F("[WARNING] Low feed level detected"));
    }
  }

  // Check for low battery
  if (currentBatteryPercent < LOW_BATTERY_THRESHOLD && currentBatteryPercent > 0) {
    anyAlert = true;
    if (nowAlert - lastWarningPrint >= 10000) {
      Serial.println(F("[WARNING] Low battery detected"));
    }
  }

  // Update warning print timer
  if (nowAlert - lastWarningPrint >= 10000) {
    lastWarningPrint = nowAlert;
  }

  // Buzzer alert: beep every 30 seconds while any alert is active
  if (anyAlert && (nowAlert - lastBuzzerAlert >= BUZZER_ALERT_INTERVAL)) {
    lastBuzzerAlert = nowAlert;
    triggerAlarm();
  }
}
// ============================================================================
// EEPROM & COMMAND HANDLING FUNCTIONS
// ============================================================================

void loadPhoneNumberFromEEPROM() {
  Serial.print(F("[EEPROM] Loading phone number... "));
  
  // Check if EEPROM has valid data (first byte should be '+')
  char firstChar = EEPROM.read(EEPROM_PHONE_ADDR);
  
  if (firstChar == '+') {
    // Read phone number from EEPROM
    for (int i = 0; i < PHONE_NUMBER_LENGTH; i++) {
      char c = EEPROM.read(EEPROM_PHONE_ADDR + i);
      if (c == '\0' || c == 255) {
        smsPhoneNumber[i] = '\0';
        break;
      }
      smsPhoneNumber[i] = c;
    }
    smsPhoneNumber[PHONE_NUMBER_LENGTH] = '\0';  // Ensure null termination
    Serial.println(smsPhoneNumber);
  } else {
    // No valid data, use default
    Serial.println(F("No saved number, using default"));
    Serial.print(F("[EEPROM] Default: "));
    Serial.println(smsPhoneNumber);
  }
}

void savePhoneNumberToEEPROM() {
  Serial.print(F("[EEPROM] Saving phone number: "));
  Serial.println(smsPhoneNumber);
  
  // Write phone number to EEPROM
  for (int i = 0; i < PHONE_NUMBER_LENGTH; i++) {
    EEPROM.write(EEPROM_PHONE_ADDR + i, smsPhoneNumber[i]);
    if (smsPhoneNumber[i] == '\0') break;
  }
  
  Serial.println(F("[EEPROM] Phone number saved!"));
}

void processIncomingCommands() {
  unsigned long now = millis();
  // --- GPIO commands from ESP32 ---
  // Skip GPIO checks for first 15 seconds after boot (avoid startup noise)
  static unsigned long bootTime = millis();
  static bool feedReady = true;   // Ready by default (pin is LOW at idle via pull-down)
  static bool smsReady = true;

  if (now - bootTime < 15000) return;  // Skip during startup

  // Feed command: ESP32 GPIO2 → Arduino Pin 17 (active HIGH pulse, 1K pull-down)
  bool feedPin = digitalRead(17);
  if (feedPin == LOW) {
    feedReady = true;  // Pin is idle (pulled down), ready for next trigger
  }
  if (feedPin == HIGH && feedReady) {
    feedReady = false;  // Lock until pin returns to LOW
    Serial.println(F("[COMMAND] Feed trigger from ESP32!"));
    dispenseFeed(2);
  }

  // SMS command: ESP32 GPIO5 → Arduino Pin 2 (active HIGH pulse, 1K pull-down)
  bool smsPin = digitalRead(3);
  if (smsPin == LOW) {
    smsReady = true;  // Pin is idle (pulled down), ready for next trigger
  }
  if (smsPin == HIGH && smsReady) {
    smsReady = false;  // Lock until pin returns to LOW
    Serial.println(F("[COMMAND] SMS alert trigger from ESP32!"));
    sendSMS("ALERT: OxyFeeder - Safety threshold crossed! Check your system immediately.");
  }

  // --- Commands from ESP32 via Serial1 RX (Pin 19) ---
  static String serial1Buffer = "";
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n' || c == '\r') {
      if (serial1Buffer.length() > 0) {
        serial1Buffer.trim();
        if (serial1Buffer.startsWith("PHONE:")) {
          Serial.print(F("[ESP32] Phone command: "));
          Serial.println(serial1Buffer);
          handleCommand(serial1Buffer);
        }
        serial1Buffer = "";
      }
    } else {
      if (serial1Buffer.length() < MAX_COMMAND_LENGTH) serial1Buffer += c;
    }
  }

  // --- USB Serial commands (for testing via Serial Monitor) ---
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (commandBuffer.length() > 0) {
        commandBuffer.trim();
        Serial.print(F("[USB] Command: "));
        Serial.println(commandBuffer);
        handleCommand(commandBuffer);
        commandBuffer = "";
      }
    } else {
      if (commandBuffer.length() < MAX_COMMAND_LENGTH) {
        commandBuffer += c;
      }
    }
    }
}

void handleCommand(String command) {
  Serial.print(F("[CMD] Received: "));
  Serial.println(command);
  
  // Parse command format: CMD:VALUE
  int colonIndex = command.indexOf(':');
  if (colonIndex == -1) {
    Serial.println(F("[CMD] Invalid format (no colon)"));
    return;
  }
  
  String cmdType = command.substring(0, colonIndex);
  String cmdValue = command.substring(colonIndex + 1);
  
  cmdType.trim();
  cmdValue.trim();
  
  // Handle different commands
  if (cmdType == "PHONE") {
    // Set phone number
    if (cmdValue.length() > 0 && cmdValue.length() <= PHONE_NUMBER_LENGTH) {
      cmdValue.toCharArray(smsPhoneNumber, PHONE_NUMBER_LENGTH + 1);
      savePhoneNumberToEEPROM();
      Serial.print(F("[CMD] Phone number updated to: "));
      Serial.println(smsPhoneNumber);
      
      // Send confirmation back
      Serial1.println(F("{\"cmd\":\"PHONE\",\"status\":\"OK\"}"));
    } else {
      Serial.println(F("[CMD] Invalid phone number"));
      Serial1.println(F("{\"cmd\":\"PHONE\",\"status\":\"ERROR\"}"));
    }
  }
  else if (cmdType == "TIME") {
    // Set RTC time - format: TIME:HH:MM:SS
    // Example: TIME:14:30:00
    if (rtcAvailable && cmdValue.length() >= 8) {
      int h = cmdValue.substring(0, 2).toInt();
      int m = cmdValue.substring(3, 5).toInt();
      int s = cmdValue.substring(6, 8).toInt();
      DateTime now = rtc.now();
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), h, m, s));
      Serial.print(F("[CMD] RTC time set to: "));
      Serial.print(h); Serial.print(F(":"));
      if (m < 10) Serial.print(F("0")); Serial.print(m);
      Serial.print(F(":"));
      if (s < 10) Serial.print(F("0")); Serial.println(s);
    } else {
      Serial.println(F("[CMD] Invalid time format. Use TIME:HH:MM:SS"));
    }
  }
  else if (cmdType == "FEED") {
    // Manual feed command
    int duration = cmdValue.toInt();
    if (duration > 0 && duration <= 30) {
      Serial.print(F("[CMD] Manual feed for "));
      Serial.print(duration);
      Serial.println(F(" seconds"));

      dispenseFeed(duration);  // ✅ Now uses the actual duration parameter!

      Serial1.println(F("{\"cmd\":\"FEED\",\"status\":\"OK\"}"));
    } else {
      Serial.println(F("[CMD] Invalid feed duration"));
      Serial1.println(F("{\"cmd\":\"FEED\",\"status\":\"ERROR\"}"));
    }
  }
  else if (cmdType == "TEST_SMS") {
    // Test SMS command
    Serial.println(F("[CMD] Sending test SMS..."));
    sendSMS("OxyFeeder Test: SMS system is working!");
    Serial1.println(F("{\"cmd\":\"TEST_SMS\",\"status\":\"OK\"}"));
  }
  else if (cmdType == "GET_PHONE") {
    // Get current phone number
    Serial1.print(F("{\"cmd\":\"GET_PHONE\",\"phone\":\""));
    Serial1.print(smsPhoneNumber);
    Serial1.println(F("\"}"));
  }
  else if (cmdType == "CLEAR_SCHEDULES") {
    // Clear all schedules - owner doesn't want any auto feeding
    scheduleCount = 0;
    appSchedulesSynced = true;  // Mark that app has taken control
    Serial.println(F("[CMD] All schedules cleared - auto feeding disabled"));
    Serial1.println(F("{\"cmd\":\"CLEAR_SCHEDULES\",\"status\":\"OK\"}"));
  }
  else if (cmdType == "SCHEDULE") {
    // Add a new schedule
    // Format: SCHEDULE:HH:MM AM/PM,duration,enabled
    // e.g., SCHEDULE:08:00 AM,10,1
    if (scheduleCount >= MAX_SCHEDULES) {
      Serial.println(F("[CMD] Max schedules reached"));
      Serial1.println(F("{\"cmd\":\"SCHEDULE\",\"status\":\"ERROR\",\"reason\":\"MAX_REACHED\"}"));
      return;
    }
    
    // Parse the schedule: "HH:MM AM/PM,duration,enabled"
    int commaIndex1 = cmdValue.indexOf(',');
    int commaIndex2 = cmdValue.lastIndexOf(',');
    
    if (commaIndex1 == -1 || commaIndex2 == -1 || commaIndex1 == commaIndex2) {
      Serial.println(F("[CMD] Invalid schedule format"));
      Serial1.println(F("{\"cmd\":\"SCHEDULE\",\"status\":\"ERROR\",\"reason\":\"INVALID_FORMAT\"}"));
      return;
    }
    
    String timeStr = cmdValue.substring(0, commaIndex1);
    int duration = cmdValue.substring(commaIndex1 + 1, commaIndex2).toInt();
    int enabled = cmdValue.substring(commaIndex2 + 1).toInt();
    
    // Parse time string "HH:MM AM" or "HH:MM PM"
    int colonIdx = timeStr.indexOf(':');
    int spaceIdx = timeStr.indexOf(' ');
    
    if (colonIdx == -1) {
      Serial.println(F("[CMD] Invalid time format"));
      Serial1.println(F("{\"cmd\":\"SCHEDULE\",\"status\":\"ERROR\",\"reason\":\"INVALID_TIME\"}"));
      return;
    }
    
    int hour = timeStr.substring(0, colonIdx).toInt();
    int minute = timeStr.substring(colonIdx + 1, spaceIdx > 0 ? spaceIdx : timeStr.length()).toInt();
    
    // Convert to 24-hour format if AM/PM present
    if (spaceIdx > 0) {
      String ampm = timeStr.substring(spaceIdx + 1);
      ampm.trim();
      ampm.toUpperCase();
      if (ampm == "PM" && hour != 12) {
        hour += 12;
      } else if (ampm == "AM" && hour == 12) {
        hour = 0;
      }
    }
    
    // Add the schedule
    schedules[scheduleCount].hour = hour;
    schedules[scheduleCount].minute = minute;
    schedules[scheduleCount].duration = duration;
    schedules[scheduleCount].enabled = (enabled == 1);
    scheduleCount++;
    
    Serial.print(F("[CMD] Schedule added: "));
    Serial.print(hour);
    Serial.print(F(":"));
    if (minute < 10) Serial.print(F("0"));
    Serial.print(minute);
    Serial.print(F(", duration="));
    Serial.print(duration);
    Serial.print(F("s, enabled="));
    Serial.println(enabled);
    
    Serial1.println(F("{\"cmd\":\"SCHEDULE\",\"status\":\"OK\"}"));
  }
  else if (cmdType == "GET_SCHEDULES") {
    // Return all schedules
    Serial1.print(F("{\"cmd\":\"GET_SCHEDULES\",\"count\":"));
    Serial1.print(scheduleCount);
    Serial1.print(F(",\"schedules\":["));
    for (int i = 0; i < scheduleCount; i++) {
      if (i > 0) Serial1.print(F(","));
      Serial1.print(F("{\"h\":"));
      Serial1.print(schedules[i].hour);
      Serial1.print(F(",\"m\":"));
      Serial1.print(schedules[i].minute);
      Serial1.print(F(",\"d\":"));
      Serial1.print(schedules[i].duration);
      Serial1.print(F(",\"e\":"));
      Serial1.print(schedules[i].enabled ? 1 : 0);
      Serial1.print(F("}"));
    }
    Serial1.println(F("]}"));
  }
  else {
    Serial.print(F("[CMD] Unknown command: "));
    Serial.println(cmdType);
    Serial1.println(F("{\"cmd\":\"UNKNOWN\",\"status\":\"ERROR\"}"));
  }
}

// ============================================================================
// END OF FIRMWARE
// ============================================================================
