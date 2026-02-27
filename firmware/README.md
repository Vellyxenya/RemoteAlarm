# RemoteAlarm ESP32 Firmware

ESP32-S3 firmware for receiving and playing audio notifications.

## Hardware
- ESP32-S3 with audio capabilities (e.g., Waveshare AI Smart Speaker Board or ESP32-S3-KORVO-C)
- Built-in speaker
- WiFi connectivity

## Features
- Connects to WiFi
- Subscribes to MQTT topic over TLS (HiveMQ Cloud support)
- Receives JSON payload with signed audio URLs
- Downloads audio files via HTTPS
- Plays audio using ESP-ADF `audio_pipeline` (HTTP -> WAV -> I2S)

## Configuration Management

This project uses the ESP-IDF Kconfig system to manage credentials. **Never hardcode WiFi or MQTT credentials in the source code.**

### How it works:
- **Kconfig.projbuild**: Defines the configuration interface.
- **sdkconfig**: A local file generated when you configure the project. It contains your actual secrets and is **automatically ignored by git** (via `.gitignore`).
- **main.c**: Uses `CONFIG_` macros that are populated from your `sdkconfig` during compilation.

### Setting your Credentials:
1. Navigate to the firmware directory:
   ```bash
   cd firmware
   ```
2. Open the configuration menu:
   ```bash
   idf.py menuconfig # See [docs/esp-setup.md](../docs/esp-setup.md) if idf.py is not found
   ```
3. Navigate to the **"Remote Alarm Configuration"** menu.
4. Enter your specific details:
   - **WiFi SSID/Password**: Your local network credentials.
   - **MQTT Broker**: The url of your HiveMQ cluster (e.g., `mqtts://...:8883`).
   - **MQTT Username/Password**: The credentials created for the ESP32 in HiveMQ.
   - **MQTT Topic**: Usually `home/audio/device1`.
5. Save (`S`) and Quit (`Q`).

## Build and Flash
1. Ensure your ESP-IDF and ESP-ADF environment is sourced (e.g., `. $HOME/esp/esp-idf/export.sh`).
2. Build the project:
   ```bash
   idf.py build # See [docs/esp-setup.md](../docs/esp-setup.md) if idf.py is not found
   ```
3. Flash to your device:
   ```bash
   idf.py -p <PORT> flash monitor
   ```

## Dependencies
- **ESP-IDF (v5.x)**
- **ESP-ADF**: Required for the audio pipeline and I2S output.
- **cJSON**: Used for parsing the MQTT notification payload.
