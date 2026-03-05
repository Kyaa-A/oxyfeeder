/*
 * =============================================================================
 * OxyFeeder RTC DS3231 Test
 * =============================================================================
 *
 * Tests the replacement RTC module for scheduled feeding functionality.
 *
 * WIRING (I2C - Fixed pins on Arduino Mega):
 *   RTC VCC  -> 5V Rail (Perfboard)
 *   RTC GND  -> GND Rail (Perfboard)
 *   RTC SDA  -> Arduino Pin 20 (SDA) - FIXED, cannot change
 *   RTC SCL  -> Arduino Pin 21 (SCL) - FIXED, cannot change
 *
 * Note: Pin 20 and 21 are safe - damaged pins are A0, 8, 9
 *
 * IMPORTANT: Make sure CR2032 coin battery is installed in RTC module!
 * Without battery, time resets to 2015 on every power cycle.
 *
 * =============================================================================
 */

#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

// Days of week for display
const char daysOfWeek[7][12] = {
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println(F("==========================================="));
  Serial.println(F("    OxyFeeder RTC DS3231 Test"));
  Serial.println(F("==========================================="));
  Serial.println();

  // Initialize I2C
  Wire.begin();

  // Scan for I2C devices first
  Serial.println(F("Scanning I2C bus..."));
  byte deviceCount = 0;
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("  Found device at 0x"));
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      if (address == 0x68) {
        Serial.println(F(" <- DS3231 RTC"));
      } else if (address == 0x57) {
        Serial.println(F(" <- DS3231 EEPROM"));
      } else {
        Serial.println();
      }
      deviceCount++;
    }
  }

  if (deviceCount == 0) {
    Serial.println(F("  No I2C devices found!"));
    Serial.println(F("  Check wiring: SDA=Pin20, SCL=Pin21"));
    Serial.println();
    Serial.println(F("TEST FAILED - No RTC detected"));
    while (1) delay(1000);
  }

  Serial.println();

  // Initialize RTC
  Serial.println(F("Initializing RTC..."));
  if (!rtc.begin()) {
    Serial.println(F("ERROR: Couldn't find RTC!"));
    Serial.println(F("Check wiring: SDA=Pin20, SCL=Pin21"));
    Serial.println();
    Serial.println(F("TEST FAILED"));
    while (1) delay(1000);
  }

  Serial.println(F("RTC Found!"));
  Serial.println();

  // Check if RTC lost power (battery dead or first use)
  if (rtc.lostPower()) {
    Serial.println(F("WARNING: RTC lost power!"));
    Serial.println(F("Setting time from compile timestamp..."));
    // This sets RTC to the time when sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println(F("Time has been set."));
    Serial.println();
  }

  // Read and display RTC temperature (DS3231 has built-in temp sensor)
  float temp = rtc.getTemperature();
  Serial.print(F("RTC Temperature: "));
  Serial.print(temp);
  Serial.println(F(" C"));
  Serial.println();

  Serial.println(F("==========================================="));
  Serial.println(F("         RTC TEST SUCCESSFUL!"));
  Serial.println(F("==========================================="));
  Serial.println();
  Serial.println(F("Current time will be displayed every second:"));
  Serial.println();
}

void loop() {
  DateTime now = rtc.now();

  // Format: YYYY/MM/DD (DayName) - HH:MM:SS
  Serial.print(now.year(), DEC);
  Serial.print('/');
  if (now.month() < 10) Serial.print('0');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  if (now.day() < 10) Serial.print('0');
  Serial.print(now.day(), DEC);

  Serial.print(" (");
  Serial.print(daysOfWeek[now.dayOfTheWeek()]);
  Serial.print(") - ");

  if (now.hour() < 10) Serial.print('0');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  if (now.minute() < 10) Serial.print('0');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  if (now.second() < 10) Serial.print('0');
  Serial.println(now.second(), DEC);

  delay(1000);
}
