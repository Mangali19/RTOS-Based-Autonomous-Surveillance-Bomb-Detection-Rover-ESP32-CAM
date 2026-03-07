#include "esp_camera.h"
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <iostream>
#include <sstream>

// ─── FreeRTOS ───────────────────────────────────────────────
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

// ─── Task Handles ───────────────────────────────────────────
TaskHandle_t cameraTaskHandle   = NULL;
TaskHandle_t motorTaskHandle    = NULL;
TaskHandle_t wsCleanupHandle    = NULL;

// ─── Queue for motor commands ───────────────────────────────
QueueHandle_t motorCommandQueue;

// ─── Semaphore to protect WebSocket client ID ───────────────
SemaphoreHandle_t cameraMutex;

// ─── Motor Pin Structure ─────────────────────────────────────
struct MOTOR_PINS
{
  int pinEn;
  int pinIN1;
  int pinIN2;
};

std::vector<MOTOR_PINS> motorPins =
{
  {12, 13, 15},   // RIGHT MOTOR (EnA, IN1, IN2)
  {12, 14,  2},   // LEFT  MOTOR (EnB, IN3, IN4)
};

// ─── Defines ─────────────────────────────────────────────────
#define LIGHT_PIN       4
#define UP              1
#define DOWN            2
#define LEFT            3
#define RIGHT           4
#define STOP            0
#define RIGHT_MOTOR     0
#define LEFT_MOTOR      1
#define FORWARD         1
#define BACKWARD       -1

const int PWMFreq         = 1000;
const int PWMResolution   = 8;
const int PWMSpeedChannel = 2;
const int PWMLightChannel = 3;

// ─── Camera GPIO ─────────────────────────────────────────────
#define PWDN_GPIO_NUM   32
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM    0
#define SIOD_GPIO_NUM   26
#define SIOC_GPIO_NUM   27
#define Y9_GPIO_NUM     35
#define Y8_GPIO_NUM     34
#define Y7_GPIO_NUM     39
#define Y6_GPIO_NUM     36
#define Y5_GPIO_NUM     21
#define Y4_GPIO_NUM     19
#define Y3_GPIO_NUM     18
#define Y2_GPIO_NUM      5
#define VSYNC_GPIO_NUM  25
#define HREF_GPIO_NUM   23
#define PCLK_GPIO_NUM   22

const char* ssid     = "MyWiFiCar";
const char* password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket wsCamera("/Camera");
AsyncWebSocket wsCarInput("/CarInput");
volatile uint32_t cameraClientId = 0;

// ─── Motor Command Struct ────────────────────────────────────
struct MotorCommand
{
  String key;
  int value;
};

