/*
 * SIM800L GSM Module Test
 * =======================
 *
 * Tests basic AT commands and SMS functionality
 *
 * Wiring (Arduino Mega):
 *   SIM800L TX  → Arduino RX3 (Pin 15)
 *   SIM800L RX  → Arduino TX3 (Pin 14)
 *   SIM800L GND → GND
 *   SIM800L VCC → 4.2V (NOT 5V directly! Use buck converter or 2x diodes)
 *
 * IMPORTANT: SIM800L needs 4.2V, not 5V!
 *   Option 1: Use LM2596 buck converter set to 4.2V
 *   Option 2: Use 2 diodes in series from 5V (drops ~1.4V)
 *   Option 3: Use dedicated SIM800L power module
 */

// Test phone number - CHANGE THIS to your number!
#define TEST_PHONE_NUMBER "+639550717546"

void setup() {
  // USB Serial for debugging
  Serial.begin(115200);

  // Serial3 for SIM800L (TX3=14, RX3=15)
  Serial3.begin(9600);

  delay(2000);

  Serial.println("====================================");
  Serial.println("   SIM800L GSM Module Test");
  Serial.println("====================================");
  Serial.println("");
  Serial.println("Commands:");
  Serial.println("  1 - Check module (AT)");
  Serial.println("  2 - Check SIM card");
  Serial.println("  3 - Check signal strength");
  Serial.println("  4 - Check network registration");
  Serial.println("  5 - Send test SMS");
  Serial.println("  6 - Check balance (Globe/Smart)");
  Serial.println("");
  Serial.println("Type a number and press Enter...");
  Serial.println("====================================");
}

void loop() {
  // Check for user input from Serial Monitor
  if (Serial.available()) {
    char cmd = Serial.read();

    switch (cmd) {
      case '1':
        Serial.println("\n>> Testing AT command...");
        sendATCommand("AT", 1000);
        break;

      case '2':
        Serial.println("\n>> Checking SIM card...");
        sendATCommand("AT+CPIN?", 1000);
        break;

      case '3':
        Serial.println("\n>> Checking signal strength...");
        sendATCommand("AT+CSQ", 1000);
        Serial.println("(Values: 0-9=Marginal, 10-14=OK, 15-19=Good, 20-30=Excellent)");
        break;

      case '4':
        Serial.println("\n>> Checking network registration...");
        sendATCommand("AT+CREG?", 1000);
        Serial.println("(0,1=Registered Home, 0,5=Registered Roaming)");
        break;

      case '5':
        Serial.println("\n>> Sending test SMS...");
        sendTestSMS();
        break;

      case '6':
        Serial.println("\n>> Checking balance...");
        sendATCommand("AT+CUSD=1,\"*143#\"", 5000);  // Globe
        // sendATCommand("AT+CUSD=1,\"*121#\"", 5000);  // Smart
        break;
    }
  }

  // Forward any response from SIM800L to Serial Monitor
  while (Serial3.available()) {
    char c = Serial3.read();
    Serial.write(c);
  }
}

void sendATCommand(const char* cmd, int timeout) {
  Serial.print("Sending: ");
  Serial.println(cmd);

  Serial3.println(cmd);

  long startTime = millis();
  while (millis() - startTime < timeout) {
    while (Serial3.available()) {
      char c = Serial3.read();
      Serial.write(c);
    }
  }
  Serial.println();
}

void sendTestSMS() {
  Serial.println("Setting SMS to text mode...");
  Serial3.println("AT+CMGF=1");
  delay(1000);

  // Flush response
  while (Serial3.available()) {
    Serial.write(Serial3.read());
  }

  Serial.print("Sending to: ");
  Serial.println(TEST_PHONE_NUMBER);

  // Set recipient
  Serial3.print("AT+CMGS=\"");
  Serial3.print(TEST_PHONE_NUMBER);
  Serial3.println("\"");
  delay(1000);

  // Wait for > prompt
  while (Serial3.available()) {
    Serial.write(Serial3.read());
  }

  // Send message
  Serial3.print("OxyFeeder Test: SMS module is working! - ");
  Serial3.print(millis() / 1000);
  Serial3.println(" seconds uptime");

  // Send Ctrl+Z to send the message
  Serial3.write(26);

  Serial.println("SMS sent! Waiting for confirmation...");

  // Wait for response
  delay(5000);
  while (Serial3.available()) {
    Serial.write(Serial3.read());
  }
  Serial.println();
}
