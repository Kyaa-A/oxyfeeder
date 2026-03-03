/*
  Actuator Test - Buzzer & Servo
  Tests Pin 6 (Servo) and Pin 7 (Buzzer)
*/

#include <Servo.h>

#define SERVO_PIN 6
#define BUZZER_PIN 7

Servo myServo;

void setup() {
  Serial.begin(9600);
  Serial.println("--- TESTING ACTUATORS ---");

  // Setup pins
  pinMode(BUZZER_PIN, OUTPUT);
  myServo.attach(SERVO_PIN);

  // 1. TEST BUZZER (try both methods)
  Serial.println("Testing Buzzer Method 1 (Active - digitalWrite)...");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
  delay(300);

  Serial.println("Testing Buzzer Method 2 (Passive - tone)...");
  tone(BUZZER_PIN, 1000);  // 1000Hz frequency
  delay(500);
  noTone(BUZZER_PIN);
  delay(300);
  tone(BUZZER_PIN, 2000);  // 2000Hz frequency
  delay(500);
  noTone(BUZZER_PIN);

  // 2. TEST SERVO
  Serial.println("Testing Servo... Dapat mo-lihok ni!");
  myServo.write(0);   // Start position
  delay(1000);
  myServo.write(90);  // Open gate
  delay(1000);
  myServo.write(0);   // Close gate
  delay(1000);

  Serial.println("TEST COMPLETE!");
}

void loop() {
  // Do nothing - test runs once in setup
}
