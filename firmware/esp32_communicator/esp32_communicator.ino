/*
  ESP32 Communicator - BLE Bridge for OxyFeeder

  This ESP32 firmware acts as a communication bridge between the Arduino Mega
  and mobile devices. It receives JSON data from the Arduino via Serial2 and
  broadcasts it over Bluetooth Low Energy (BLE) to the mobile app.

  It also RECEIVES commands from the app and forwards them to Arduino.
  Schedule management: ESP32 stores feeding schedules from the app and
  triggers feeds autonomously via GPIO pulse at the scheduled times.

  Hardware Setup:
  - ESP32 receives data from Arduino Mega via Serial2 (hardware serial)
  - ESP32 broadcasts data over BLE to mobile devices
  - ESP32 receives commands from app via BLE and forwards to Arduino
  - USB Serial available for debugging

  Data Flow:
    Arduino (Serial) -> ESP32 (Serial2 RX=GPIO26) -> BLE -> Mobile App (sensor data)
    Mobile App -> BLE -> ESP32 -> GPIO pulse -> Arduino (commands)
*/

// ----------------------------------------------------------------------------
// 1) Include Libraries for BLE
// ----------------------------------------------------------------------------

#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"

// ----------------------------------------------------------------------------
// 2) BLE Service and Characteristic UUIDs
// ----------------------------------------------------------------------------

// Define unique UUIDs for our OxyFeeder BLE service
#define SERVICE_UUID            "0000abcd-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID     "0000abce-0000-1000-8000-00805f9b34fb"  // For sending data to app
#define COMMAND_CHAR_UUID       "0000abcf-0000-1000-8000-00805f9b34fb"  // For receiving commands from app

// ----------------------------------------------------------------------------
// 3) Global Variables
// ----------------------------------------------------------------------------

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;      // Data to app (notify)
BLECharacteristic* pCommandCharacteristic = NULL; // Commands from app (write)
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Buffer for receiving JSON data from Arduino
String receivedData = "";
const int MAX_DATA_LENGTH = 200; // Maximum expected JSON string length

// Command GPIO pins (direct wire to Arduino, active HIGH pulse)
#define CMD_FEED_PIN 12  // GPIO12, wire to Arduino Pin 17
#define CMD_SMS_PIN 13   // GPIO13, wire to Arduino Pin 3

// UART TX to Arduino (GPIO22 → Arduino Pin 19 RX1) for sending phone number
#define ARDUINO_TX_PIN 22

// ----------------------------------------------------------------------------
// Schedule Management
// ----------------------------------------------------------------------------

struct FeedSchedule {
  int hour;       // 0-23 (24h format)
  int minute;     // 0-59
  int duration;   // seconds
  bool enabled;
};

#define MAX_SCHEDULES 10
FeedSchedule schedules[MAX_SCHEDULES];
int scheduleCount = 0;

// Time tracking (synced from phone)
bool timeSynced = false;
unsigned long timeSyncMillis = 0;  // millis() when time was synced
int syncHour = 0, syncMinute = 0, syncSecond = 0;

// Feed lock to prevent re-triggering in the same minute
int lastFedHour = -1, lastFedMinute = -1;

// ----------------------------------------------------------------------------
// Threshold Monitoring (synced from app, checked against sensor data)
// ----------------------------------------------------------------------------

// Defaults match Arduino hardcoded values (safety fallback)
float thresholdDO = 4.0;       // mg/L - alert below this
int thresholdFeed = 20;        // % - alert below this
int thresholdBattery = 25;     // % - alert below this

// Alert throttle
unsigned long lastAlertSMS = 0;
const unsigned long ALERT_SMS_COOLDOWN = 300000;  // 5 minutes between alert SMS

// Simple JSON value parser (avoids needing ArduinoJson library)
float parseJsonFloat(String json, String key) {
  String search = "\"" + key + "\":";
  int idx = json.indexOf(search);
  if (idx < 0) {
    search = "\"" + key + "\": ";
    idx = json.indexOf(search);
  }
  if (idx < 0) return -1;
  int start = idx + search.length();
  while (start < (int)json.length() && json.charAt(start) == ' ') start++;
  int end = start;
  while (end < (int)json.length() && (isDigit(json.charAt(end)) || json.charAt(end) == '.')) end++;
  if (end == start) return -1;
  return json.substring(start, end).toFloat();
}

