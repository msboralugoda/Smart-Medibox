# 💊 Medibox – ESP32 Smart Medicine Box

A two-stage biomedical device project developed as part of the EN2853: Embedded Systems and Applications course at the University of Moratuwa. This device assists users in managing their medication schedules and ensures proper storage conditions through IoT integration, sensing, and actuation.

## 📦 Overview

The Medibox is a smart medicine box implemented on the ESP32 microcontroller and simulated using Wokwi. It supports medication reminders, environmental monitoring, and intelligent actuation using sensors, actuators, and Node-RED dashboards via MQTT.

---

## 🧪 Features

### ✅ Stage 1 (Core Features)
- ⏰ Real-Time Clock: Fetched from NTP server and displayed on an OLED screen.
- 🌍 Time Zone Configuration: User-selectable UTC offset.
- 🔔 Dual Alarms: Set, view, and delete two alarms.
- ⏳ Snooze & Stop: Stop or snooze alarms for 5 minutes via button press.
- 🌡️ Environmental Monitoring:
  - Temperature and humidity readings using DHT22.
  - Warning alerts if values exceed healthy ranges:
    - Temperature: 24–32°C
    - Humidity: 65%–80%
- 📢 Indications: Buzzer, LED, and OLED messages for alarms and warnings.

### 🆕 Stage 2 (Enhancements)
- 🌞 Light-Sensitive Storage:
  - Light monitoring using LDR with user-configurable sampling and sending intervals.
  - Display average light intensity (normalized 0–1) on Node-RED with charts and numeric displays.
- 🤖 Servo-Controlled Sliding Window:
  - Adjusts servo angle (0–180°) to control light exposure based on a formula involving temperature, light intensity, and user-tuned parameters.
- 📊 Configurable Parameters (via Node-RED):
  - θ<sub>offset</sub>: Minimum angle of window (30–120°).
  - γ: Controlling factor (0–1).
  - T<sub>med</sub>: Ideal medicine storage temperature (10–40°C).
- 🔄 MQTT Integration:
  - Bidirectional communication between ESP32 and Node-RED for real-time control and feedback.

---

## 🌐 Architecture

- **Microcontroller**: ESP32
- **Sensors**: DHT22 (temperature/humidity), LDR (light intensity)
- **Actuators**: Servo motor (window control), Buzzer, LED
- **Display**: OLED (time, alerts)
- **Interface**: Node-RED dashboard with sliders, charts, numeric displays
- **Communication**: MQTT via `test.mosquitto.org`

---

## 🚀 Getting Started

1. Clone the repository.
2. Open the Wokwi project file or upload the Arduino code to your ESP32.
3. Import the Node-RED flow JSON.
4. Connect to MQTT broker.
5. Simulate alarms, monitor environment, and test dynamic servo control.

---

## 🎯 Use Case

Designed for elderly or chronically ill patients who require timely medication and proper storage of photosensitive drugs. This prototype merges assistive technology and environmental intelligence in an accessible way.

---