// ─── HTML Page ───────────────────────────────────────────────
const char* htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
    <style>
      .arrows { font-size:40px; color:red; }
      td.button { background-color:black; border-radius:25%; box-shadow:5px 5px #888888; }
      td.button:active { transform:translate(5px,5px); box-shadow:none; }
      .noselect { -webkit-user-select:none; user-select:none; }
      .slidecontainer { width:100%; }
      .slider { -webkit-appearance:none; width:100%; height:15px; border-radius:5px;
                background:#d3d3d3; outline:none; opacity:0.7; transition:opacity .2s; }
      .slider:hover { opacity:1; }
      .slider::-webkit-slider-thumb { -webkit-appearance:none; width:25px; height:25px;
                border-radius:50%; background:red; cursor:pointer; }
      .slider::-moz-range-thumb { width:25px; height:25px; border-radius:50%;
                background:red; cursor:pointer; }
      #status { font-size:14px; color:green; margin-top:8px; }
    </style>
  </head>
  <body class="noselect" align="center" style="background-color:white">
    <h3 style="color:teal">Trifuse Rover Control</h3>
    <table id="mainTable" style="width:400px; margin:auto; table-layout:fixed" CELLSPACING=10>
      <tr><td colspan=3><img id="cameraImage" src="" style="width:400px; height:250px"></td></tr>
      <tr>
        <td></td>
        <td class="button" ontouchstart='sendButtonInput("MoveCar","1")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8679;</span></td>
        <td></td>
      </tr>
      <tr>
        <td class="button" ontouchstart='sendButtonInput("MoveCar","3")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8678;</span></td>
        <td class="button"></td>
        <td class="button" ontouchstart='sendButtonInput("MoveCar","4")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8680;</span></td>
      </tr>
      <tr>
        <td></td>
        <td class="button" ontouchstart='sendButtonInput("MoveCar","2")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8681;</span></td>
        <td></td>
      </tr>
      <tr/><tr/>
      <tr>
        <td style="text-align:left"><b>Speed:</b></td>
        <td colspan=2>
          <div class="slidecontainer">
            <input type="range" min="0" max="255" value="150" class="slider" id="Speed" oninput='sendButtonInput("Speed",value)'>
          </div>
        </td>
      </tr>
      <tr>
        <td style="text-align:left"><b>Light:</b></td>
        <td colspan=2>
          <div class="slidecontainer">
            <input type="range" min="0" max="255" value="0" class="slider" id="Light" oninput='sendButtonInput("Light",value)'>
          </div>
        </td>
      </tr>
    </table>
    <p id="status">Connecting...</p>
    <script>
      var webSocketCameraUrl   = "ws://" + window.location.hostname + "/Camera";
      var webSocketCarInputUrl = "ws://" + window.location.hostname + "/CarInput";
      var websocketCamera;
      var websocketCarInput;

      function initCameraWebSocket() {
        websocketCamera = new WebSocket(webSocketCameraUrl);
        websocketCamera.binaryType = 'blob';
        websocketCamera.onopen  = function(event) { document.getElementById("status").innerText = "Camera Connected"; };
        websocketCamera.onclose = function(event) { setTimeout(initCameraWebSocket, 2000); };
        websocketCamera.onmessage = function(event) {
          var imageId = document.getElementById("cameraImage");
          imageId.src = URL.createObjectURL(event.data);
        };
      }

      function initCarInputWebSocket() {
        websocketCarInput = new WebSocket(webSocketCarInputUrl);
        websocketCarInput.onopen = function(event) {
          sendButtonInput("Speed", document.getElementById("Speed").value);
          sendButtonInput("Light", document.getElementById("Light").value);
        };
        websocketCarInput.onclose   = function(event) { setTimeout(initCarInputWebSocket, 2000); };
        websocketCarInput.onmessage = function(event) {};
      }

      function initWebSocket() { initCameraWebSocket(); initCarInputWebSocket(); }

      function sendButtonInput(key, value) {
        var data = key + "," + value;
        websocketCarInput.send(data);
      }

      window.onload = initWebSocket;
      document.getElementById("mainTable").addEventListener("touchend", function(event) {
        event.preventDefault();
      });
    </script>
  </body>
</html>
)HTMLHOMEPAGE";

// ═══════════════════════════════════════════════════════════════
//  MOTOR FUNCTIONS
// ═══════════════════════════════════════════════════════════════
void rotateMotor(int motorNumber, int motorDirection)
{
  if (motorDirection == FORWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, HIGH);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);
  }
  else if (motorDirection == BACKWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, HIGH);
  }
  else
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);
  }
}

void moveCar(int inputValue)
{
  Serial.printf("MoveCar: %d\n", inputValue);
  switch (inputValue)
  {
    case UP:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR,  FORWARD);
      break;
    case DOWN:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR,  BACKWARD);
      break;
    case LEFT:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR,  BACKWARD);
      break;
    case RIGHT:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR,  FORWARD);
      break;
    case STOP:
    default:
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR,  STOP);
      break;
  }
}