void checkThresholdsFromJSON(String json) {
  float doVal = parseJsonFloat(json, "do");
  float feedVal = parseJsonFloat(json, "feed");
  float batteryVal = parseJsonFloat(json, "battery");

  bool alert = false;
  String reason = "";

  if (doVal > 0 && doVal < thresholdDO) {
    alert = true;
    reason = "Low DO";
  }
  if (feedVal >= 0 && feedVal < thresholdFeed) {
    alert = true;
    if (reason.length() > 0) reason += "+";
    reason += "Low Feed";
  }
  if (batteryVal > 0 && batteryVal < thresholdBattery) {
    alert = true;
    if (reason.length() > 0) reason += "+";
    reason += "Low Battery";
  }

  if (alert) {
    unsigned long now = millis();
    if (now - lastAlertSMS >= ALERT_SMS_COOLDOWN || lastAlertSMS == 0) {
      lastAlertSMS = now;
      Serial.print("THRESHOLD ALERT: ");
      Serial.println(reason);
      // Pulse SMS pin to trigger Arduino SMS
      digitalWrite(CMD_SMS_PIN, HIGH);
      delay(500);
      digitalWrite(CMD_SMS_PIN, LOW);
      Serial.println("Pulsed SMS pin for threshold alert");
    }
  }
}

// Get current time based on sync
void getCurrentTime(int &h, int &m, int &s) {
  if (!timeSynced) { h = -1; m = -1; s = -1; return; }

  unsigned long elapsed = (millis() - timeSyncMillis) / 1000; // seconds since sync
  unsigned long totalSeconds = syncHour * 3600UL + syncMinute * 60UL + syncSecond + elapsed;
  totalSeconds %= 86400UL; // wrap at 24 hours

  h = totalSeconds / 3600;
  m = (totalSeconds % 3600) / 60;
  s = totalSeconds % 60;
}

// Parse time label like "08:00 AM" or "05:30 PM" into 24h format
bool parseTimeLabel(String label, int &hour, int &minute) {
  // Format: "HH:MM AM" or "HH:MM PM"
  int colonIdx = label.indexOf(':');
  if (colonIdx < 0) return false;

  hour = label.substring(0, colonIdx).toInt();
  minute = label.substring(colonIdx + 1, colonIdx + 3).toInt();

  // Check for AM/PM
  label.toUpperCase();
  if (label.indexOf("PM") >= 0 && hour != 12) hour += 12;
  if (label.indexOf("AM") >= 0 && hour == 12) hour = 0;

  return (hour >= 0 && hour < 24 && minute >= 0 && minute < 60);
}

// Process schedule-related commands from app
void processScheduleCommand(String cmd) {
  if (cmd.startsWith("SYNC_TIME:")) {
    // Format: SYNC_TIME:HH:MM:SS
    String timeStr = cmd.substring(10);
    int c1 = timeStr.indexOf(':');
    int c2 = timeStr.indexOf(':', c1 + 1);
    if (c1 > 0 && c2 > 0) {
      syncHour = timeStr.substring(0, c1).toInt();
      syncMinute = timeStr.substring(c1 + 1, c2).toInt();
      syncSecond = timeStr.substring(c2 + 1).toInt();
      timeSyncMillis = millis();
      timeSynced = true;
      Serial.print("Time synced: ");
      Serial.print(syncHour); Serial.print(":");
      Serial.print(syncMinute); Serial.print(":");
      Serial.println(syncSecond);
    }
  }
  else if (cmd.startsWith("CLEAR_SCHEDULES")) {
    scheduleCount = 0;
    lastFedHour = -1;
    lastFedMinute = -1;
    Serial.println("All schedules cleared");
  }
  else if (cmd.startsWith("SCHEDULE:")) {
    // Format: SCHEDULE:HH:MM AM/PM,duration,enabled
    // e.g., SCHEDULE:08:00 AM,5,1
    if (scheduleCount >= MAX_SCHEDULES) {
      Serial.println("Max schedules reached");
      return;
    }

    String data = cmd.substring(9); // after "SCHEDULE:"
    int comma1 = data.indexOf(',');
    int comma2 = data.indexOf(',', comma1 + 1);

    if (comma1 < 0 || comma2 < 0) {
      Serial.println("Invalid schedule format");
      return;
    }

    String timeLabel = data.substring(0, comma1);
    int duration = data.substring(comma1 + 1, comma2).toInt();
    int enabled = data.substring(comma2 + 1).toInt();

    int hour, minute;
    if (!parseTimeLabel(timeLabel, hour, minute)) {
      Serial.println("Invalid time in schedule");
      return;
    }

    schedules[scheduleCount].hour = hour;
    schedules[scheduleCount].minute = minute;
    schedules[scheduleCount].duration = duration;
    schedules[scheduleCount].enabled = (enabled == 1);
    scheduleCount++;

    Serial.print("Schedule added: ");
    Serial.print(hour); Serial.print(":"); Serial.print(minute);
    Serial.print(" dur="); Serial.print(duration);
    Serial.print(" en="); Serial.println(enabled);
  }
  else if (cmd.startsWith("THRESHOLD:")) {
    // Format: THRESHOLD:DO,4.0 or THRESHOLD:FEED,55 or THRESHOLD:BATTERY,30
    String data = cmd.substring(10);
    int comma = data.indexOf(',');
    if (comma < 0) {
      Serial.println("Invalid threshold format");
      return;
    }
    String type = data.substring(0, comma);
    String val = data.substring(comma + 1);

    if (type == "DO") {
      thresholdDO = val.toFloat();
      Serial.print("Threshold DO set: "); Serial.println(thresholdDO);
    } else if (type == "FEED") {
      thresholdFeed = val.toInt();
      Serial.print("Threshold Feed set: "); Serial.println(thresholdFeed);
    } else if (type == "BATTERY") {
      thresholdBattery = val.toInt();
      Serial.print("Threshold Battery set: "); Serial.println(thresholdBattery);
    }
  }
}

