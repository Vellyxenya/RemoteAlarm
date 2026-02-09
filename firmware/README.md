# RemoteAlarm ESP32 Firmware

ESP32-S3 firmware for receiving and playing audio notifications.

## Hardware
- ESP32-S3 with audio capabilities (e.g., Waveshare AI Smart Speaker Board)
- Built-in speaker
- WiFi connectivity

## Features
- Connects to WiFi
- Subscribes to MQTT topic over TLS
- Receives signed download URLs
- Downloads audio files via HTTPS
- Plays audio using ESP-ADF

## Setup
1. Install ESP-IDF (v5.x recommended)
2. Install ESP-ADF
3. Configure WiFi and MQTT credentials:
   ```bash
   idf.py menuconfig
   ```
4. Build and flash:
   ```bash
   idf.py build
   idf.py flash monitor
   ```

## Configuration
Set in menuconfig:
- WiFi SSID and password
- MQTT broker URL (HiveMQ)
- MQTT username and password
- MQTT device topic
- Optional: HMAC secret key

## Dependencies
- ESP-IDF (v5.x)
- ESP-ADF (audio framework)
- esp-mqtt component
- esp-tls component
