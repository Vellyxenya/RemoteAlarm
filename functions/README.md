# RemoteAlarm Cloud Functions

Firebase Cloud Functions for handling audio upload events and MQTT notification.

## Features
- Triggers on Firebase Storage audio upload
- Generates signed download URLs
- Publishes MQTT messages to ESP32 device

## Setup
```bash
firebase init functions
npm install
```

## Dependencies
- firebase-functions
- firebase-admin
- mqtt (or mqtts client library)

## Environment Variables
Required:
- `MQTT_BROKER_URL`: HiveMQ Cloud broker URL
- `MQTT_USERNAME`: MQTT authentication username
- `MQTT_PASSWORD`: MQTT authentication password
- `MQTT_DEVICE_TOPIC`: Topic for ESP32 device (e.g., `home/audio/<device-id>`)
