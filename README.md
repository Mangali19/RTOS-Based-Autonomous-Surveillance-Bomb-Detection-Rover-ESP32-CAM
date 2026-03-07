# 🤖 RTOS-Based Autonomous Surveillance & Bomb Detection Rover — ESP32-CAM

<p align="center">
  <img src="image.png" alt="Trifuse Rover" width="600"/>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Platform-ESP32--CAM-blue?style=for-the-badge&logo=espressif"/>
  <img src="https://img.shields.io/badge/RTOS-FreeRTOS-green?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Language-C%2B%2B-orange?style=for-the-badge&logo=cplusplus"/>
  <img src="https://img.shields.io/badge/Protocol-WebSocket-yellow?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/License-MIT-red?style=for-the-badge"/>
</p>

---

## 📌 Overview

**Trifuse Rover** is a low-cost, FreeRTOS-based autonomous surveillance and bomb detection robot
built on the **ESP32-CAM** microcontroller. It streams live video over WiFi to a browser-based
control interface and uses **probabilistic multi-sensor fusion** (gas + metal + temperature) to
detect potential explosive threats — without putting any human life at risk.

All major operations — camera streaming, motor control, and WebSocket cleanup — run as
**independent FreeRTOS tasks** pinned to separate CPU cores, ensuring **<5ms deterministic
motor latency** even during continuous video streaming.

> 🎓 Mini Project — B.Tech ECE, IIIT-RGUKT Srikakulam, 2025
> 👨‍🏫 Guide: Dr. H. Srinivasa Varaprasad, M.Tech, Ph.D

---

## 🎯 Key Features

- 📡 **Real-time WiFi video streaming** via Async WebSocket (~30 FPS, VGA)
- 🎮 **Browser-based remote control** — no app needed, works on any phone/PC
- 🔥 **MQ-9 Gas Sensor** — detects CO and flammable gases (10–1000 ppm)
- 🧲 **Metal Detector Sensor** — detects metallic/IED objects electromagnetically
- 🌡️ **NTC Thermistor** — detects abnormal temperature spikes near explosives
- 🧠 **Probabilistic Bomb Detection** — alert triggers only when 2+ sensors confirm threat
- 📷 **Pan-Tilt Servo Camera** — full horizontal + vertical rotation control
- 💡 **Remote LED Flash** — brightness control via slider in browser
- ⚡ **FreeRTOS Multi-tasking** — camera on Core 0, motors on Core 1 (true parallelism)
- 🔒 **Mutex-protected shared data** — no race conditions between tasks
- 📬 **Queue-driven motor commands** — safe WebSocket-to-hardware communication

---

---

## 🛠️ Hardware Components

| Component | Quantity | Purpose |
|-----------|----------|---------|
| ESP32-CAM (AI Thinker) | 1 | Main controller + OV2640 camera |
| ESP8266 (NodeMCU) | 1 | Sensor node (gas, metal, temp) |
| L293D Motor Driver | 1 | H-Bridge DC motor control |
| BO DC Motors | 4 | Chassis movement |
| MQ-9 Gas Sensor | 1 | CO / flammable gas detection |
| NTC Thermistor | 1 | Temperature anomaly detection |
| Metal Detector Sensor | 1 | Metallic object detection |
| SG90 Pan-Tilt Servo | 2 | Camera directional control |
| Li-Ion Battery Pack | 1 | Power supply |
| 4WD Car Chassis | 1 | Robot base platform |
| Breadboard + Jumper Wires | – | Prototyping connections |

---

## 💻 Software & Libraries

| Tool | Purpose |
|------|---------|
| Arduino IDE 2.x | Code editor + uploader |
| ESP32 Board Package (Espressif) | ESP32-CAM support |
| ESPAsyncWebServer | Async HTTP + WebSocket server |
| AsyncTCP | Async TCP for ESP32 |
| esp_camera.h | Camera driver (OV2640) |
| FreeRTOS | Task, queue, semaphore management |
| Adafruit MQTT | Sensor data publishing to cloud |
| WiFi.h / ESP8266WiFi.h | WiFi connectivity |

---

## 📐 Pin Configuration

### ESP32-CAM (Motor + Camera)

| GPIO | Function |
|------|----------|
| GPIO12 | Motor Enable (PWM Speed) |
| GPIO13 | Right Motor IN1 |
| GPIO15 | Right Motor IN2 |
| GPIO14 | Left Motor IN3 |
| GPIO2 | Left Motor IN4 |
| GPIO4 | LED Flash (PWM) |
| GPIO12 | Pan Servo Signal |
| GPIO13 | Tilt Servo Signal |

### ESP8266 (Sensor Node)

| GPIO | Sensor | Type |
|------|--------|------|
| GPIO14 | Metal Detector | Digital INPUT |
| GPIO0 | NTC Thermistor | Analog INPUT |
| GPIO2 | MQ-9 Gas Sensor | Analog INPUT |

---
### 2. Install Required Libraries in Arduino IDE

Go to Sketch → Include Library → Manage Libraries and install:
- ESPAsyncWebServer by lacamera
- AsyncTCP by dvarrel
- Adafruit MQTT Library by Adafruit

### 3. Flash ESP32-CAM

### 4. Flash ESP8266


Replace YOUR_AIO_USERNAME and YOUR_AIO_KEY with your Adafruit IO credentials before uploading.

### 5. Connect and Control


---

## 🧠 Probabilistic Bomb Detection Logic

The rover avoids false alarms by requiring 2 or more sensors to trigger simultaneously:


Alert levels are published to Adafruit IO MQTT feed in real time.

---

## ⚡ FreeRTOS Task Summary

| Task | Core | Priority | Stack | Period |
|------|------|----------|-------|--------|
| CameraTask | 0 | 5 High | 8192 B | 33ms ~30 FPS |
| MotorTask | 1 | 4 Med | 4096 B | Queue-driven |
| WSCleanupTask | 1 | 1 Low | 2048 B | 1000ms |
| ESP8266 SensorRead | - | - | - | 500ms |
| ESP8266 AlertCheck | - | - | - | 300ms |
| ESP8266 MQTTPublish | - | - | - | 2000ms |

Synchronization primitives used:
- xQueueCreate(10) — motor command buffer decouples ISR from hardware
- xSemaphoreCreateMutex() — protects cameraClientId shared variable
- xTaskCreatePinnedToCore() — dedicates each task to a specific CPU core

---

## 📂 Project Structure


---

## 📊 Performance Metrics

| Metric | Value |
|--------|-------|
| Video Stream FPS | ~30 FPS VGA |
| Motor Command Latency | less than 5ms |
| WiFi Range | ~30 meters open area |
| Sensor Read Period | 500ms |
| Alert Check Period | 300ms |
| MQTT Publish Period | 2000ms |
| Operating Voltage | 5V USB or 7.4V Li-Ion |

---

## 🔭 Future Improvements

- [ ] Add SLAM-based autonomous navigation
- [ ] Integrate TensorFlow Lite for AI-based threat classification
- [ ] Add GSM module for SMS alerts without WiFi
- [ ] Replace browser UI with Android app
- [ ] Add GPS module for location tagging of detected threats
- [ ] Upgrade to ESP32-S3 for faster AI inference

---



## 📜 License

This project is licensed under the MIT License — see the LICENSE file for details.

---

<p align="center">
If you found this project useful, please consider giving it a star!
</p>