// Check if it's time to feed
void checkSchedules() {
  if (!timeSynced || scheduleCount == 0) return;

  int h, m, s;
  getCurrentTime(h, m, s);
  if (h < 0) return;

  for (int i = 0; i < scheduleCount; i++) {
    if (!schedules[i].enabled) continue;

    if (h == schedules[i].hour && m == schedules[i].minute && s < 10) {
      // Don't re-trigger in same minute
      if (h == lastFedHour && m == lastFedMinute) continue;

      lastFedHour = h;
      lastFedMinute = m;

      Serial.print("SCHEDULED FEED at ");
      Serial.print(h); Serial.print(":"); Serial.println(m);

      // Pulse feed GPIO (same as Feed Now)
      digitalWrite(CMD_FEED_PIN, HIGH);
      delay(500);
      digitalWrite(CMD_FEED_PIN, LOW);
      Serial.println("Pulsed FEED pin HIGH for 500ms (scheduled)");
      break;
    }
  }

  // Reset feed lock after the minute passes
  if (s >= 30) {
    lastFedHour = -1;
    lastFedMinute = -1;
  }
}

// ----------------------------------------------------------------------------
// 4) BLE Callback Classes
// ----------------------------------------------------------------------------

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("BLE Client Connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("BLE Client Disconnected");
    }
};

// Callback for receiving commands from the app
class CommandCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue().c_str();

      if (rxValue.length() > 0) {
        Serial.print("Received command from app: ");
        Serial.println(rxValue);

        // Forward command to Arduino via GPIO pulse
        if (rxValue.startsWith("FEED")) {
          digitalWrite(CMD_FEED_PIN, HIGH);
          delay(500);
          digitalWrite(CMD_FEED_PIN, LOW);
          Serial.println("Pulsed FEED pin HIGH for 500ms");
        } else if (rxValue.startsWith("TEST_SMS")) {
          digitalWrite(CMD_SMS_PIN, HIGH);
          delay(500);
          digitalWrite(CMD_SMS_PIN, LOW);
          Serial.println("Pulsed SMS pin HIGH for 500ms");
        } else if (rxValue.startsWith("PHONE:")) {
          // Forward phone number to Arduino via Serial2 TX
          Serial2.println(rxValue);
          Serial.print("Forwarded to Arduino: ");
          Serial.println(rxValue);
        } else if (rxValue.startsWith("SYNC_TIME") || rxValue.startsWith("SCHEDULE") || rxValue.startsWith("CLEAR_SCHEDULES") || rxValue.startsWith("THRESHOLD")) {
          processScheduleCommand(rxValue);
        } else {
          Serial.print("Unknown command: ");
          Serial.println(rxValue);
        }
      }
    }
};

// ----------------------------------------------------------------------------
// 5) setup() - Initialize all systems
// ----------------------------------------------------------------------------

