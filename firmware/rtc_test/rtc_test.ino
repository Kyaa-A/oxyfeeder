// I2C Scanner - Test RTC Direct on Arduino
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println("--- I2C Scanner (Direct on Arduino) ---");
  Serial.println("Looking for RTC at 0x68...");
}

void loop() {
  byte error, address;
  int devicesFound = 0;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);

      if (address == 0x68) Serial.print(" <-- DS3231 RTC!");
      if (address == 0x57) Serial.print(" (DS3231 EEPROM)");

      Serial.println();
      devicesFound++;
    }
  }

  if (devicesFound == 0) {
    Serial.println("No I2C devices found. RTC module may be damaged.");
  } else {
    Serial.print("Total devices: ");
    Serial.println(devicesFound);
  }

  Serial.println("-------------------");
  delay(3000);
}
