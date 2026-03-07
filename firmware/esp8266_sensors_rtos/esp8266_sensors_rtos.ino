// Note: ESP8266 uses FreeRTOS internally via ESP8266 Arduino Core
// Tasks use system_os_task or bare threads via scheduler

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// ─── Sensor Pins ─────────────────────────────────────────────
#define METAL_PIN   14    // GPIO14 Digital
#define THERM_PIN    0    // GPIO0  Analog
#define GAS_PIN      2    // GPIO2  Analog

// ─── Thresholds ──────────────────────────────────────────────
#define TEMP_THRESHOLD   50.0
#define GAS_THRESHOLD     1.5

// ─── WiFi / Adafruit IO ──────────────────────────────────────
const char* ssid     = "MyWiFiCar";
const char* password = "12345678";

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "YOUR_AIO_USERNAME"
#define AIO_KEY         "YOUR_AIO_KEY"

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish metalFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/metal");
Adafruit_MQTT_Publish gasFeed   = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gas");
Adafruit_MQTT_Publish tempFeed  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish alertFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/alert");

// ─── Shared volatile sensor data ─────────────────────────────
volatile int   metalDetected = 0;
volatile float temperature   = 0.0;
volatile float gasVoltage    = 0.0;
volatile int   alertLevel    = 0;   // 0=safe, 1=warning, 2=danger

// ─── Timing control (simulates task periods) ─────────────────
unsigned long lastSensorRead = 0;
unsigned long lastMQTTSend   = 0;
unsigned long lastAlertCheck = 0;

const int SENSOR_PERIOD_MS = 500;   // read sensors every 500ms
const int MQTT_PERIOD_MS   = 2000;  // publish every 2 seconds
const int ALERT_PERIOD_MS  = 300;   // check alerts every 300ms

// ═══════════════════════════════════════════════════════════════
//  TASK A — READ SENSORS (runs every 500ms)
// ═══════════════════════════════════════════════════════════════
void task_readSensors()
{
  // Metal Detector (Digital, active LOW)
  metalDetected = (digitalRead(METAL_PIN) == LOW) ? 1 : 0;

  // NTC Thermistor
  int thermRaw  = analogRead(THERM_PIN);
  float voltage = thermRaw * (3.3 / 1023.0);
  temperature   = (voltage - 0.5) * 100.0;

  // MQ-9 Gas Sensor
  int gasRaw  = analogRead(GAS_PIN);
  gasVoltage  = gasRaw * (3.3 / 1023.0);

  Serial.printf("[Sensors] Metal:%d | Temp:%.1f C | Gas:%.2f V\n",
                metalDetected, temperature, gasVoltage);
}

// ═══════════════════════════════════════════════════════════════
//  TASK B — PROBABILISTIC ALERT CHECK (runs every 300ms)
// ═══════════════════════════════════════════════════════════════
void task_alertCheck()
{
  int threatCount = 0;

  if (metalDetected)                  threatCount++;
  if (temperature  > TEMP_THRESHOLD)  threatCount++;
  if (gasVoltage   > GAS_THRESHOLD)   threatCount++;

  if (threatCount >= 2)
  {
    alertLevel = 2;   // DANGER — 2 or more sensors triggered
    Serial.println("!!! BOMB ALERT: HIGH DANGER !!!");
  }
  else if (threatCount == 1)
  {
    alertLevel = 1;   // WARNING — single sensor
    Serial.println("!! WARNING: Single sensor triggered");
  }
  else
  {
    alertLevel = 0;   // SAFE
  }
}

// ═══════════════════════════════════════════════════════════════
//  TASK C — MQTT PUBLISH (runs every 2000ms)
// ═══════════════════════════════════════════════════════════════
void task_mqttPublish()
{
  // Reconnect if needed
  if (!mqtt.connected())
  {
    Serial.print("Reconnecting MQTT...");
    int8_t ret = mqtt.connect();
    if (ret != 0)
    {
      Serial.printf("Failed: %s\n", mqtt.connectErrorString(ret));
      mqtt.disconnect();
      return;
    }
    Serial.println("Connected!");
  }
  mqtt.processPackets(10);

  // Publish all sensor values
  metalFeed.publish(metalDetected ? "Metal Detected" : "No Metal");
  tempFeed.publish(temperature);
  gasFeed.publish(gasVoltage);

  // Publish alert level
  if      (alertLevel == 2) alertFeed.publish("DANGER");
  else if (alertLevel == 1) alertFeed.publish("WARNING");
  else                      alertFeed.publish("SAFE");

  Serial.printf("[MQTT] Published — Alert Level: %d\n", alertLevel);
}

// ═══════════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════════
void setup()
{
  Serial.begin(115200);
  delay(100);

  pinMode(METAL_PIN, INPUT);
  pinMode(THERM_PIN, INPUT);
  pinMode(GAS_PIN,   INPUT);

  // Connect WiFi
  Serial.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
}

// ═══════════════════════════════════════════════════════════════
//  LOOP — cooperative scheduler simulating RTOS tasks
// ═══════════════════════════════════════════════════════════════
void loop()
{
  unsigned long now = millis();

  // Task A: Sensor reading — 500ms period
  if (now - lastSensorRead >= SENSOR_PERIOD_MS)
  {
    lastSensorRead = now;
    task_readSensors();
  }

  // Task B: Alert check — 300ms period
  if (now - lastAlertCheck >= ALERT_PERIOD_MS)
  {
    lastAlertCheck = now;
    task_alertCheck();
  }

  // Task C: MQTT publish — 2000ms period
  if (now - lastMQTTSend >= MQTT_PERIOD_MS)
  {
    lastMQTTSend = now;
    task_mqttPublish();
  }
}
