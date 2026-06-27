/*
 * =============================================================================
 * OxyFeeder Camera Server - ESP32-CAM Video Streaming
 * =============================================================================
 *
 * Description:
 *   This firmware runs on an ESP32-CAM module (AI-Thinker model) and provides
 *   a simple video streaming server for remote fishpond monitoring. The camera
 *   connects to your WiFi network and streams video via HTTP that can be
 *   viewed in a web browser or integrated into the OxyFeeder Flutter app.
 *
 * WiFi Setup (WiFiManager - no hardcoding needed!):
 *   1. On first boot, ESP32-CAM creates a hotspot: "OxyFeeder-CAM"
 *   2. Connect your phone to this hotspot (password: oxyfeeder123)
 *   3. A config page opens automatically (or go to 192.168.4.1)
 *   4. Select your WiFi network and enter password
 *   5. ESP32-CAM saves credentials and connects automatically
 *   6. To change WiFi later, visit http://<camera-ip>/reset
 *
 * Board: AI-Thinker ESP32-CAM
 * Library Required: WiFiManager by tzapu (install via Library Manager)
 *
 * =============================================================================
 * WIRING DIAGRAM: ESP32-CAM to FTDI Programmer (for uploading code)
 * =============================================================================
 *
 *   ESP32-CAM          FTDI Programmer
 *   ---------          ---------------
 *   5V       <-------> 5V (or VCC)
 *   GND      <-------> GND
 *   U0R (RX) <-------> TX
 *   U0T (TX) <-------> RX
 *   IO0      <-------> GND  (ONLY during upload! Remove after flashing)
 *
 * Upload Steps:
 *   1. Connect wires as shown above (including IO0 to GND)
 *   2. In Arduino IDE: Tools > Board > "AI Thinker ESP32-CAM"
 *   3. Select correct COM port
 *   4. Press the RESET button on ESP32-CAM, then click Upload
 *   5. Wait for "Connecting..." then release RESET if needed
 *   6. After upload completes, DISCONNECT IO0 from GND
 *   7. Press RESET again to run the program
 *   8. Open Serial Monitor (115200 baud) to see setup instructions
 *
 * Note: The ESP32-CAM has no built-in USB. You MUST use an external
 *       FTDI adapter (USB-to-Serial) to upload code.
 *
 * =============================================================================
 */

#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <Preferences.h>  // ESP32 non-volatile storage
#include "esp_http_server.h"

// =============================================================================
// WIFI CONFIGURATION - Dynamic, no hardcoding!
// =============================================================================
// Works like a phone — connects to any WiFi, remembers it, switches anytime.
//
// First time / new location:
//   1. Camera creates hotspot: "OxyFeeder-CAM" (pass: oxyfeeder123)
//   2. Connect phone to OxyFeeder-CAM
//   3. Open browser → 192.168.4.1 → select WiFi → enter password → done
//   4. Camera saves and connects. Next boot = auto connects.
//
// Change WiFi later (without hotspot):
//   - While connected: open browser → http://oxyfeeder-cam.local/wifi
//   - Enter new WiFi name + password → camera restarts on new network
//
// Camera always reachable at: http://oxyfeeder-cam.local/stream
// (no need to know the IP — hostname never changes)
// =============================================================================

#define AP_NAME "OxyFeeder-CAM"           // Hotspot name for setup
#define AP_PASSWORD "oxyfeeder123"        // Hotspot password (min 8 chars)
#define CONFIG_TIMEOUT 180                // Seconds before config portal times out

// Stream settings
#define STREAM_PORT 80                          // HTTP port for video stream
#define FRAME_SIZE FRAMESIZE_VGA               // Resolution: VGA (640x480)
                                                // Options: FRAMESIZE_QVGA (320x240)
                                                //          FRAMESIZE_VGA (640x480)
                                                //          FRAMESIZE_SVGA (800x600)
                                                //          FRAMESIZE_XGA (1024x768)
                                                // Lower = faster, less bandwidth

// =============================================================================
// AI-THINKER ESP32-CAM PIN DEFINITIONS (DO NOT CHANGE)
// =============================================================================

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Onboard LED (Flash)
#define LED_GPIO_NUM       4

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

httpd_handle_t stream_httpd = NULL;
Preferences prefs;
WiFiUDP udpBroadcast;
#define DISCOVERY_PORT 5556

// MIME type boundary for MJPEG stream
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// =============================================================================
// CAMERA INITIALIZATION
// =============================================================================

bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;

  // Frame size and quality settings
  // Higher quality = larger files = more bandwidth
  if (psramFound()) {
    config.frame_size = FRAME_SIZE;
    config.jpeg_quality = 12;  // 0-63, lower = better quality
    config.fb_count = 2;       // Double buffer for smoother streaming
    Serial.println("PSRAM found - using higher quality settings");
  } else {
    config.frame_size = FRAMESIZE_QVGA;  // Fallback to lower res without PSRAM
    config.jpeg_quality = 15;
    config.fb_count = 1;
    Serial.println("No PSRAM - using lower quality settings");
  }

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return false;
  }

  // Optional: Adjust camera sensor settings for better image
  sensor_t * s = esp_camera_sensor_get();
  if (s != NULL) {
    s->set_brightness(s, 0);     // -2 to 2
    s->set_contrast(s, 0);       // -2 to 2
    s->set_saturation(s, 0);     // -2 to 2
    s->set_whitebal(s, 1);       // 0 = disable, 1 = enable
    s->set_awb_gain(s, 1);       // 0 = disable, 1 = enable
    s->set_wb_mode(s, 0);        // 0 to 4 - white balance mode
    s->set_exposure_ctrl(s, 1);  // 0 = disable, 1 = enable
    s->set_aec2(s, 0);           // 0 = disable, 1 = enable
    s->set_gain_ctrl(s, 1);      // 0 = disable, 1 = enable
    s->set_agc_gain(s, 0);       // 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
    s->set_bpc(s, 0);            // 0 = disable, 1 = enable
    s->set_wpc(s, 1);            // 0 = disable, 1 = enable
    s->set_raw_gma(s, 1);        // 0 = disable, 1 = enable
    s->set_lenc(s, 1);           // 0 = disable, 1 = enable
    s->set_hmirror(s, 0);        // 0 = disable, 1 = enable (flip horizontal)
    s->set_vflip(s, 0);          // 0 = disable, 1 = enable (flip vertical)
  }

  Serial.println("Camera initialized successfully!");
  return true;
}

// =============================================================================
// WIFI CONNECTION - Using WiFiManager
// =============================================================================

WiFiManager wifiManager;

// Callback when entering config mode (AP mode)
void configModeCallback(WiFiManager *myWiFiManager) {
  Serial.println();
  Serial.println("===========================================");
  Serial.println("  WIFI SETUP MODE");
  Serial.println("===========================================");
  Serial.println();
  Serial.println("Could not connect to saved WiFi.");
  Serial.println("Starting configuration portal...");
  Serial.println();
  Serial.println("Connect your phone to:");
  Serial.printf("  SSID: %s\n", AP_NAME);
  Serial.printf("  Password: %s\n", AP_PASSWORD);
  Serial.println();
  Serial.println("Then open browser - config page will appear.");
  Serial.println("Select your WiFi and enter password.");
  Serial.println();
  Serial.println("===========================================");

  // Blink LED slowly to indicate config mode
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_GPIO_NUM, HIGH);
    delay(300);
    digitalWrite(LED_GPIO_NUM, LOW);
    delay(300);
  }
}

// Try connecting to a specific SSID/password, returns true if connected within 10s
bool tryConnect(const char* ssid, const char* password) {
  if (strlen(ssid) == 0) return false;
  Serial.printf("Trying WiFi: %s\n", ssid);
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  return WiFi.status() == WL_CONNECTED;
}

bool connectWiFi() {
  Serial.println();
  Serial.println("===========================================");
  Serial.println("OxyFeeder Camera Server - WiFi Setup");
  Serial.println("===========================================");

  WiFi.setSleep(false);

  // 1. Try user-saved credentials from Preferences (set via /wifi page)
  prefs.begin("wifi", true);  // read-only
  String savedSSID = prefs.getString("ssid", "");
  String savedPass = prefs.getString("pass", "");
  prefs.end();

  // WiFiManager handles everything — tries saved credentials first,
  // then opens OxyFeeder-CAM hotspot for reconfiguration if needed
  // Fall back to WiFiManager config portal
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setConfigPortalTimeout(CONFIG_TIMEOUT);
  if (!wifiManager.autoConnect(AP_NAME, AP_PASSWORD)) {
    Serial.println("\nConfig portal timed out! Restarting...");
    delay(3000);
    ESP.restart();
    return false;
  }

connected:

  // Start mDNS — camera always reachable at http://oxyfeeder-cam.local/stream
  if (MDNS.begin("oxyfeeder-cam")) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("mDNS started: http://oxyfeeder-cam.local/stream");
  } else {
    Serial.println("mDNS failed - use IP address instead");
  }

  Serial.println("\n");
  Serial.println("===========================================");
  Serial.println("       WiFi Connected Successfully!        ");
  Serial.println("===========================================");
  Serial.println();
  Serial.println("Camera Stream URLs:");
  Serial.println("  http://oxyfeeder-cam.local/stream  (hostname - works anywhere)");
  Serial.println();
  Serial.print("   http://");
  Serial.print(WiFi.localIP());
  Serial.println("/stream");
  Serial.println();
  Serial.println("Enter this URL in your browser or Flutter app");
  Serial.println("to view the live camera feed.");
  Serial.println();
  Serial.println("Tip: To change WiFi, hold RESET for 10 sec");
  Serial.println("     or reflash to clear saved credentials.");
  Serial.println();
  Serial.println("===========================================");

  return true;
}

