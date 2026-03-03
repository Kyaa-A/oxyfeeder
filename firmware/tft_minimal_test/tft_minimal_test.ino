/*
  TFT Minimal Test - ST7796S 4.0" 480x320
  Just fills screen with RED - if you see RED, SPI is working
*/

#include <Arduino_GFX_Library.h>

// Pin definitions (same as main firmware)
#define TFT_CS   40
#define TFT_DC   38
#define TFT_RST  -1   // RST tied to 3.3V

// Hardware SPI: MOSI=51, SCK=52 (fixed on Mega)
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);
Arduino_GFX *tft = new Arduino_ST7796(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

void setup() {
  Serial.begin(9600);
  Serial.println("TFT Minimal Test");
  Serial.println("================");

  Serial.print("Initializing TFT... ");

  if (!tft->begin()) {
    Serial.println("FAILED!");
    while(1); // Stop here
  }

  Serial.println("OK!");

  // Fill entire screen with RED
  Serial.println("Filling screen RED...");
  tft->fillScreen(0xF800);  // RED in RGB565
  Serial.println("Done! You should see a RED screen.");

  delay(2000);

  // Try other colors
  Serial.println("Filling screen GREEN...");
  tft->fillScreen(0x07E0);  // GREEN
  delay(2000);

  Serial.println("Filling screen BLUE...");
  tft->fillScreen(0x001F);  // BLUE
  delay(2000);

  Serial.println("Filling screen WHITE...");
  tft->fillScreen(0xFFFF);  // WHITE
  delay(2000);

  Serial.println("Filling screen BLACK...");
  tft->fillScreen(0x0000);  // BLACK

  Serial.println("\nTest complete!");
  Serial.println("If screen stayed WHITE the whole time = wiring issue");
  Serial.println("If you saw colors = TFT working!");
}

void loop() {
  // Cycle colors slowly
  tft->fillScreen(0xF800);  // RED
  delay(1000);
  tft->fillScreen(0x07E0);  // GREEN
  delay(1000);
  tft->fillScreen(0x001F);  // BLUE
  delay(1000);
}
