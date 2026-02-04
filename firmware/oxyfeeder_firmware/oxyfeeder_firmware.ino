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
  
  SENSORS:
    - Dissolved Oxygen: DFRobot Analog on A1
    - Voltage Sensor: 0-25V Module on A0
    - Load Cell: HX711 on DT=8, SCK=9
    - RTC: DS3231 on I2C (SDA=20, SCL=21)
  
  ACTUATORS:
    - DC Motor (Spinner): L298N on IN1=10, IN2=11, ENA=12
    - Servo (Gate): Pin 6
    - Buzzer: Pin 7
  
  DISPLAY:
    - TFT LCD: ILI9341 via SPI (CS=40, DC=38, RST=-1)
  
  =============================================================================
*/

// ============================================================================
// LIBRARIES
// ============================================================================

#include <Wire.h>
#include <SPI.h>
#include <Servo.h>
#include <RTClib.h>
#include <HX711.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <EEPROM.h>

// ============================================================================
// PIN DEFINITIONS
// ============================================================================

// Sensors
#define DO_SENSOR_PIN       A1    // Dissolved Oxygen (Analog)
#define VOLTAGE_SENSOR_PIN  A0    // Battery Voltage (Analog)
#define HX711_DT_PIN        8     // Load Cell Data
#define HX711_SCK_PIN       9     // Load Cell Clock

// Actuators
#define MOTOR_IN1           10    // L298N Input 1
#define MOTOR_IN2           11    // L298N Input 2
#define MOTOR_ENA           12    // L298N Enable A (PWM)
#define SERVO_PIN           6     // Servo PWM
#define BUZZER_PIN          7     // Buzzer

// Display (SPI)
#define TFT_CS              40    // Chip Select
#define TFT_DC              38    // Data/Command
#define TFT_RST             -1    // Reset (use Arduino reset)

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
const int SERVO_OPEN_ANGLE = 90;
const int SERVO_CLOSED_ANGLE = 0;

// Load cell calibration - ADJUST THIS after calibrating with known weight
float calibration_factor = -7050.0;  // Start with this, adjust as needed
const float EMPTY_HOPPER_KG = 0.0;   // Weight when hopper is empty
const float FULL_HOPPER_KG = 5.0;    // Weight when hopper is full (in kg)

// Dissolved Oxygen calibration
// DFRobot DO sensor: V = 0-3V corresponds to 0-20 mg/L
const float DO_VOLTAGE_REF = 5.0;
const float DO_MAX_VALUE = 20.0;     // Max mg/L at max voltage

// Safety thresholds
const float DO_CRITICAL_THRESHOLD = 4.0;  // mg/L - trigger alarm below this
const int LOW_FEED_THRESHOLD = 20;        // % - trigger warning below this
const int LOW_BATTERY_THRESHOLD = 25;     // % - trigger warning below this

// Voltage sensor calibration
// 0-25V module with voltage divider ratio of 5:1
const float VOLTAGE_RATIO = 5.0;
const float BATTERY_FULL_VOLTAGE = 14.4;  // 12V battery fully charged
const float BATTERY_EMPTY_VOLTAGE = 11.0; // 12V battery empty

// Update intervals
const unsigned long SENSOR_UPDATE_INTERVAL = 2000;   // 2 seconds
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000;  // 1 second
const unsigned long JSON_SEND_INTERVAL = 2000;       // 2 seconds

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

RTC_DS3231 rtc;
HX711 scale;
Servo feedGate;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// ============================================================================
// GLOBAL VARIABLES - Current Sensor Readings
// ============================================================================

float currentDissolvedOxygen = 0.0;   // mg/L
int currentFeedLevel = 0;              // percentage 0-100
int currentBatteryPercent = 0;         // percentage 0-100
float currentBatteryVoltage = 0.0;     // volts
float currentWeight = 0.0;             // kg

DateTime currentTime;
bool rtcAvailable = false;
bool scaleAvailable = false;

// Timing variables
unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastJsonSend = 0;
bool lastFeedingDone = false;  // Prevents repeated feeding in same minute

