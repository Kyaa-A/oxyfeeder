/*
 * OxyFeeder System Diagnostic Sketch
 * ===================================
 * Bench Test - Tests all components EXCEPT DO Sensor
 *
 * Purpose: Verify hardware is working before final assembly
 *
 * Wiring Configuration:
 * - Voltage Sensor: A0 (reads 12V via divider)
 * - DO Sensor: A1 (SKIPPED - not connected for this test)
 * - Servo: Pin 6
 * - Buzzer: Pin 7
 * - Load Cell (HX711): DT=8, SCK=9
 * - L298N Motor: IN1=10, IN2=11, ENA=12
 * - GSM (SIM800L): Serial3 (RX=15, TX=14)
 * - ESP32 Bridge: Serial1 (TX=18, RX=19)
 * - RTC (DS3231): I2C (SDA=20, SCL=21)
 * - TFT LCD (ILI9341): CS=40, DC=38, MOSI=51, SCK=52, RST=3.3V
 */

#include <Wire.h>
#include <Servo.h>
#include <HX711.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

// ============== PIN DEFINITIONS ==============
// Analog Sensors
#define VOLTAGE_SENSOR_PIN A0
#define DO_SENSOR_PIN      A1  // Not used in this test

// Digital Outputs
#define SERVO_PIN          6
#define BUZZER_PIN         7

// Load Cell (HX711)
#define LOADCELL_DT        8
#define LOADCELL_SCK       9

// Motor Driver (L298N)
#define MOTOR_IN1          10
#define MOTOR_IN2          11
#define MOTOR_ENA          12

// TFT Display (ILI9341)
#define TFT_CS             40
#define TFT_DC             38
#define TFT_RST            -1  // Hardwired to 3.3V

// ============== VOLTAGE SENSOR CALIBRATION ==============
// For a voltage divider reading 12V:
// If using 30K/7.5K divider: ratio = (30+7.5)/7.5 = 5.0
// If using 25K/5K divider: ratio = (25+5)/5 = 6.0
// Adjust based on your module
#define VOLTAGE_DIVIDER_RATIO  5.0
#define ARDUINO_REF_VOLTAGE    5.0

// ============== OBJECTS ==============
Servo feederServo;
HX711 loadCell;
RTC_DS3231 rtc;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// ============== TIMING ==============
unsigned long lastServoToggle = 0;
unsigned long lastMotorRun = 0;
unsigned long lastDisplayUpdate = 0;
bool servoOpen = false;
bool motorRunning = false;
unsigned long motorStartTime = 0;

// ============== TEST INTERVALS ==============
#define SERVO_TOGGLE_INTERVAL  5000   // 5 seconds
#define MOTOR_RUN_DURATION     2000   // 2 seconds
#define MOTOR_CYCLE_INTERVAL   10000  // 10 seconds between motor tests
#define DISPLAY_UPDATE_INTERVAL 500   // Update display every 500ms

// ============== DATA VARIABLES ==============
float voltage = 0.0;
float weight = 0.0;
String timeString = "";
int testCycle = 0;

