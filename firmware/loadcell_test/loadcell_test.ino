#include "HX711.h"

const int LOADCELL_DOUT_PIN = 10;
const int LOADCELL_SCK_PIN = 11;

HX711 scale;

void setup() {
  Serial.begin(115200);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.println("--- OxyFeeder Calibration ---");
  Serial.println("1. Ayaw usa tauri og load ang scale.");
  Serial.println("2. Paghulat og 5 seconds para sa TARE...");

  delay(5000);
  scale.set_scale(); // Default scale
  scale.tare();      // Reset to zero

  Serial.println("TARE DONE! Ibutang na ang 'Known Weight' (e.g. 500g water bottle)");
}

void loop() {
  if (scale.is_ready()) {
    long reading = scale.get_units(10); // Average of 10 readings
    Serial.print("Raw Average: ");
    Serial.print(reading);
    Serial.println("  |  Divide this by your weight to get Calibration Factor");
  }
  delay(1000);
}
