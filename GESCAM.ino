#include "esp_camera.h"
#include <WiFi.h>

// ============================================================
// WIFI
// ============================================================

const char* ssid = "Toji";
const char* password = "22051977";

// ============================================================
// AI THINKER ESP32-CAM PINOUT
// ============================================================

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

// ============================================================
// BUILT-IN WHITE LED
// ============================================================

#define FLASH_LED_PIN 4

// ============================================================
// TCP SERVERS
// ============================================================

WiFiServer videoServer(81);
WiFiServer controlServer(82);

// ============================================================
// CAMERA SETTINGS
// ============================================================

#define FRAME_SIZE       FRAMESIZE_VGA
#define JPEG_QUALITY     12

// ============================================================
// JPEG BUFFER
// ============================================================

#define JPEG_BUFFER_SIZE 40000

uint8_t jpegBuffer[JPEG_BUFFER_SIZE];

// ============================================================
// LED MODES
// ============================================================

enum LedMode
{
  LED_OFF,
  LED_ON,
  LED_SLOW,
  LED_FAST
};

LedMode ledMode = LED_OFF;

bool ledState = false;

unsigned long lastBlinkTime = 0;

// ============================================================
// SET LED
// ============================================================

void setLED(bool state)
{
  ledState = state;

  digitalWrite(
    FLASH_LED_PIN,
    state ? HIGH : LOW
  );
}

// ============================================================
// UPDATE LED
// ============================================================

void updateLED()
{
  unsigned long now = millis();

  // OFF
  if (ledMode == LED_OFF)
  {
    setLED(false);
    return;
  }

  // ON
  if (ledMode == LED_ON)
  {
    setLED(true);
    return;
  }

  // SLOW BLINK
  if (ledMode == LED_SLOW)
  {
    if (now - lastBlinkTime >= 700)
    {
      lastBlinkTime = now;

      setLED(!ledState);
    }

    return;
  }

  // FAST BLINK
  if (ledMode == LED_FAST)
  {
    if (now - lastBlinkTime >= 150)
    {
      lastBlinkTime = now;

      setLED(!ledState);
    }

    return;
  }
}

// ============================================================
// CAMERA INITIALIZATION
// ============================================================

bool initCamera()
{
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

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

  config.xclk_freq_hz = 10000000;

  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAME_SIZE;

  config.jpeg_quality = JPEG_QUALITY;

  if (psramFound())
  {
    Serial.println("PSRAM detected");

    config.fb_count = 2;

    config.fb_location =
      CAMERA_FB_IN_PSRAM;

    config.grab_mode =
      CAMERA_GRAB_LATEST;
  }
  else
  {
    Serial.println("WARNING: PSRAM NOT DETECTED");

    config.fb_count = 1;

    config.fb_location =
      CAMERA_FB_IN_DRAM;

    config.grab_mode =
      CAMERA_GRAB_WHEN_EMPTY;
  }

  Serial.println();
  Serial.println("==============================================");
  Serial.println("       ESP32-CAM OV3660");
  Serial.println("       GESTURE CONTROL CAMERA");
  Serial.println("==============================================");

  Serial.println("Sensor     : OV3660");
  Serial.println("Resolution : 640 x 480");
  Serial.println("Format     : JPEG");

  Serial.print("JPEG       : Quality ");
  Serial.println(JPEG_QUALITY);

  Serial.println("XCLK       : 10 MHz");
  Serial.println("Buffers    : 2");
  Serial.println("Camera FB  : PSRAM");
  Serial.println("TX Buffer  : 40 KB");

  Serial.println("==============================================");

  Serial.println();
  Serial.println("Initializing OV3660...");

  esp_err_t err =
    esp_camera_init(&config);

  if (err != ESP_OK)
  {
    Serial.print(
      "Camera initialization failed: 0x"
    );

    Serial.println(
      err,
      HEX
    );

    return false;
  }

  sensor_t* sensor =
    esp_camera_sensor_get();

  if (sensor == NULL)
  {
    Serial.println(
      "ERROR: Sensor not found"
    );

    return false;
  }

  Serial.print("Sensor ID: 0x");

  Serial.println(
    sensor->id.PID,
    HEX
  );

  if (sensor->id.PID == OV3660_PID)
  {
    sensor->set_vflip(
      sensor,
      1
    );

    sensor->set_brightness(
      sensor,
      0
    );

    sensor->set_contrast(
      sensor,
      0
    );

    sensor->set_saturation(
      sensor,
      0
    );

    sensor->set_sharpness(
      sensor,
      1
    );

    sensor->set_whitebal(
      sensor,
      1
    );

    sensor->set_exposure_ctrl(
      sensor,
      1
    );

    sensor->set_gain_ctrl(
      sensor,
      1
    );

    sensor->set_lenc(
      sensor,
      1
    );

    sensor->set_bpc(
      sensor,
      1
    );

    sensor->set_wpc(
      sensor,
      1
    );

    sensor->set_raw_gma(
      sensor,
      1
    );
  }

  Serial.println();
  Serial.println(
    "OV3660 initialized successfully"
  );

  return true;
}

// ============================================================
// RELIABLE TCP SEND
// ============================================================