// =============================================================================
// HTTP STREAM HANDLER
// =============================================================================

// Handler for the MJPEG video stream
static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  char *part_buf[64];

  // Set response type to multipart stream
  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) {
    return res;
  }

  // Disable caching
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  Serial.println("Client connected to stream");

  while (true) {
    // Capture frame
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
      break;
    }

    // Send boundary
    size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, fb->len);
    res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    if (res != ESP_OK) {
      esp_camera_fb_return(fb);
      break;
    }

    // Send content type and length header
    res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    if (res != ESP_OK) {
      esp_camera_fb_return(fb);
      break;
    }

    // Send JPEG image data
    res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
    if (res != ESP_OK) {
      esp_camera_fb_return(fb);
      break;
    }

    // Return frame buffer to be reused
    esp_camera_fb_return(fb);

    // Small delay to control frame rate (optional)
    // delay(10);
  }

  Serial.println("Client disconnected from stream");
  return res;
}

// Handler for root URL - shows simple HTML page with stream
static esp_err_t index_handler(httpd_req_t *req) {
  const char* html =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<title>OxyFeeder Camera</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>"
    "body { font-family: Arial; text-align: center; background: #1a1a2e; color: #eee; margin: 0; padding: 20px; }"
    "h1 { color: #00d9ff; }"
    "img { width: 100%; height: auto; object-fit: contain; border: 2px solid #00d9ff; border-radius: 8px; }"
    ".info { margin: 20px 0; padding: 15px; background: #16213e; border-radius: 8px; }"
    ".btn { display: inline-block; margin: 10px; padding: 10px 20px; background: #e74c3c; color: white; "
    "text-decoration: none; border-radius: 5px; font-size: 14px; }"
    ".btn:hover { background: #c0392b; }"
    "</style>"
    "</head>"
    "<body>"
    "<h1>OxyFeeder Camera</h1>"
    "<div class='info'>Live Fishpond Monitoring</div>"
    "<img src='/stream' />"
    "<div class='info'>"
    "Stream URL: <code>/stream</code><br><br>"
    "<a href='/wifi' class='btn' style='background:#00d9ff;color:#000;'>Change WiFi</a>"
    "<a href='/reset' class='btn' onclick=\"return confirm('Reset WiFi settings?');\">Reset WiFi</a>"
    "</div>"
    "</body>"
    "</html>";

  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, html, strlen(html));
}

// Handler for WiFi reset - clears saved credentials and restarts
static esp_err_t reset_handler(httpd_req_t *req) {
  const char* html =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<title>WiFi Reset</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>"
    "body { font-family: Arial; text-align: center; background: #1a1a2e; color: #eee; padding: 50px; }"
    "h1 { color: #e74c3c; }"
    ".info { margin: 20px; padding: 20px; background: #16213e; border-radius: 8px; }"
    "</style>"
    "</head>"
    "<body>"
    "<h1>WiFi Settings Reset!</h1>"
    "<div class='info'>"
    "<p>Saved WiFi credentials have been cleared.</p>"
    "<p>Device is restarting...</p>"
    "<p>Connect to <strong>OxyFeeder-CAM</strong> hotspot to reconfigure.</p>"
    "</div>"
    "</body>"
    "</html>";

  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, html, strlen(html));

  Serial.println();
  Serial.println("===========================================");
  Serial.println("  WiFi RESET requested via web interface");
  Serial.println("===========================================");
  Serial.println("Clearing saved credentials and restarting...");

  delay(1000);

  // Clear WiFi settings
  wifiManager.resetSettings();

  delay(1000);

  // Restart ESP32
  ESP.restart();

  return ESP_OK;
}