// ═══════════════════════════════════════════════════════════════
//  RTOS TASK 1 — CAMERA STREAMING (Core 0)
// ═══════════════════════════════════════════════════════════════
void cameraTask(void *pvParameters)
{
  Serial.printf("cameraTask running on core %d\n", xPortGetCoreID());
  while (true)
  {
    if (xSemaphoreTake(cameraMutex, portMAX_DELAY) == pdTRUE)
    {
      uint32_t clientId = cameraClientId;
      xSemaphoreGive(cameraMutex);

      if (clientId != 0)
      {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb)
        {
          wsCamera.binary(clientId, fb->buf, fb->len);
          esp_camera_fb_return(fb);

          // Wait until frame is delivered
          AsyncWebSocketClient *clientPtr = wsCamera.client(clientId);
          while (clientPtr && clientPtr->queueIsFull())
          {
            vTaskDelay(pdMS_TO_TICKS(1));
          }
        }
        else
        {
          Serial.println("Frame buffer error");
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(33));  // ~30 FPS target
  }
}

// ═══════════════════════════════════════════════════════════════
//  RTOS TASK 2 — MOTOR COMMAND PROCESSING (Core 1)
// ═══════════════════════════════════════════════════════════════
void motorTask(void *pvParameters)
{
  Serial.printf("motorTask running on core %d\n", xPortGetCoreID());
  MotorCommand cmd;
  while (true)
  {
    // Block until a command arrives in queue (max wait 5ms)
    if (xQueueReceive(motorCommandQueue, &cmd, pdMS_TO_TICKS(5)) == pdTRUE)
    {
      if (cmd.key == "MoveCar")
      {
        moveCar(cmd.value);
      }
      else if (cmd.key == "Speed")
      {
        ledcWrite(PWMSpeedChannel, cmd.value);
      }
      else if (cmd.key == "Light")
      {
        ledcWrite(PWMLightChannel, cmd.value);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1));  // yield to scheduler
  }
}

// ═══════════════════════════════════════════════════════════════
//  RTOS TASK 3 — WEBSOCKET CLEANUP (Core 1)
// ═══════════════════════════════════════════════════════════════
void wsCleanupTask(void *pvParameters)
{
  while (true)
  {
    wsCamera.cleanupClients();
    wsCarInput.cleanupClients();
    vTaskDelay(pdMS_TO_TICKS(1000));  // cleanup every 1 second
  }
}

// ═══════════════════════════════════════════════════════════════
//  WEBSOCKET EVENT HANDLERS
// ═══════════════════════════════════════════════════════════════
void onCarInputWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                               AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
    case WS_EVT_CONNECT:
      Serial.printf("CarInput WS client #%u connected\n", client->id());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("CarInput WS client #%u disconnected\n", client->id());
      {
        MotorCommand stopCmd = {"MoveCar", STOP};
        xQueueSend(motorCommandQueue, &stopCmd, 0);
        MotorCommand lightOff = {"Light", 0};
        xQueueSend(motorCommandQueue, &lightOff, 0);
      }
      break;
    case WS_EVT_DATA:
    {
      AwsFrameInfo *info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
      {
        std::string myData((char*)data, len);
        std::istringstream ss(myData);
        std::string key, value;
        std::getline(ss, key, ',');
        std::getline(ss, value, ',');
        Serial.printf("Key[%s] Value[%s]\n", key.c_str(), value.c_str());

        MotorCommand cmd;
        cmd.key   = String(key.c_str());
        cmd.value = atoi(value.c_str());
        xQueueSend(motorCommandQueue, &cmd, 0);  // non-blocking send
      }
      break;
    }
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
    default:
      break;
  }
}

void onCameraWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                             AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
    case WS_EVT_CONNECT:
      Serial.printf("Camera WS client #%u connected\n", client->id());
      if (xSemaphoreTake(cameraMutex, portMAX_DELAY) == pdTRUE)
      {
        cameraClientId = client->id();
        xSemaphoreGive(cameraMutex);
      }
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("Camera WS client #%u disconnected\n", client->id());
      if (xSemaphoreTake(cameraMutex, portMAX_DELAY) == pdTRUE)
      {
        cameraClientId = 0;
        xSemaphoreGive(cameraMutex);
      }
      break;
    default:
      break;
  }
}

