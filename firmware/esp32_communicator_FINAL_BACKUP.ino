/*
  ================================================================================
  ESP32 Communicator - BLE Bridge for OxyFeeder (FINAL VERSION)
  ================================================================================

  Target: ESP32 DevKit (any variant)

  This ESP32 firmware acts as a communication bridge between the Arduino Mega
  and mobile devices. It receives JSON data from Arduino and broadcasts via BLE.
  It can also receive commands from the app and forward them to Arduino.

  ================================================================================
  FEATURES:
  ================================================================================
  - Receives sensor data from Arduino via Serial2
  - Broadcasts data to mobile app via BLE notifications
  - Receives commands from app and forwards to Arduino
  - Connection status monitoring
  - Auto-reconnect and advertising restart

  ================================================================================
  BLE CONFIGURATION:
  ================================================================================
  Device Name: "OxyFeeder"
  Service UUID: 0000abcd-0000-1000-8000-00805f9b34fb
  Data Characteristic UUID: 0000abce-0000-1000-8000-00805f9b34fb (READ, NOTIFY)
  Command Characteristic UUID: 0000abcf-0000-1000-8000-00805f9b34fb (WRITE)

  ================================================================================
  WIRING (Through Level Shifter):
  ================================================================================

  Arduino Mega          Level Shifter (BSS138)      ESP32
  -----------          -------------------         -----
  5V      -----------> HV
  GND     -----------> GND (both sides) <--------- GND
  TX1 (18) ---------> HV1 --> LV1 ---------------> D16 (RX2)
  RX1 (19) <--------- HV2 <-- LV2 <--------------- D17 (TX2)
                       LV <------------------------ 3V3
  5V      ------------------------------------------> VIN (power ESP32)

  ================================================================================
*/

// ============================================================================
// LIBRARY INCLUDES
// ============================================================================

#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"

// ============================================================================
// BLE CONFIGURATION
// ============================================================================

#define DEVICE_NAME         "OxyFeeder"
#define SERVICE_UUID        "0000abcd-0000-1000-8000-00805f9b34fb"
#define CHAR_DATA_UUID      "0000abce-0000-1000-8000-00805f9b34fb"  // For sensor data (READ, NOTIFY)
#define CHAR_COMMAND_UUID   "0000abcf-0000-1000-8000-00805f9b34fb"  // For commands (WRITE)

// ============================================================================
// PIN DEFINITIONS
// ============================================================================

#define RXD2 16  // ESP32 RX2 - receives from Arduino TX1
#define TXD2 17  // ESP32 TX2 - sends to Arduino RX1 (optional, for commands)

// Built-in LED (if available on your board)
#define LED_PIN 2

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

BLEServer* pServer = NULL;
BLECharacteristic* pDataCharacteristic = NULL;
BLECharacteristic* pCommandCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

// Buffer for incoming data from Arduino
String receivedData = "";
const int MAX_DATA_LENGTH = 256;

// Statistics
unsigned long packetsReceived = 0;
unsigned long packetsSent = 0;
unsigned long lastActivityTime = 0;

// ============================================================================
// BLE CALLBACK CLASSES
// ============================================================================

/*
 * Server Callbacks - Handle connection/disconnection events
 */
class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    digitalWrite(LED_PIN, HIGH);  // LED on when connected
    Serial.println("=================================");
    Serial.println("BLE Client CONNECTED");
    Serial.println("=================================");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    digitalWrite(LED_PIN, LOW);  // LED off when disconnected
    Serial.println("=================================");
    Serial.println("BLE Client DISCONNECTED");
    Serial.println("=================================");
  }
};

/*
 * Command Characteristic Callbacks - Handle incoming commands from app
 */
class CommandCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();

    if (value.length() > 0) {
      String command = String(value.c_str());
      command.trim();

      Serial.print("Command received from app: ");
      Serial.println(command);

      // Forward command to Arduino
      Serial2.println(command);
      Serial.println("Command forwarded to Arduino");
    }
  }
};

// ============================================================================
// BLE INITIALIZATION
// ============================================================================