void setup() {
  // Initialize USB Serial for debugging
  Serial.begin(115200);
  delay(500); // Short delay for serial to settle (no blocking wait)

  // Serial2: RX from Arduino (data), TX to Arduino (phone number commands)
  Serial2.begin(9600, SERIAL_8N1, 26, ARDUINO_TX_PIN); // RX=GPIO26, TX=GPIO14

  // Command GPIO pins
  pinMode(CMD_FEED_PIN, OUTPUT);
  pinMode(CMD_SMS_PIN, OUTPUT);
  digitalWrite(CMD_FEED_PIN, LOW);   // Idle LOW (Arduino has 1K pull-down, detects HIGH pulse)
  digitalWrite(CMD_SMS_PIN, LOW);    // Idle LOW (Arduino has 1K pull-down, detects HIGH pulse)

  Serial.println("ESP32 OxyFeeder Communicator v3.0 Starting...");
  Serial.println("Serial2 initialized for Arduino communication");

  // Initialize BLE
  BLEDevice::init("OxyFeeder");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristic for sending data TO app (READ + NOTIFY)
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());

  // Create BLE Characteristic for receiving commands FROM app (WRITE)
  pCommandCharacteristic = pService->createCharacteristic(
                      COMMAND_CHAR_UUID,
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_WRITE_NR
                    );
  pCommandCharacteristic->setCallbacks(new CommandCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();

  Serial.println("BLE Server Started - Advertising as 'OxyFeeder'");
  Serial.println("Data Characteristic: " CHARACTERISTIC_UUID);
  Serial.println("Command Characteristic: " COMMAND_CHAR_UUID);
  Serial.println("Waiting for mobile app connection...");
}

// ----------------------------------------------------------------------------
// 6) loop() - Main communication loop
// ----------------------------------------------------------------------------

void loop() {
  // Debug: print heartbeat every 5 seconds to confirm loop is running
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 5000) {
    lastDebug = millis();
    Serial.print("[DEBUG] Loop alive. Serial2 available: ");
    Serial.print(Serial2.available());
    Serial.print(" | Pin26: ");
    Serial.print(digitalRead(26));

    // Show time and schedule count
    if (timeSynced) {
      int h, m, s;
      getCurrentTime(h, m, s);
      Serial.print(" | Time: ");
      if (h < 10) Serial.print("0");
      Serial.print(h); Serial.print(":");
      if (m < 10) Serial.print("0");
      Serial.print(m); Serial.print(":");
      if (s < 10) Serial.print("0");
      Serial.print(s);
    }
    Serial.print(" | Schedules: ");
    Serial.println(scheduleCount);
  }

  // Check feeding schedules
  checkSchedules();

  // Check for incoming data from Arduino via Serial2
  if (Serial2.available()) {
    char incomingChar = Serial2.read();

    // Build complete JSON string
    if (incomingChar == '\n' || incomingChar == '\r') {
      // End of JSON string received
      if (receivedData.length() > 0) {
        // Only forward lines that start with '{' (JSON data)
        // Pin 1 also sends debug text - filter it out
        if (receivedData.charAt(0) == '{') {
          Serial.print("JSON from Arduino: ");
          Serial.println(receivedData);

          // Check thresholds against synced values from app
          checkThresholdsFromJSON(receivedData);

          // Update BLE characteristic if device is connected
          if (deviceConnected) {
            pCharacteristic->setValue(receivedData.c_str());
            pCharacteristic->notify();
            Serial.println("Data sent to mobile app via BLE");
          } else {
            Serial.println("No BLE client connected - data not sent");
          }
        }

        // Clear buffer for next message
        receivedData = "";
      }
    } else {
      // Add character to buffer (ignore if buffer gets too long)
      if (receivedData.length() < MAX_DATA_LENGTH) {
        receivedData += incomingChar;
      }
    }
  }

  // Handle BLE connection status changes
  if (!deviceConnected && oldDeviceConnected) {
    // Client disconnected - restart advertising
    delay(500);
    pServer->startAdvertising();
    Serial.println("Restarting BLE advertising");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    // Client just connected - wait for real data from Arduino
    delay(1000); // Give client time to set up notifications
    Serial.println("BLE client connected - waiting for real sensor data");
    oldDeviceConnected = deviceConnected;
  }

  // Small delay to prevent overwhelming the system
  delay(10);
}
