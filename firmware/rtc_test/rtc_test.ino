// Test Pins 22, 23, 24 - ALL AT ONCE
// IMPORTANT: Tangtanga ang RTC wires completely!

void setup() {
  Serial.begin(115200);
  pinMode(22, INPUT_PULLUP);
  pinMode(23, INPUT_PULLUP);
  pinMode(24, INPUT_PULLUP);
  Serial.println("--- Testing Pins 22, 23, 24 ---");
  Serial.println("ALL should be 1 if healthy (no wires connected)");
}

void loop() {
  Serial.print("Pin 22: "); Serial.print(digitalRead(22));
  Serial.print(" | Pin 23: "); Serial.print(digitalRead(23));
  Serial.print(" | Pin 24: "); Serial.println(digitalRead(24));
  delay(500);
}