void initBLE() {
  Serial.println("Initializing BLE...");

  // Create BLE Device
  BLEDevice::init(DEVICE_NAME);

  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  // Create BLE Service
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Create Data Characteristic (for sensor data)
  pDataCharacteristic = pService->createCharacteristic(
    CHAR_DATA_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pDataCharacteristic->addDescriptor(new BLE2902());

  // Create Command Characteristic (for receiving commands)
  pCommandCharacteristic = pService->createCharacteristic(
    CHAR_COMMAND_UUID,
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR  // Write without response (faster)
  );
  pCommandCharacteristic->setCallbacks(new CommandCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // Functions that help with iPhone connections
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE initialized successfully!");
  Serial.print("Device name: ");
  Serial.println(DEVICE_NAME);
  Serial.println("Waiting for connections...");
}

// ============================================================================
// SERIAL COMMUNICATION
// ============================================================================

void initSerial() {
  // USB Serial for debugging
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  // Serial2 for Arduino communication
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial.println();
  Serial.println("================================================");
  Serial.println("   ESP32 OxyFeeder Communicator - FINAL         ");
  Serial.println("================================================");
  Serial.println();
  Serial.println("Serial2: RX=" + String(RXD2) + ", TX=" + String(TXD2));
  Serial.println("Baud rate: 9600");
  Serial.println();
}

/*
 * Process data received from Arduino
 * Expects JSON format: {"do": 7.2, "feed": 75, "battery": 85}
 */
void processArduinoData() {
  while (Serial2.available()) {
    char c = Serial2.read();

    if (c == '\n' || c == '\r') {
      // End of message
      if (receivedData.length() > 0) {
        packetsReceived++;
        lastActivityTime = millis();

        Serial.print("[");
        Serial.print(packetsReceived);
        Serial.print("] From Arduino: ");
        Serial.println(receivedData);

        // Send to BLE if connected
        if (deviceConnected) {
          pDataCharacteristic->setValue(receivedData.c_str());
          pDataCharacteristic->notify();
          packetsSent++;
          Serial.println("     --> Sent to app via BLE");
        } else {
          Serial.println("     --> No client connected");
        }

        receivedData = "";
      }
    } else {
      // Build message string
      if (receivedData.length() < MAX_DATA_LENGTH) {
        receivedData += c;
      }
    }
  }
}

// ============================================================================
// CONNECTION MANAGEMENT
// ============================================================================

void handleConnectionChanges() {
  // Handle disconnection - restart advertising
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);  // Give BLE stack time to get ready
    pServer->startAdvertising();
    Serial.println("Restarting BLE advertising...");
    oldDeviceConnected = deviceConnected;
  }

  // Handle new connection
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    Serial.println("New client connected, ready to send data");
  }
}

// ============================================================================
// STATUS MONITORING
// ============================================================================

void printStatus() {
  static unsigned long lastStatusPrint = 0;

  // Print status every 30 seconds
  if (millis() - lastStatusPrint >= 30000) {
    lastStatusPrint = millis();

    Serial.println();
    Serial.println("--- STATUS ---");
    Serial.print("BLE: ");
    Serial.println(deviceConnected ? "Connected" : "Waiting...");
    Serial.print("Packets received from Arduino: ");
    Serial.println(packetsReceived);
    Serial.print("Packets sent to app: ");
    Serial.println(packetsSent);
    Serial.print("Last activity: ");
    Serial.print((millis() - lastActivityTime) / 1000);
    Serial.println(" seconds ago");
    Serial.print("Free heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    Serial.println("--------------");
    Serial.println();
  }
}

// ============================================================================
// LED INDICATOR
// ============================================================================

void updateLED() {
  // Blink pattern when not connected
  if (!deviceConnected) {
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink >= 1000) {
      lastBlink = millis();
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
  }
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize communications
  initSerial();
  initBLE();

  lastActivityTime = millis();

  Serial.println();
  Serial.println("Setup complete! Ready to bridge Arduino <-> BLE");
  Serial.println("================================================");
  Serial.println();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  // Process incoming data from Arduino
  processArduinoData();

  // Handle BLE connection state changes
  handleConnectionChanges();

  // Update LED indicator
  updateLED();

  // Print periodic status
  printStatus();

  // Small delay to prevent CPU hogging
  delay(10);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/*
 * Send a test message to Arduino (for debugging)
 * Call from Serial Monitor: send "TEST" command
 */
void handleSerialCommands() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "TEST") {
      Serial2.println("PING");
      Serial.println("Sent PING to Arduino");
    } else if (cmd == "STATUS") {
      printStatus();
    } else if (cmd == "FEED") {
      Serial2.println("FEED");
      Serial.println("Sent FEED command to Arduino");
    }
  }
}

/*
 * Manual BLE data send (for testing)
 */
void sendTestData() {
  if (deviceConnected) {
    String testData = "{\"do\": 7.5, \"feed\": 80, \"battery\": 90}";
    pDataCharacteristic->setValue(testData.c_str());
    pDataCharacteristic->notify();
    Serial.println("Test data sent via BLE");
  }
}