// Non-blocking feeding state machine
enum FeedingState {
  FEEDING_IDLE,
  FEEDING_GATE_OPENING,
  FEEDING_DISPENSING,
  FEEDING_GATE_CLOSING,
  FEEDING_COMPLETE
};

FeedingState feedingState = FEEDING_IDLE;
unsigned long feedingStartTime = 0;
unsigned long feedingDuration = 0;
unsigned long stateTransitionTime = 0;

// SMS cooldown to prevent spam
unsigned long lastSmsSent = 0;
const unsigned long SMS_COOLDOWN = 300000;  // 5 minutes between SMS

// SMS Phone Number (stored in EEPROM)
#define EEPROM_PHONE_ADDR 0       // EEPROM starting address for phone number
#define PHONE_NUMBER_LENGTH 15    // Max length of phone number
char smsPhoneNumber[PHONE_NUMBER_LENGTH + 1] = "+639639192343";  // Default number

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
float readWeight();
int readFeedLevel();

void dispenseFeed(int seconds);
void updateFeedingState();  // Non-blocking feeding state machine
void openGate();
void closeGate();
void triggerAlarm();
void stopAlarm();

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
  Serial1.begin(9600);      // ESP32 Communication
  Serial3.begin(9600);      // GSM Module (SIM800L)
  
  while (!Serial) {
    ; // Wait for USB Serial (needed for some boards)
  }
  
  Serial.println(F(""));
  Serial.println(F("=============================================="));
  Serial.println(F("  OxyFeeder Firmware v2.0 - PRODUCTION BUILD  "));
  Serial.println(F("=============================================="));
  Serial.println(F(""));
  
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
  currentWeight = readWeight();
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
  
  // 2. UPDATE DISPLAY (every 1 second)
  if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdate = now;
    updateDisplay();
  }
  
  // 3. SEND JSON TO ESP32 (every 2 seconds)
  if (now - lastJsonSend >= JSON_SEND_INTERVAL) {
    lastJsonSend = now;
    sendJsonToESP32();
  }
  
  // 4. UPDATE NON-BLOCKING FEEDING STATE
  updateFeedingState();

  // 5. CHECK FEEDING SCHEDULE
  checkFeedingSchedule();

  // 6. CHECK SAFETY ALERTS
  checkSafetyAlerts();

  // 7. PROCESS INCOMING COMMANDS FROM APP
  processIncomingCommands();
  
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
    
    // Check if RTC lost power and needs to be set
    if (rtc.lostPower()) {
      Serial.println(F("RTC lost power, setting to compile time!"));
      // Set RTC to compile time - you may want to manually set this
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
  
  // Initialize Load Cell (HX711)
  Serial.print(F("  - Load Cell HX711: "));
  scale.begin(HX711_DT_PIN, HX711_SCK_PIN);
  
  if (scale.is_ready()) {
    scaleAvailable = true;
    scale.set_scale(calibration_factor);
    scale.tare();  // Reset to zero
    Serial.println(F("OK - Tared"));
  } else {
    scaleAvailable = false;
    Serial.println(F("FAILED! Check wiring."));
  }
  
  // Analog pins (no special init needed)
  Serial.println(F("  - DO Sensor (A1): OK"));
  Serial.println(F("  - Voltage Sensor (A0): OK"));
}

void initActuators() {
  Serial.println(F("[INIT] Initializing actuators..."));
  
  // Motor driver pins
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_ENA, OUTPUT);
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_ENA, 0);
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
  Serial.println(F("[INIT] Initializing TFT display..."));
  
  tft.begin();
  tft.setRotation(1);  // Landscape mode
  tft.fillScreen(ILI9341_BLACK);
  
  // Draw startup screen
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(3);
  tft.setCursor(60, 50);
  tft.println(F("OxyFeeder"));
  
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(50, 100);
  tft.println(F("Initializing..."));
  
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(80, 150);
  tft.println(F("Production Build v2.0"));
  
  delay(2000);
  tft.fillScreen(ILI9341_BLACK);
  
  Serial.println(F("  - TFT ILI9341: OK"));
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