void setup() {
  // Initialize Serial ports
  Serial.begin(9600);     // Debug/Monitor (changed to 9600 for compatibility)
  Serial1.begin(9600);    // ESP32 Bridge
  Serial3.begin(9600);    // GSM Module

  Serial.println(F(""));
  Serial.println(F("========================================"));
  Serial.println(F("   OxyFeeder BENCH TEST DIAGNOSTIC"));
  Serial.println(F("   (DO Sensor EXCLUDED from this test)"));
  Serial.println(F("========================================"));
  Serial.println(F(""));

  // Initialize I2C
  Wire.begin();

  // Initialize Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Startup beep
  beep(100);
  delay(100);
  beep(100);
  Serial.println(F("[OK] Buzzer initialized"));

  // Initialize Motor Driver
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_ENA, OUTPUT);
  stopMotor();
  Serial.println(F("[OK] Motor driver initialized"));

  // Initialize Servo
  feederServo.attach(SERVO_PIN);
  feederServo.write(0);  // Start closed
  Serial.println(F("[OK] Servo initialized (position: 0)"));

  // Initialize Load Cell
  loadCell.begin(LOADCELL_DT, LOADCELL_SCK);
  if (loadCell.is_ready()) {
    Serial.println(F("[OK] Load Cell (HX711) detected"));
    loadCell.set_scale();  // Use default scale for raw reading
    loadCell.tare();       // Reset to zero
    Serial.println(F("[OK] Load Cell tared"));
  } else {
    Serial.println(F("[ERROR] Load Cell (HX711) NOT detected!"));
  }

  // Initialize RTC
  if (rtc.begin()) {
    Serial.println(F("[OK] RTC (DS3231) detected"));
    if (rtc.lostPower()) {
      Serial.println(F("[WARN] RTC lost power, setting time to compile time"));
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    DateTime now = rtc.now();
    Serial.print(F("[OK] RTC Time: "));
    Serial.println(formatDateTime(now));
  } else {
    Serial.println(F("[ERROR] RTC (DS3231) NOT detected!"));
  }

  // Initialize TFT Display
  tft.begin();
  tft.setRotation(1);  // Landscape
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println(F("OxyFeeder Bench Test"));
  tft.setTextSize(1);
  tft.setCursor(10, 40);
  tft.println(F("Initializing..."));
  Serial.println(F("[OK] TFT Display initialized"));

  // Test ESP32 communication
  Serial1.println(F("{\"status\":\"bench_test_started\"}"));
  Serial.println(F("[OK] Sent startup message to ESP32 (Serial1)"));

  // Test GSM Module
  Serial3.println(F("AT"));
  Serial.println(F("[OK] Sent AT command to GSM (Serial3)"));

  delay(1000);

  // Check GSM response
  if (Serial3.available()) {
    String response = Serial3.readString();
    Serial.print(F("[GSM Response] "));
    Serial.println(response);
  } else {
    Serial.println(F("[WARN] No response from GSM module"));
  }

  Serial.println(F(""));
  Serial.println(F("========================================"));
  Serial.println(F("         STARTING MAIN TEST LOOP"));
  Serial.println(F("========================================"));
  Serial.println(F(""));

  // Three beeps to indicate ready
  beep(200);
  delay(200);
  beep(200);
  delay(200);
  beep(200);
}

void loop() {
  unsigned long currentMillis = millis();

  // ============== READ SENSORS ==============

  // Read Voltage Sensor
  int rawVoltage = analogRead(VOLTAGE_SENSOR_PIN);
  voltage = (rawVoltage / 1023.0) * ARDUINO_REF_VOLTAGE * VOLTAGE_DIVIDER_RATIO;

  // Read Load Cell
  if (loadCell.is_ready()) {
    weight = loadCell.get_units(5);  // Average of 5 readings
  }

  // Read RTC
  DateTime now = rtc.now();
  timeString = formatDateTime(now);

  // ============== SERVO CONTROL ==============
  if (currentMillis - lastServoToggle >= SERVO_TOGGLE_INTERVAL) {
    lastServoToggle = currentMillis;
    servoOpen = !servoOpen;

    if (servoOpen) {
      feederServo.write(90);  // Open position
      Serial.println(F("[SERVO] Opening to 90 degrees"));
      beep(50);
    } else {
      feederServo.write(0);   // Closed position
      Serial.println(F("[SERVO] Closing to 0 degrees"));
      beep(50);
    }
  }

  // ============== MOTOR CONTROL ==============
  if (!motorRunning && (currentMillis - lastMotorRun >= MOTOR_CYCLE_INTERVAL)) {
    // Start motor
    motorRunning = true;
    motorStartTime = currentMillis;
    runMotorForward();
    Serial.println(F("[MOTOR] Starting (forward) for 2 seconds"));
    beep(100);
  }

  if (motorRunning && (currentMillis - motorStartTime >= MOTOR_RUN_DURATION)) {
    // Stop motor
    motorRunning = false;
    lastMotorRun = currentMillis;
    stopMotor();
    Serial.println(F("[MOTOR] Stopped"));
    beep(100);
  }

  // ============== DISPLAY UPDATE ==============
  if (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdate = currentMillis;
    testCycle++;

    // Print to Serial Monitor
    printToSerial();

    // Update TFT Display
    updateDisplay();

    // Send to ESP32
    sendToESP32();
  }
}