// Handler for GET /wifi — shows the WiFi config form
static esp_err_t wifi_get_handler(httpd_req_t *req) {
  prefs.begin("wifi", true);
  String currentSSID = prefs.getString("ssid", "");
  prefs.end();

  char html[1200];
  snprintf(html, sizeof(html),
    "<!DOCTYPE html>"
    "<html><head><title>Camera WiFi</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>"
    "body{font-family:Arial;text-align:center;background:#1a1a2e;color:#eee;padding:30px;}"
    "h1{color:#00d9ff;} .card{background:#16213e;padding:20px;border-radius:8px;margin:20px auto;max-width:360px;}"
    "input{width:90%%;padding:10px;margin:8px 0;border-radius:5px;border:1px solid #00d9ff;background:#0f3460;color:#eee;font-size:16px;}"
    ".btn{padding:12px 30px;background:#00d9ff;color:#000;border:none;border-radius:5px;font-size:16px;cursor:pointer;width:96%%;margin-top:8px;}"
    ".note{font-size:12px;color:#aaa;margin-top:10px;}"
    "</style></head><body>"
    "<h1>Camera WiFi Setup</h1>"
    "<div class='card'>"
    "<p>Current: <strong>%s</strong></p>"
    "<form method='POST' action='/wifi'>"
    "<input name='ssid' placeholder='WiFi Name (SSID)' required><br>"
    "<input name='pass' type='password' placeholder='Password'><br>"
    "<input type='submit' class='btn' value='Save & Restart'>"
    "</form>"
    "<p class='note'>Camera will restart and connect to the new WiFi.<br>"
    "If it fails, it will try TestWifi then start OxyFeeder-CAM hotspot.</p>"
    "</div></body></html>",
    currentSSID.length() > 0 ? currentSSID.c_str() : "None saved"
  );

  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, html, strlen(html));
}

// Handler for POST /wifi — saves credentials and restarts
static esp_err_t wifi_post_handler(httpd_req_t *req) {
  char body[256] = {0};
  int received = httpd_req_recv(req, body, sizeof(body) - 1);
  if (received <= 0) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }
  body[received] = '\0';

  // Parse ssid= and pass= from URL-encoded body
  char ssid[64] = {0};
  char pass[64] = {0};

  // Simple URL-decode helper inline
  auto urlDecode = [](const char* src, char* dst, int maxLen) {
    int i = 0, j = 0;
    while (src[i] && j < maxLen - 1) {
      if (src[i] == '%' && src[i+1] && src[i+2]) {
        char hex[3] = {src[i+1], src[i+2], 0};
        dst[j++] = (char)strtol(hex, nullptr, 16);
        i += 3;
      } else if (src[i] == '+') {
        dst[j++] = ' ';
        i++;
      } else {
        dst[j++] = src[i++];
      }
    }
    dst[j] = '\0';
  };

  // Extract ssid value
  char* ssidPtr = strstr(body, "ssid=");
  if (ssidPtr) {
    ssidPtr += 5;
    char raw[64] = {0};
    int k = 0;
    while (ssidPtr[k] && ssidPtr[k] != '&' && k < 63) { raw[k] = ssidPtr[k]; k++; }
    urlDecode(raw, ssid, sizeof(ssid));
  }

  // Extract pass value
  char* passPtr = strstr(body, "pass=");
  if (passPtr) {
    passPtr += 5;
    char raw[64] = {0};
    int k = 0;
    while (passPtr[k] && passPtr[k] != '&' && k < 63) { raw[k] = passPtr[k]; k++; }
    urlDecode(raw, pass, sizeof(pass));
  }

  Serial.printf("WiFi config received — SSID: %s\n", ssid);

  // Save to Preferences
  prefs.begin("wifi", false);
  prefs.putString("ssid", ssid);
  prefs.putString("pass", pass);
  prefs.end();

  const char* html =
    "<!DOCTYPE html><html><head><title>Saved</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>body{font-family:Arial;text-align:center;background:#1a1a2e;color:#eee;padding:50px;}"
    "h1{color:#00d9ff;}.card{background:#16213e;padding:20px;border-radius:8px;}</style></head>"
    "<body><h1>WiFi Saved!</h1>"
    "<div class='card'><p>Restarting and connecting to new WiFi...</p>"
    "<p>Check the IP in the app once it connects.</p></div></body></html>";

  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, html, strlen(html));

  delay(1500);
  ESP.restart();
  return ESP_OK;
}

// =============================================================================
// WEB SERVER SETUP
// =============================================================================

void startStreamServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = STREAM_PORT;
  config.ctrl_port = STREAM_PORT;
  config.max_uri_handlers = 8;

  // Register URI handlers
  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t reset_uri = {
    .uri       = "/reset",
    .method    = HTTP_GET,
    .handler   = reset_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t wifi_get_uri = {
    .uri       = "/wifi",
    .method    = HTTP_GET,
    .handler   = wifi_get_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t wifi_post_uri = {
    .uri       = "/wifi",
    .method    = HTTP_POST,
    .handler   = wifi_post_handler,
    .user_ctx  = NULL
  };

  Serial.printf("Starting web server on port %d\n", config.server_port);

  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &index_uri);
    httpd_register_uri_handler(stream_httpd, &stream_uri);
    httpd_register_uri_handler(stream_httpd, &reset_uri);
    httpd_register_uri_handler(stream_httpd, &wifi_get_uri);
    httpd_register_uri_handler(stream_httpd, &wifi_post_uri);
    Serial.println("Web server started successfully!");
    Serial.println("  /        - Camera page with stream");
    Serial.println("  /stream  - Raw MJPEG stream");
    Serial.println("  /reset   - Clear WiFi and reconfigure");
    Serial.println("  /wifi    - Change WiFi credentials");
  } else {
    Serial.println("Error starting web server!");
  }
}

// =============================================================================
// SETUP
// =============================================================================

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(1000);

  Serial.println();
  Serial.println("===========================================");
  Serial.println("    OxyFeeder ESP32-CAM Camera Server     ");
  Serial.println("===========================================");
  Serial.println();

  // Initialize onboard LED (flash)
  pinMode(LED_GPIO_NUM, OUTPUT);
  digitalWrite(LED_GPIO_NUM, LOW);  // Turn off flash

  // Initialize camera
  Serial.println("Initializing camera...");
  if (!initCamera()) {
    Serial.println("FATAL: Camera initialization failed!");
    Serial.println("Check camera module connection and restart.");
    while (true) {
      // Blink LED to indicate error
      digitalWrite(LED_GPIO_NUM, HIGH);
      delay(500);
      digitalWrite(LED_GPIO_NUM, LOW);
      delay(500);
    }
  }

  // Connect to WiFi
  if (!connectWiFi()) {
    Serial.println("FATAL: WiFi connection failed!");
    Serial.println("Check SSID/PASSWORD and restart.");
    while (true) {
      // Rapid blink to indicate WiFi error
      digitalWrite(LED_GPIO_NUM, HIGH);
      delay(100);
      digitalWrite(LED_GPIO_NUM, LOW);
      delay(100);
    }
  }

  // Start UDP discovery broadcast
  udpBroadcast.begin(DISCOVERY_PORT);

  // Start mDNS — camera reachable at http://oxyfeeder-cam.local/stream
  if (MDNS.begin("oxyfeeder-cam")) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("mDNS started: http://oxyfeeder-cam.local/stream");
  }

  // Start the streaming web server
  startStreamServer();

  // Quick flash to indicate successful startup
  digitalWrite(LED_GPIO_NUM, HIGH);
  delay(200);
  digitalWrite(LED_GPIO_NUM, LOW);

  Serial.println();
  Serial.println("Setup complete! Camera is streaming.");
  Serial.println();
}

// =============================================================================
// LOOP
// =============================================================================

void loop() {
  // The HTTP server runs in background tasks
  // Nothing needed in main loop for basic streaming

  // Serial command handler
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "RESET_WIFI") {
      Serial.println("Resetting WiFi credentials...");
      wifiManager.resetSettings();
      delay(1000);
      ESP.restart();
    } else if (cmd == "IP") {
      Serial.print("Camera IP: http://");
      Serial.print(WiFi.localIP());
      Serial.println("/stream");
    }
  }

  // Broadcast IP via UDP so app can auto-discover camera
  static unsigned long lastBroadcastTime = 0;
  if (millis() - lastBroadcastTime > 5000) {
    lastBroadcastTime = millis();
    IPAddress myIP = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP() : WiFi.softAPIP();
    String msg = "OXYFEEDER_CAM:" + myIP.toString();
    // Broadcast to subnet (works in both STA and AP mode)
    IPAddress broadcast(myIP[0], myIP[1], myIP[2], 255);
    udpBroadcast.beginPacket(broadcast, DISCOVERY_PORT);
    udpBroadcast.print(msg);
    udpBroadcast.endPacket();
  }

  // Optional: Print status periodically
  static unsigned long lastStatusTime = 0;
  if (millis() - lastStatusTime > 30000) {  // Every 30 seconds
    lastStatusTime = millis();
    Serial.printf("Status: WiFi %s | IP: %s | Free heap: %d bytes\n",
      WiFi.status() == WL_CONNECTED ? "OK" : "DISCONNECTED",
      WiFi.localIP().toString().c_str(),
      ESP.getFreeHeap()
    );

    // Reconnect WiFi if disconnected
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected! Attempting reconnect...");
      WiFi.reconnect();
    }
  }

  delay(10);
}
