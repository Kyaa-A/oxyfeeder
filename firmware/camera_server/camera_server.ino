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
 * Board: AI-Thinker ESP32-CAM
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
 *   8. Open Serial Monitor (115200 baud) to see the IP address
 *
 * Note: The ESP32-CAM has no built-in USB. You MUST use an external
 *       FTDI adapter (USB-to-Serial) to upload code.
 *
 * =============================================================================
 */

#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"

// =============================================================================
// USER CONFIGURATION - Update these values!
// =============================================================================

const char* WIFI_SSID = "ZTE_2.4G_7aNbXv";      // Your WiFi network name
const char* WIFI_PASSWORD = "Adminaly@1";  // Your WiFi password

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
// WIFI CONNECTION
// =============================================================================

bool connectWiFi() {
  Serial.println();
  Serial.println("===========================================");
  Serial.println("OxyFeeder Camera Server - Connecting to WiFi");
  Serial.println("===========================================");
  Serial.printf("SSID: %s\n", WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);  // Disable WiFi sleep for better streaming

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi connection FAILED!");
    Serial.println("Please check your SSID and PASSWORD.");
    return false;
  }

  Serial.println("\n");
  Serial.println("===========================================");
  Serial.println("       WiFi Connected Successfully!        ");
  Serial.println("===========================================");
  Serial.println();
  Serial.println("Camera Stream URL:");
  Serial.println();
  Serial.print("   http://");
  Serial.print(WiFi.localIP());
  Serial.println("/stream");
  Serial.println();
  Serial.println("Enter this URL in your browser or Flutter app");
  Serial.println("to view the live camera feed.");
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
    "img { max-width: 100%; border: 2px solid #00d9ff; border-radius: 8px; }"
    ".info { margin: 20px 0; padding: 15px; background: #16213e; border-radius: 8px; }"
    "</style>"
    "</head>"
    "<body>"
    "<h1>OxyFeeder Camera</h1>"
    "<div class='info'>Live Fishpond Monitoring</div>"
    "<img src='/stream' />"
    "<div class='info'>Stream URL: <code>/stream</code></div>"
    "</body>"
    "</html>";

  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, html, strlen(html));
}

// =============================================================================
// WEB SERVER SETUP
// =============================================================================

void startStreamServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = STREAM_PORT;
  config.ctrl_port = STREAM_PORT;

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

  Serial.printf("Starting web server on port %d\n", config.server_port);

  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &index_uri);
    httpd_register_uri_handler(stream_httpd, &stream_uri);
    Serial.println("Web server started successfully!");
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
