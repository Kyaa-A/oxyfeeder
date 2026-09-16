#include <Servo.h>

const int PRIMARY_SERVO_PIN = 6;
const int COMPARISON_SERVO_PIN = 22;

Servo primaryServo;
Servo comparisonServo;

void setup() {
  Serial.begin(9600);

  primaryServo.attach(PRIMARY_SERVO_PIN);
  comparisonServo.attach(COMPARISON_SERVO_PIN);

  primaryServo.write(0);
  comparisonServo.write(0);
  delay(1500);
}

void loop() {
  Serial.println(F("Servo command: 90 degrees"));
  primaryServo.write(90);
  comparisonServo.write(90);
  delay(2000);

  Serial.println(F("Servo command: 0 degrees"));
  primaryServo.write(0);
  comparisonServo.write(0);
  delay(2000);
}