float readWeight() {
  if (!scaleAvailable) {
    return 0.0;
  }
  
  // Get average of 5 readings for stability
  if (scale.is_ready()) {
    float weight = scale.get_units(5);
    
    // Ignore negative values (noise when empty)
    if (weight < 0) weight = 0;
    
    return weight;
  }
  
  return 0.0;
}

int readFeedLevel() {
  // Convert weight to percentage
  // EMPTY_HOPPER_KG = 0%, FULL_HOPPER_KG = 100%
  float weight = currentWeight;
  
  int percent = map(weight * 100, EMPTY_HOPPER_KG * 100, 
                    FULL_HOPPER_KG * 100, 0, 100);
  
  // Clamp to 0-100%
  percent = constrain(percent, 0, 100);
  
  return percent;
}

// ============================================================================
// ACTUATOR CONTROL FUNCTIONS
// ============================================================================

void dispenseFeed(int seconds) {
  if (feedingState != FEEDING_IDLE) {
    Serial.println(F("[ACTUATOR] Already feeding, ignoring request"));
    return;
  }

  Serial.print(F("[ACTUATOR] Starting feeding sequence for "));
  Serial.print(seconds);
  Serial.println(F(" seconds..."));

  // Store duration and start state machine
  feedingDuration = (unsigned long)seconds * 1000UL;
  feedingState = FEEDING_GATE_OPENING;
  stateTransitionTime = millis();
}

void updateFeedingState() {
  unsigned long now = millis();
  static FeedingState lastState = FEEDING_IDLE;  // Track state changes
  bool stateJustChanged = (feedingState != lastState);

  switch (feedingState) {
    case FEEDING_IDLE:
      // Nothing to do
      break;

    case FEEDING_GATE_OPENING:
      // Only trigger once when entering this state
      if (stateJustChanged) {
        feedGate.write(SERVO_OPEN_ANGLE);
        Serial.println(F("[FEEDING] Gate opening..."));
      }

      // Wait 500ms for servo to reach position
      if (now - stateTransitionTime >= 500) {
        // Start dispensing
        digitalWrite(MOTOR_IN1, HIGH);
        digitalWrite(MOTOR_IN2, LOW);
        analogWrite(MOTOR_ENA, 255);

        feedingState = FEEDING_DISPENSING;
        feedingStartTime = now;
        Serial.println(F("[FEEDING] Dispensing feed..."));
      }
      break;

    case FEEDING_DISPENSING:
      // Check if feeding duration has elapsed
      if (now - feedingStartTime >= feedingDuration) {
        // Stop motor
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, LOW);
        analogWrite(MOTOR_ENA, 0);

        feedingState = FEEDING_GATE_CLOSING;
        stateTransitionTime = now;
        Serial.println(F("[FEEDING] Feed dispensed, closing gate..."));
      }
      break;

    case FEEDING_GATE_CLOSING:
      // Only trigger once when entering this state
      if (stateJustChanged) {
        feedGate.write(SERVO_CLOSED_ANGLE);
        Serial.println(F("[FEEDING] Closing gate..."));
      }

      // Wait 500ms for servo
      if (now - stateTransitionTime >= 500) {
        feedingState = FEEDING_COMPLETE;
      }
      break;

    case FEEDING_COMPLETE:
      // Only trigger once when entering this state
      if (stateJustChanged) {
        triggerAlarm();
        Serial.println(F("[FEEDING] Feeding sequence complete!"));
      }

      // Return to idle
      feedingState = FEEDING_IDLE;
      break;
  }

  // Update state tracking for next iteration
  lastState = feedingState;
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
// DISPLAY FUNCTIONS
// ============================================================================