bool sendBuffer(
  WiFiClient& client,
  const uint8_t* data,
  size_t length
)
{
  size_t sentTotal = 0;

  unsigned long lastProgress =
    millis();

  while (sentTotal < length)
  {
    if (!client.connected())
    {
      Serial.println(
        "Client disconnected during send"
      );

      return false;
    }

    size_t remaining =
      length - sentTotal;

    size_t chunk =
      remaining > 4096
      ? 4096
      : remaining;

    size_t sent =
      client.write(
        data + sentTotal,
        chunk
      );

    if (sent > 0)
    {
      sentTotal += sent;

      lastProgress =
        millis();
    }
    else
    {
      delay(1);
    }

    if (
      millis() - lastProgress >
      10000
    )
    {
      Serial.println(
        "TCP transmission timeout"
      );

      return false;
    }

    updateLED();

    delay(0);
  }

  return true;
}

// ============================================================
// SEND FRAME LENGTH
// ============================================================

bool sendFrameLength(
  WiFiClient& client,
  uint32_t length
)
{
  uint8_t header[4];

  header[0] =
    (length >> 24) & 0xFF;

  header[1] =
    (length >> 16) & 0xFF;

  header[2] =
    (length >> 8) & 0xFF;

  header[3] =
    length & 0xFF;

  return sendBuffer(
    client,
    header,
    4
  );
}

// ============================================================
// GESTURE CONTROL SERVER
// ============================================================

void checkGestureControl()
{
  WiFiClient client =
    controlServer.available();

  if (!client)
  {
    return;
  }

  client.setTimeout(100);

  unsigned long start =
    millis();

  String command = "";

  while (
    client.connected() &&
    millis() - start < 500
  )
  {
    if (client.available())
    {
      command =
        client.readStringUntil('\n');

      command.trim();

      break;
    }

    updateLED();

    delay(1);
  }

  if (command.length() > 0)
  {
    Serial.print(
      "GESTURE COMMAND: "
    );

    Serial.println(command);

    // --------------------------------------------------------
    // OPEN PALM
    // --------------------------------------------------------

    if (command == "ON")
    {
      ledMode = LED_ON;

      setLED(true);

      Serial.println(
        "LED MODE: ON"
      );
    }

    // --------------------------------------------------------
    // FIST
    // --------------------------------------------------------

    else if (command == "OFF")
    {
      ledMode = LED_OFF;

      setLED(false);

      Serial.println(
        "LED MODE: OFF"
      );
    }

    // --------------------------------------------------------
    // ONE FINGER
    // --------------------------------------------------------

    else if (command == "SLOW")
    {
      ledMode = LED_SLOW;

      lastBlinkTime =
        millis();

      Serial.println(
        "LED MODE: SLOW BLINK"
      );
    }

    // --------------------------------------------------------
    // TWO FINGERS
    // --------------------------------------------------------

    else if (command == "FAST")
    {
      ledMode = LED_FAST;

      lastBlinkTime =
        millis();

      Serial.println(
        "LED MODE: FAST BLINK"
      );
    }

    client.println("OK");
  }

  client.stop();
}

// ============================================================
// CAMERA STREAM
// ============================================================

