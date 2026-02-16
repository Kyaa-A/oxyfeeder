void setup() {
  Serial.begin(115200);
  pinMode(A2, INPUT); // Gamiton na nato ang A2
  Serial.println("OxyFeeder Diagnostic: Testing Pin A2...");
}

void loop() {
  int rawA2 = analogRead(A2); // Basaha ang A2
  float sPinVoltage = rawA2 * (5.0 / 1023.0);
  float batteryVoltage = sPinVoltage * 5.0; // Multiplier of 5

  Serial.print("Pin A2 Raw: "); Serial.print(rawA2);
  Serial.print(" | S-Pin V: "); Serial.print(sPinVoltage, 2);
  Serial.print("V | BATTERY: "); Serial.print(batteryVoltage, 1);
  Serial.println("V");

  delay(1000);
}