// ═══════════════════════════════════════════════════════════════
//  CAMERA INIT
// ═══════════════════════════════════════════════════════════════
void setupCamera()
{
  camera_config_t config;
  config.ledc_channel  = LEDC_CHANNEL_0;
  config.ledc_timer    = LEDC_TIMER_0;
  config.pin_d0        = Y2_GPIO_NUM;
  config.pin_d1        = Y3_GPIO_NUM;
  config.pin_d2        = Y4_GPIO_NUM;
  config.pin_d3        = Y5_GPIO_NUM;
  config.pin_d4        = Y6_GPIO_NUM;
  config.pin_d5        = Y7_GPIO_NUM;
  config.pin_d6        = Y8_GPIO_NUM;
  config.pin_d7        = Y9_GPIO_NUM;
  config.pin_xclk      = XCLK_GPIO_NUM;
  config.pin_pclk      = PCLK_GPIO_NUM;
  config.pin_vsync     = VSYNC_GPIO_NUM;
  config.pin_href      = HREF_GPIO_NUM;
  config.pin_sscb_sda  = SIOD_GPIO_NUM;
  config.pin_sscb_scl  = SIOC_GPIO_NUM;
  config.pin_pwdn      = PWDN_GPIO_NUM;
  config.pin_reset     = RESET_GPIO_NUM;
  config.xclk_freq_hz  = 20000000;
  config.pixel_format  = PIXFORMAT_JPEG;
  config.frame_size    = FRAMESIZE_VGA;
  config.jpeg_quality  = 10;
  config.fb_count      = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return;
  }
  if (psramFound())
  {
    heap_caps_malloc_extmem_enable(20000);
    Serial.println("PSRAM initialized");
  }
}

// ═══════════════════════════════════════════════════════════════
//  PIN SETUP
// ═══════════════════════════════════════════════════════════════
void setUpPinModes()
{
  ledcSetup(PWMSpeedChannel, PWMFreq, PWMResolution);
  ledcSetup(PWMLightChannel, PWMFreq, PWMResolution);

  for (int i = 0; i < (int)motorPins.size(); i++)
  {
    pinMode(motorPins[i].pinEn,  OUTPUT);
    pinMode(motorPins[i].pinIN1, OUTPUT);
    pinMode(motorPins[i].pinIN2, OUTPUT);
    ledcAttachPin(motorPins[i].pinEn, PWMSpeedChannel);
  }
  moveCar(STOP);
  pinMode(LIGHT_PIN, OUTPUT);
  ledcAttachPin(LIGHT_PIN, PWMLightChannel);
}

// ═══════════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════════
void setup()
{
  Serial.begin(115200);
  setUpPinModes();

  // Create FreeRTOS primitives
  motorCommandQueue = xQueueCreate(10, sizeof(MotorCommand));
  cameraMutex       = xSemaphoreCreateMutex();

  // WiFi SoftAP
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", htmlHomePage);
  });
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not Found");
  });

  wsCamera.onEvent(onCameraWebSocketEvent);
  wsCarInput.onEvent(onCarInputWebSocketEvent);
  server.addHandler(&wsCamera);
  server.addHandler(&wsCarInput);
  server.begin();
  Serial.println("HTTP server started");

  setupCamera();

  // ── Create RTOS Tasks ──────────────────────────────────────
  // Camera task on Core 0, high priority
  xTaskCreatePinnedToCore(
    cameraTask,         // function
    "CameraTask",       // name
    8192,               // stack size (bytes)
    NULL,               // parameter
    5,                  // priority (higher = more urgent)
    &cameraTaskHandle,  // handle
    0                   // Core 0
  );

  // Motor task on Core 1, medium priority
  xTaskCreatePinnedToCore(
    motorTask,
    "MotorTask",
    4096,
    NULL,
    4,
    &motorTaskHandle,
    1   // Core 1
  );

  // WebSocket cleanup task on Core 1, low priority
  xTaskCreatePinnedToCore(
    wsCleanupTask,
    "WSCleanup",
    2048,
    NULL,
    1,
    &wsCleanupHandle,
    1   // Core 1
  );

  Serial.println("All RTOS tasks created");
}

// ═══════════════════════════════════════════════════════════════
//  LOOP — minimal, tasks handle everything
// ═══════════════════════════════════════════════════════════════
void loop()
{
  // All work is done in RTOS tasks
  // Loop just monitors heap
  Serial.printf("[Heap] Free: %d | PSRAM Free: %d\n",
                ESP.getFreeHeap(), ESP.getFreePsram());
  vTaskDelay(pdMS_TO_TICKS(5000));
}