// ============== HELPER FUNCTIONS ==============

void beep(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

void runMotorForward() {
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_ENA, 200);  // ~78% speed
}

void runMotorBackward() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);
  analogWrite(MOTOR_ENA, 200);
}

void stopMotor() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_ENA, 0);
}

String formatDateTime(DateTime dt) {
  char buffer[20];
  sprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d",
          dt.month(), dt.day(), dt.year(),
          dt.hour(), dt.minute(), dt.second());
  return String(buffer);
}

void printToSerial() {
  Serial.println(F("----------------------------------------"));
  Serial.print(F("Test Cycle: "));
  Serial.println(testCycle);
  Serial.println(F(""));

  Serial.print(F("VOLTAGE:    "));
  Serial.print(voltage, 2);
  Serial.println(F(" V"));

  Serial.print(F("DO SENSOR:  "));
  Serial.println(F("(SKIPPED - Not connected)"));

  Serial.print(F("WEIGHT:     "));
  Serial.print(weight, 2);
  Serial.println(F(" units"));

  Serial.print(F("TIME:       "));
  Serial.println(timeString);

  Serial.print(F("SERVO:      "));
  Serial.println(servoOpen ? F("OPEN (90°)") : F("CLOSED (0°)"));

  Serial.print(F("MOTOR:      "));
  Serial.println(motorRunning ? F("RUNNING") : F("STOPPED"));

  Serial.println(F("----------------------------------------"));
  Serial.println(F(""));
}

void updateDisplay() {
  // Clear display area (keep header)
  tft.fillRect(0, 50, 320, 190, ILI9341_BLACK);

  tft.setTextSize(2);

  // Voltage
  tft.setCursor(10, 60);
  tft.setTextColor(ILI9341_YELLOW);
  tft.print(F("Voltage: "));
  tft.setTextColor(ILI9341_WHITE);
  tft.print(voltage, 2);
  tft.println(F(" V"));

  // Weight
  tft.setCursor(10, 90);
  tft.setTextColor(ILI9341_CYAN);
  tft.print(F("Weight:  "));
  tft.setTextColor(ILI9341_WHITE);
  tft.print(weight, 1);
  tft.println(F(" u"));

  // Time
  tft.setCursor(10, 120);
  tft.setTextColor(ILI9341_GREEN);
  tft.print(F("Time: "));
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.println(timeString);

  // Servo Status
  tft.setTextSize(2);
  tft.setCursor(10, 150);
  tft.setTextColor(ILI9341_MAGENTA);
  tft.print(F("Servo:   "));
  if (servoOpen) {
    tft.setTextColor(ILI9341_GREEN);
    tft.println(F("OPEN"));
  } else {
    tft.setTextColor(ILI9341_RED);
    tft.println(F("CLOSED"));
  }

  // Motor Status
  tft.setCursor(10, 180);
  tft.setTextColor(ILI9341_ORANGE);
  tft.print(F("Motor:   "));
  if (motorRunning) {
    tft.setTextColor(ILI9341_GREEN);
    tft.println(F("RUN"));
  } else {
    tft.setTextColor(ILI9341_RED);
    tft.println(F("STOP"));
  }

  // Test cycle counter
  tft.setCursor(10, 220);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_DARKGREY);
  tft.print(F("Cycle: "));
  tft.println(testCycle);
}

void sendToESP32() {
  // Create JSON string
  String json = "{";
  json += "\"voltage\":" + String(voltage, 2) + ",";
  json += "\"weight\":" + String(weight, 2) + ",";
  json += "\"time\":\"" + timeString + "\",";
  json += "\"servo\":\"" + String(servoOpen ? "open" : "closed") + "\",";
  json += "\"motor\":\"" + String(motorRunning ? "running" : "stopped") + "\",";
  json += "\"cycle\":" + String(testCycle);
  json += "}";

  Serial1.println(json);
}