void streamCamera(
  WiFiClient& client
)
{
  client.setNoDelay(true);

  client.setTimeout(10000);

  Serial.println();
  Serial.println(
    "=============================================="
  );

  Serial.println(
    "       VIDEO CLIENT CONNECTED"
  );

  Serial.println(
    "=============================================="
  );

  unsigned long frames = 0;

  unsigned long startTime =
    millis();

  while (client.connected())
  {
    // --------------------------------------------------------
    // IMPORTANT:
    // Check gesture commands while video is running.
    // --------------------------------------------------------

    checkGestureControl();

    updateLED();

    // --------------------------------------------------------
    // CAPTURE
    // --------------------------------------------------------

    camera_fb_t* fb =
      esp_camera_fb_get();

    if (fb == NULL)
    {
      Serial.println(
        "ERROR: Camera capture failed"
      );

      delay(10);

      continue;
    }

    size_t jpegSize =
      fb->len;

    // --------------------------------------------------------
    // SIZE CHECK
    // --------------------------------------------------------

    if (
      jpegSize == 0 ||
      jpegSize > JPEG_BUFFER_SIZE
    )
    {
      Serial.print(
        "ERROR: JPEG too large: "
      );

      Serial.println(
        jpegSize
      );

      esp_camera_fb_return(
        fb
      );

      continue;
    }

    // --------------------------------------------------------
    // JPEG HEADER
    // --------------------------------------------------------

    if (
      fb->buf[0] != 0xFF ||
      fb->buf[1] != 0xD8
    )
    {
      Serial.println(
        "ERROR: Invalid JPEG START"
      );

      esp_camera_fb_return(
        fb
      );

      continue;
    }

    // --------------------------------------------------------
    // JPEG END
    // --------------------------------------------------------

    if (
      fb->buf[jpegSize - 2] != 0xFF ||
      fb->buf[jpegSize - 1] != 0xD9
    )
    {
      Serial.println(
        "ERROR: Invalid JPEG END"
      );

      esp_camera_fb_return(
        fb
      );

      continue;
    }

    // --------------------------------------------------------
    // COPY PSRAM → NORMAL RAM
    // --------------------------------------------------------

    memcpy(
      jpegBuffer,
      fb->buf,
      jpegSize
    );

    // --------------------------------------------------------
    // RETURN CAMERA BUFFER
    // --------------------------------------------------------

    esp_camera_fb_return(
      fb
    );

    // --------------------------------------------------------
    // SEND SIZE
    // --------------------------------------------------------

    if (
      !sendFrameLength(
        client,
        jpegSize
      )
    )
    {
      Serial.println(
        "ERROR: Frame header send failed"
      );

      break;
    }

    // --------------------------------------------------------
    // SEND JPEG
    // --------------------------------------------------------

    if (
      !sendBuffer(
        client,
        jpegBuffer,
        jpegSize
      )
    )
    {
      Serial.println(
        "ERROR: JPEG send failed"
      );

      break;
    }

    // --------------------------------------------------------
    // FRAME SUCCESS
    // --------------------------------------------------------

    frames++;

    if (frames % 30 == 0)
    {
      unsigned long now =
        millis();

      float fps =
        frames * 1000.0 /
        (now - startTime);

      Serial.print(
        "FRAME OK | "
      );

      Serial.print(
        frames
      );

      Serial.print(
        " | JPEG: "
      );

      Serial.print(
        jpegSize
      );

      Serial.print(
        " | FPS: "
      );

      Serial.println(
        fps,
        1
      );
    }

    delay(1);
  }

  client.stop();

  Serial.println();
  Serial.println(
    "VIDEO CLIENT DISCONNECTED"
  );

  Serial.print(
    "Frames sent: "
  );

  Serial.println(
    frames
  );
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);

  delay(2000);

  // ----------------------------------------------------------
  // LED
  // ----------------------------------------------------------

  pinMode(
    FLASH_LED_PIN,
    OUTPUT
  );

  setLED(false);

  // ----------------------------------------------------------
  // START
  // ----------------------------------------------------------

  Serial.println();

  Serial.println(
    "=============================================="
  );

  Serial.println(
    "       ESP32 OV3660"
  );

  Serial.println(
    "       GESTURE CONTROL CAMERA"
  );

  Serial.println(
    "=============================================="
  );

  // ----------------------------------------------------------
  // PSRAM
  // ----------------------------------------------------------

  if (psramFound())
  {
    Serial.println(
      "PSRAM detected"
    );
  }
  else
  {
    Serial.println(
      "WARNING: PSRAM NOT DETECTED"
    );
  }

  // ----------------------------------------------------------
  // CAMERA
  // ----------------------------------------------------------

  if (!initCamera())
  {
    Serial.println(
      "CAMERA INITIALIZATION FAILED"
    );

    while (true)
    {
      delay(1000);
    }
  }

  // ----------------------------------------------------------
  // WIFI
  // ----------------------------------------------------------

  Serial.println();

  Serial.println(
    "Connecting to Wi-Fi..."
  );

  WiFi.mode(
    WIFI_STA
  );

  WiFi.setSleep(
    false
  );

  WiFi.begin(
    ssid,
    password
  );

  int attempts = 0;

  while (
    WiFi.status() != WL_CONNECTED &&
    attempts < 40
  )
  {
    delay(500);

    Serial.print(".");

    attempts++;
  }

  Serial.println();

  if (
    WiFi.status() != WL_CONNECTED
  )
  {
    Serial.println(
      "Wi-Fi connection failed!"
    );

    while (true)
    {
      delay(1000);
    }
  }

  // ----------------------------------------------------------
  // WIFI INFORMATION
  // ----------------------------------------------------------

  Serial.println(
    "Wi-Fi connected"
  );

  Serial.print(
    "ESP32 IP: "
  );

  Serial.println(
    WiFi.localIP()
  );

  Serial.print(
    "Signal RSSI: "
  );

  Serial.print(
    WiFi.RSSI()
  );

  Serial.println(
    " dBm"
  );

  // ----------------------------------------------------------
  // START SERVERS
  // ----------------------------------------------------------

  videoServer.begin();

  controlServer.begin();

  Serial.println();

  Serial.println(
    "TCP video server started on port 81"
  );

  Serial.println(
    "TCP gesture server started on port 82"
  );

  Serial.println();

  Serial.println(
    "=============================================="
  );

  Serial.print(
    "VIDEO   : tcp://"
  );

  Serial.print(
    WiFi.localIP()
  );

  Serial.println(
    ":81"
  );

  Serial.print(
    "CONTROL : tcp://"
  );

  Serial.print(
    WiFi.localIP()
  );

  Serial.println(
    ":82"
  );

  Serial.println(
    "=============================================="
  );

  Serial.println();

  Serial.println(
    "ESP32 OV3660 READY!"
  );
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
  updateLED();

  // Check control even when video isn't running
  checkGestureControl();

  // Check video
  WiFiClient client =
    videoServer.available();

  if (client)
  {
    streamCamera(
      client
    );
  }

  delay(1);
}