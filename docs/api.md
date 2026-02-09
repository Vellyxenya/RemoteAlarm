# API Reference

## Cloud Function

### Trigger: Storage onFinalize

**Event**: New file uploaded to `audio/*`

**Payload**:
```javascript
{
  name: 'audio/uuid.wav',
  bucket: 'project-id.appspot.com',
  contentType: 'audio/wav',
  timeCreated: '2026-02-09T12:34:56Z'
}
```

### Generated Signed URL

**Format**:
```
https://storage.googleapis.com/bucket/audio/uuid.wav?GoogleAccessId=...&Expires=...&Signature=...
```

**Expiration**: 5 minutes (300 seconds)

### MQTT Message

**Topic**: `home/audio/<device-id>`

**Payload**:
```json
{
  "file_url": "https://storage.googleapis.com/...",
  "timestamp": "2026-02-09T12:34:56.789Z",
  "filename": "uuid.wav",
  "content_type": "audio/wav"
}
```

**Optional with HMAC**:
```json
{
  "file_url": "https://storage.googleapis.com/...",
  "timestamp": "2026-02-09T12:34:56.789Z",
  "filename": "uuid.wav",
  "content_type": "audio/wav",
  "signature": "a1b2c3d4..."
}
```

## Flutter App

### Audio Recording

**Method**: `startRecording()`, `stopRecording()`

**Format**: WAV (preferred) or AAC

**Duration**: 1-5 seconds

**Sample Rate**: 16kHz recommended

### Upload

**Endpoint**: Firebase Storage

**Path**: `audio/<uuid>.wav`

**Method**: `putFile()`

**Authentication**: Firebase Auth token (automatic)

## ESP32 Firmware

### MQTT Subscribe

**Topic**: `home/audio/<device-id>`

**QoS**: 1 (at least once delivery)

**Broker**: HiveMQ Cloud (MQTTS, port 8883)

### HTTP Download

**Method**: GET

**URL**: Signed URL from MQTT message

**TLS**: Required

**Timeout**: 30 seconds recommended

### Audio Playback

**Format Support**: WAV, MP3

**Pipeline**: HTTP stream → Decoder → I2S output

**Sample Rates**: 8kHz, 16kHz, 44.1kHz

## Firebase Storage Rules

```javascript
rules_version = '2';
service firebase.storage {
  match /b/{bucket}/o {
    match /audio/{fileName} {
      // Allow authenticated users to write
      allow write: if request.auth != null;
      
      // No direct read access
      allow read: if false;
    }
  }
}
```

## Environment Variables

### Cloud Functions
- `MQTT_BROKER_URL`: e.g., `mqtt://abc123.hivemq.cloud`
- `MQTT_PORT`: `8883`
- `MQTT_USERNAME`: Device username
- `MQTT_PASSWORD`: Device password
- `MQTT_DEVICE_TOPIC`: e.g., `home/audio/device-001`
- `HMAC_SECRET`: (Optional) Shared secret for message signing
- `SIGNED_URL_EXPIRES`: (Optional) Default: 300 seconds

### ESP32 Firmware (menuconfig)
- `CONFIG_WIFI_SSID`: WiFi network name
- `CONFIG_WIFI_PASSWORD`: WiFi password
- `CONFIG_MQTT_BROKER_URI`: e.g., `mqtts://abc123.hivemq.cloud:8883`
- `CONFIG_MQTT_USERNAME`: MQTT username
- `CONFIG_MQTT_PASSWORD`: MQTT password
- `CONFIG_MQTT_DEVICE_TOPIC`: e.g., `home/audio/device-001`
- `CONFIG_HMAC_SECRET`: (Optional) Shared secret
