/*
 * 4.0" TFT ST7796S Test
 * =====================
 *
 * Tests 480x320 TFT display with ST7796S driver
 *
 * Wiring (Arduino Mega → TFT via Logic Level Shifter):
 *   TFT VCC   → 5V
 *   TFT GND   → GND
 *   TFT LED   → 3.3V (backlight)
 *   TFT CS    → Pin 40 (via shifter)
 *   TFT RST   → Pin 49 (via shifter, or direct to 3.3V)
 *   TFT DC    → Pin 38 (via shifter)
 *   TFT SDI   → Pin 51 / MOSI (via shifter)
 *   TFT SCK   → Pin 52 (via shifter)
 *
 * Library needed: GFX Library for Arduino
 *   Install via: Sketch → Include Library → Manage Libraries
 *   Search "GFX Library for Arduino" by Moon On Our Nation
 *   Also install "Adafruit GFX Library"
 */

#include <Arduino_GFX_Library.h>

/* Color Definitions (RGB565 format) */
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define CYAN    0x07FF
#define YELLOW  0xFFE0
#define MAGENTA 0xF81F

/* Pin Definitions */
#define TFT_SCK  52
#define TFT_MOSI 51
#define TFT_MISO 50
#define TFT_CS   40
#define TFT_DC   38
#define TFT_RST  49

/* Initialize the ST7796S Driver */
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);
Arduino_GFX *gfx = new Arduino_ST7796(bus, TFT_RST, 1 /* rotation */, true /* IPS */);

void setup() {
  Serial.begin(115200);
  Serial.println("\n====================================");
  Serial.println("   OxyFeeder 4.0\" TFT Test");
  Serial.println("====================================\n");

  Serial.println("Initializing ST7796S display...");

  if (!gfx->begin()) {
    Serial.println("ERROR: gfx->begin() failed!");
    Serial.println("\nCheck:");
    Serial.println("  - Wiring connections");
    Serial.println("  - Logic level shifter");
    Serial.println("  - 3.3V to LED pin (backlight)");
    while(1);
  }

  Serial.println("Display initialized OK!");

  // Clear screen to black
  gfx->fillScreen(BLACK);

  // Header
  gfx->setTextColor(CYAN);
  gfx->setTextSize(3);
  gfx->setCursor(20, 20);
  gfx->println("OXYFEEDER v1.0");

  gfx->drawFastHLine(0, 60, 480, WHITE);

  // System check
  gfx->setTextSize(2);
  gfx->setCursor(20, 80);
  gfx->setTextColor(YELLOW);
  gfx->println("SYSTEM CHECK:");

  gfx->setCursor(20, 120);
  gfx->setTextColor(GREEN);
  gfx->println("ST7796S Driver: OK");
  gfx->println("Resolution: 480x320");

  Serial.println("\nYou should see:");
  Serial.println("  - Cyan 'OXYFEEDER v1.0' header");
  Serial.println("  - White horizontal line");
  Serial.println("  - Yellow 'SYSTEM CHECK:'");
  Serial.println("  - Green status text");
  Serial.println("  - White uptime counter");
  Serial.println("\nPress any key to show sensor demo...");
}

int testNum = 0;

void loop() {
  // Update uptime counter
  gfx->fillRect(120, 200, 150, 30, BLACK);
  gfx->setCursor(20, 200);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  gfx->print("Uptime: ");
  gfx->print(millis() / 1000);
  gfx->print("s");

  // Check for key press to change display
  if (Serial.available()) {
    Serial.read();
    testNum++;
    showTestScreen(testNum % 3);
  }

  delay(1000);
}

void showTestScreen(int test) {
  gfx->fillScreen(BLACK);

  switch(test) {
    case 0:
      // Main OxyFeeder display
      drawMainScreen();
      Serial.println("Test 1: Main dashboard");
      break;

    case 1:
      // Sensor readings
      drawSensorScreen();
      Serial.println("Test 2: Sensor readings");
      break;

    case 2:
      // Alert screen
      drawAlertScreen();
      Serial.println("Test 3: Alert screen");
      break;
  }
}

void drawMainScreen() {
  // Header
  gfx->setTextColor(CYAN);
  gfx->setTextSize(3);
  gfx->setCursor(20, 20);
  gfx->println("OXYFEEDER v1.0");
  gfx->drawFastHLine(0, 60, 480, WHITE);

  // Sensor boxes
  gfx->drawRect(10, 80, 150, 100, CYAN);
  gfx->setCursor(20, 90);
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  gfx->println("DO Level");
  gfx->setTextSize(3);
  gfx->setTextColor(GREEN);
  gfx->setCursor(30, 130);
  gfx->println("6.5");
  gfx->setTextSize(2);
  gfx->println("   mg/L");

  gfx->drawRect(170, 80, 150, 100, YELLOW);
  gfx->setCursor(180, 90);
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  gfx->println("Feed");
  gfx->setTextSize(3);
  gfx->setTextColor(GREEN);
  gfx->setCursor(200, 130);
  gfx->println("75%");

  gfx->drawRect(330, 80, 140, 100, GREEN);
  gfx->setCursor(340, 90);
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  gfx->println("Battery");
  gfx->setTextSize(3);
  gfx->setTextColor(GREEN);
  gfx->setCursor(350, 130);
  gfx->println("12.8V");
}

void drawSensorScreen() {
  gfx->setTextColor(YELLOW);
  gfx->setTextSize(3);
  gfx->setCursor(20, 20);
  gfx->println("SENSOR READINGS");
  gfx->drawFastHLine(0, 60, 480, WHITE);

  gfx->setTextSize(2);
  gfx->setCursor(20, 80);
  gfx->setTextColor(CYAN);
  gfx->print("Dissolved O2: ");
  gfx->setTextColor(GREEN);
  gfx->println("6.5 mg/L");

  gfx->setCursor(20, 120);
  gfx->setTextColor(CYAN);
  gfx->print("Feed Level:   ");
  gfx->setTextColor(GREEN);
  gfx->println("75%");

  gfx->setCursor(20, 160);
  gfx->setTextColor(CYAN);
  gfx->print("Battery:      ");
  gfx->setTextColor(GREEN);
  gfx->println("12.8V");

  gfx->setCursor(20, 200);
  gfx->setTextColor(CYAN);
  gfx->print("Water Temp:   ");
  gfx->setTextColor(GREEN);
  gfx->println("28.5 C");

  gfx->setCursor(20, 240);
  gfx->setTextColor(CYAN);
  gfx->print("Last Feed:    ");
  gfx->setTextColor(WHITE);
  gfx->println("08:30 AM");
}

void drawAlertScreen() {
  gfx->fillScreen(RED);

  gfx->setTextColor(WHITE);
  gfx->setTextSize(4);
  gfx->setCursor(100, 40);
  gfx->println("!! ALERT !!");

  gfx->setTextSize(3);
  gfx->setCursor(40, 120);
  gfx->println("LOW OXYGEN LEVEL");

  gfx->setTextSize(4);
  gfx->setCursor(140, 180);
  gfx->setTextColor(YELLOW);
  gfx->println("3.2 mg/L");

  gfx->setTextSize(2);
  gfx->setCursor(80, 260);
  gfx->setTextColor(WHITE);
  gfx->println("SMS Alert Sent!");
}
