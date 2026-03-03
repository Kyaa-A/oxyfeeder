/*
  DO Sensor Diagnostic Test
  Reads raw voltage from Pin A1

  Test procedure:
  1. Air - baseline reading
  2. Electric fan pointed at probe - should increase voltage
  3. Stirred water (high O2) - high voltage
  4. Boiled then cooled water (low O2) - low voltage
*/

#define DO_PIN A1

void setup() {
  Serial.begin(9600);
  Serial.println("=====================================");
  Serial.println("  OxyFeeder: DO Sensor Bench Test");
  Serial.println("=====================================");
  Serial.println("Reading from Pin A1...");
  Serial.println("");
  Serial.println("Test sequence:");
  Serial.println("1. Hold probe in still air (baseline)");
  Serial.println("2. Point electric fan at probe (high O2)");
  Serial.println("3. Dip in stirred water (high O2)");
  Serial.println("4. Dip in boiled-cooled water (low O2)");
  Serial.println("");
}

void loop() {
  int rawValue = analogRead(DO_PIN);
  float voltage = rawValue * (5.0 / 1023.0);

  Serial.print("Raw: ");
  Serial.print(rawValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 3);
  Serial.println(" V");

  delay(1000);
}
