#include "HX711.h"

// --- PIN DEFINITIONS ---
#define VOLT_PIN A2          // Healthy Pin (A0 is dead)
#define DO_PIN A1            // Dissolved Oxygen sensor
#define LOADCELL_DT_PIN 10   // DT pin (Pin 8/9 not working)
#define LOADCELL_SCK_PIN 11  // SCK pin

// --- OBJECTS ---
HX711 scale;

// --- CALIBRATION ---
float calibration_factor = 2.0; // Katong nakuha nimo sa iPhone test

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n==================================");
  Serial.println("   OXYFEEDER: ALL SENSORS TEST    ");
  Serial.println("==================================");

  // Initialize Voltage Sensor
  pinMode(VOLT_PIN, INPUT);

  // Initialize DO Sensor
  pinMode(DO_PIN, INPUT);

  // Initialize Load Cell
  scale.begin(LOADCELL_DT_PIN, LOADCELL_SCK_PIN);
  if (scale.is_ready()) {
    scale.set_scale(calibration_factor);
    scale.tare(); // I-reset sa zero inig start
    Serial.println("[OK] HX711 Load Cell Found!");
  } else {
    Serial.println("[!!] HX711 NOT FOUND! Check wires on 10 & 11.");
  }

  Serial.println("[DO] Pin A1 configured - check voltage readings");
}

void loop() {
  // 1. VOLTAGE SENSOR READING
  int rawVolt = analogRead(VOLT_PIN);
  float sPinVoltage = rawVolt * (5.0 / 1023.0);
  float batteryVoltage = sPinVoltage * 5.0; // Multiplier of 5

  // 2. DO SENSOR READING
  int rawDO = analogRead(DO_PIN);
  float voltageDO = rawDO * (5.0 / 1023.0);

  // 3. LOAD CELL READING
  float weight = 0;
  long rawWeight = 0;
  if (scale.is_ready()) {
    weight = scale.get_units(5); // Average of 5
    rawWeight = scale.read();
  }

  // --- OUTPUT TO SERIAL MONITOR ---
  Serial.print("BATT: ");
  Serial.print(batteryVoltage, 1); Serial.print("V");

  Serial.print("  |  DO: ");
  Serial.print(voltageDO, 2); Serial.print("V");

  Serial.print("  |  WEIGHT_RAW: ");
  Serial.println(rawWeight);

  delay(1000); // Update every second
}
