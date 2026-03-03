/*
  TFT Proof of Life Test - ST7796S 4.0" 480x320

  Hardware Configuration (2 Logic Level Shifters):
  ------------------------------------------------
  Shifter #1 (ESP32): Pin 18 (TX1), Pin 19 (RX1)

  Shifter #2 (LCD):
    HV1 → LV1: Pin 40 (CS)
    HV2 → LV2: Pin 38 (DC)
    HV3 → LV3: Pin 52 (SCK)
    HV4 → LV4: Pin 51 (MOSI)

  LCD Power:
    VCC = 5V
    GND = Common GND
    LED = 3.3V
    RST = 3.3V (hardwired)
*/

#include <Arduino_GFX_Library.h>

// Pin Definitions
#define TFT_CS   40
#define TFT_DC   38
#define TFT_RST  -1   // RST hardwired to 3.3V

// Hardware SPI: MOSI=51, SCK=52 (fixed on Mega)
// Using 2MHz speed for logic level shifter compatibility
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS, 2000000);  // 2MHz SPI speed
Arduino_GFX *gfx = new Arduino_ST7796(bus, TFT_RST, 0 /* rotation */, false /* IPS */);

// Colors (RGB565)
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define CYAN    0x07FF

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("=========================================");
  Serial.println("  TFT Proof of Life Test - ST7796S");
  Serial.println("=========================================");
  Serial.println();

  Serial.print("Initializing TFT display... ");

  if (!gfx->begin()) {
    Serial.println("FAILED!");
    Serial.println("Check wiring:");
    Serial.println("  - CS (Pin 40) -> Shifter HV1/LV1 -> LCD CS");
    Serial.println("  - DC (Pin 38) -> Shifter HV2/LV2 -> LCD DC");
    Serial.println("  - SCK (Pin 52) -> Shifter HV3/LV3 -> LCD SCK");
    Serial.println("  - MOSI (Pin 51) -> Shifter HV4/LV4 -> LCD SDI");
    while(1);
  }

  Serial.println("Display initialized OK!");
  Serial.println();

  // Color test sequence
  Serial.println("Starting color test...");

  Serial.println("  Filling RED...");
  gfx->fillScreen(RED);
  delay(1000);

  Serial.println("  Filling GREEN...");
  gfx->fillScreen(GREEN);
  delay(1000);

  Serial.println("  Filling BLUE...");
  gfx->fillScreen(BLUE);
  delay(1000);

  // Display success message
  Serial.println("  Drawing text...");
  gfx->fillScreen(BLACK);

  // Title
  gfx->setTextColor(CYAN);
  gfx->setTextSize(4);
  gfx->setCursor(60, 100);
  gfx->println("OXYFEEDER");

  gfx->setTextSize(3);
  gfx->setCursor(120, 150);
  gfx->println("SYSTEM");

  // Success message
  gfx->setTextColor(GREEN);
  gfx->setTextSize(2);
  gfx->setCursor(50, 220);
  gfx->println("HARDWARE TEST: SUCCESSFUL");

  Serial.println();
  Serial.println("=========================================");
  Serial.println("  TEST COMPLETE!");
  Serial.println("  If you see colors and text on screen,");
  Serial.println("  your TFT is working perfectly!");
  Serial.println("=========================================");
}

void loop() {
  // Cycle colors slowly to confirm display is alive
  delay(5000);

  gfx->fillScreen(RED);
  delay(1000);

  gfx->fillScreen(GREEN);
  delay(1000);

  gfx->fillScreen(BLUE);
  delay(1000);

  // Redraw text
  gfx->fillScreen(BLACK);
  gfx->setTextColor(CYAN);
  gfx->setTextSize(4);
  gfx->setCursor(60, 100);
  gfx->println("OXYFEEDER");
  gfx->setTextSize(3);
  gfx->setCursor(120, 150);
  gfx->println("SYSTEM");
  gfx->setTextColor(GREEN);
  gfx->setTextSize(2);
  gfx->setCursor(50, 220);
  gfx->println("HARDWARE TEST: SUCCESSFUL");
}