void updateDisplay() {
  // Clear only the data areas (not the whole screen - prevents flicker)
  
  // Header
  tft.fillRect(0, 0, 320, 40, ILI9341_NAVY);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(80, 12);
  tft.print(F("OxyFeeder"));
  
  // Time display
  tft.fillRect(0, 45, 320, 35, ILI9341_BLACK);
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(3);
  tft.setCursor(90, 50);
  
  if (rtcAvailable) {
    if (currentTime.hour() < 10) tft.print(F("0"));
    tft.print(currentTime.hour());
    tft.print(F(":"));
    if (currentTime.minute() < 10) tft.print(F("0"));
    tft.print(currentTime.minute());
    tft.print(F(":"));
    if (currentTime.second() < 10) tft.print(F("0"));
    tft.print(currentTime.second());
  } else {
    tft.print(F("--:--:--"));
  }
  
  // Sensor values - DO
  tft.fillRect(0, 90, 160, 50, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(10, 95);
  tft.print(F("Dissolved Oxygen"));
  tft.setTextSize(2);
  tft.setCursor(10, 110);
  if (currentDissolvedOxygen < DO_CRITICAL_THRESHOLD) {
    tft.setTextColor(ILI9341_RED);
  } else {
    tft.setTextColor(ILI9341_GREEN);
  }
  tft.print(currentDissolvedOxygen, 1);
  tft.print(F(" mg/L"));
  
  // Sensor values - Feed Level
  tft.fillRect(160, 90, 160, 50, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(170, 95);
  tft.print(F("Feed Level"));
  tft.setTextSize(2);
  tft.setCursor(170, 110);
  if (currentFeedLevel < LOW_FEED_THRESHOLD) {
    tft.setTextColor(ILI9341_ORANGE);
  } else {
    tft.setTextColor(ILI9341_GREEN);
  }
  tft.print(currentFeedLevel);
  tft.print(F(" %"));
  
  // Sensor values - Battery
  tft.fillRect(0, 150, 160, 50, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(10, 155);
  tft.print(F("Battery"));
  tft.setTextSize(2);
  tft.setCursor(10, 170);
  if (currentBatteryPercent < LOW_BATTERY_THRESHOLD) {
    tft.setTextColor(ILI9341_RED);
  } else {
    tft.setTextColor(ILI9341_GREEN);
  }
  tft.print(currentBatteryPercent);
  tft.print(F("% ("));
  tft.print(currentBatteryVoltage, 1);
  tft.print(F("V)"));
  
  // Status area
  tft.fillRect(160, 150, 160, 50, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(170, 155);
  tft.print(F("Status"));
  tft.setTextSize(2);
  tft.setCursor(170, 170);
  tft.setTextColor(ILI9341_GREEN);
  tft.print(F("ONLINE"));
  
  // Footer - next feeding time
  tft.fillRect(0, 210, 320, 30, ILI9341_DARKGREY);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(10, 220);
  tft.print(F("Next Feed: "));
  
  // Show next feeding time
  if (rtcAvailable) {
    int currentMinutes = currentTime.hour() * 60 + currentTime.minute();
    int nextFeedHour = -1;
    int nextFeedMin = -1;
    
    // Find next scheduled feeding time
    if (scheduleCount > 0) {
      int minDistance = 24 * 60;  // Max possible distance
      for (int i = 0; i < scheduleCount; i++) {
        if (!schedules[i].enabled) continue;
        int schedMinutes = schedules[i].hour * 60 + schedules[i].minute;
        int distance = schedMinutes - currentMinutes;
        if (distance < 0) distance += 24 * 60;  // Wrap to next day
        if (distance < minDistance) {
          minDistance = distance;
          nextFeedHour = schedules[i].hour;
          nextFeedMin = schedules[i].minute;
        }
      }
    } else if (!appSchedulesSynced) {
      // Use defaults only if app hasn't synced yet
      int feed1Minutes = DEFAULT_FEED_HOUR_1 * 60 + DEFAULT_FEED_MIN_1;
      int feed2Minutes = DEFAULT_FEED_HOUR_2 * 60 + DEFAULT_FEED_MIN_2;
      
      if (currentMinutes < feed1Minutes) {
        nextFeedHour = DEFAULT_FEED_HOUR_1;
        nextFeedMin = DEFAULT_FEED_MIN_1;
      } else if (currentMinutes < feed2Minutes) {
        nextFeedHour = DEFAULT_FEED_HOUR_2;
        nextFeedMin = DEFAULT_FEED_MIN_2;
      } else {
        nextFeedHour = DEFAULT_FEED_HOUR_1;
        nextFeedMin = DEFAULT_FEED_MIN_1;
        tft.print(F("Tmrw "));
      }
    }
    // If appSchedulesSynced && scheduleCount == 0, nextFeedHour stays -1 (will show "Disabled")
    
    if (nextFeedHour >= 0) {
      if (nextFeedHour < 10) tft.print(F("0"));
      tft.print(nextFeedHour);
      tft.print(F(":"));
      if (nextFeedMin < 10) tft.print(F("0"));
      tft.print(nextFeedMin);
    } else {
      tft.print(F("Disabled"));
    }
  } else {
    tft.print(F("--:--"));
  }
}

// ============================================================================
// COMMUNICATION FUNCTIONS
// ============================================================================

void sendJsonToESP32() {
  // Build JSON string matching the expected format
  Serial1.print(F("{"));
  Serial1.print(F("\"do\": "));
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
  
  // Set SMS to text mode (just in case)
  Serial3.println(F("AT+CMGF=1"));
  delay(500);
  
  // Set recipient phone number (from configurable variable)
  Serial3.print(F("AT+CMGS=\""));
  Serial3.print(smsPhoneNumber);
  Serial3.println(F("\""));
  delay(500);
  
  // Send message
  Serial3.print(message);
  delay(100);
  
  // Send Ctrl+Z to send the message
  Serial3.write(26);
  delay(3000);
  
  lastSmsSent = now;
  Serial.println(F("[GSM] SMS sent!"));
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
  
  // Check dynamic schedules from app
  if (scheduleCount > 0) {
    for (int i = 0; i < scheduleCount; i++) {
      if (schedules[i].enabled && 
          schedules[i].hour == hour && 
          schedules[i].minute == minute && 
          second < 10) {
        shouldFeed = true;
        break;
      }
    }
  } else if (!appSchedulesSynced) {
    // Only use defaults if app has NEVER synced schedules
    // Once app syncs (even with empty list), defaults are disabled
    bool isFeedTime1 = (hour == DEFAULT_FEED_HOUR_1 && minute == DEFAULT_FEED_MIN_1 && second < 10);
    bool isFeedTime2 = (hour == DEFAULT_FEED_HOUR_2 && minute == DEFAULT_FEED_MIN_2 && second < 10);
    shouldFeed = isFeedTime1 || isFeedTime2;
  }
  // If appSchedulesSynced is true but scheduleCount is 0, no feeding happens
  
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
  // Use non-blocking dispenseFeed with default duration
  // The state machine handles: gate open -> dispense -> gate close -> beep
  dispenseFeed(FEED_DURATION_SECONDS);
}

void checkSafetyAlerts() {
  // Check for critical low dissolved oxygen
  if (currentDissolvedOxygen < DO_CRITICAL_THRESHOLD && currentDissolvedOxygen > 0) {
    Serial.println(F("[ALERT] CRITICAL: Low Dissolved Oxygen!"));
    
    // Beep alarm
    triggerAlarm();
    
    // Send SMS alert
    sendSMS("CRITICAL ALERT: OxyFeeder - Low Dissolved Oxygen detected! Level is below 4.0 mg/L. Check pond immediately!");
  }
  
  // Check for low battery (warning only, no SMS)
  if (currentBatteryPercent < LOW_BATTERY_THRESHOLD && currentBatteryPercent > 0) {
    Serial.println(F("[WARNING] Low battery detected"));
    // Could add SMS here if desired
  }
  
  // Check for low feed level (warning only, no SMS)
  if (currentFeedLevel < LOW_FEED_THRESHOLD) {
    Serial.println(F("[WARNING] Low feed level detected"));
    // Could add SMS here if desired
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
  // Check for incoming data from ESP32 (via Serial1)
  while (Serial1.available()) {
    char c = Serial1.read();
    
    if (c == '\n' || c == '\r') {
      // End of command
      if (commandBuffer.length() > 0) {
        handleCommand(commandBuffer);
        commandBuffer = "";
      }
    } else {
      // Add character to buffer
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
