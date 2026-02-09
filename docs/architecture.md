# Architecture Overview

## System Components

### 1. Flutter Mobile App
- Records 1-5 second audio clips
- Uploads to Firebase Storage
- Uses anonymous authentication

### 2. Firebase Storage
- Private bucket for audio files
- Triggers Cloud Function on upload
- Files stored with UUID filenames

### 3. Firebase Cloud Function
- Triggered automatically on audio upload
- Generates signed download URL (5-minute expiration)
- Publishes MQTT message with URL

### 4. MQTT Broker (HiveMQ Cloud)
- Secure MQTT over TLS (port 8883)
- Username/password authentication
- Topic ACL for device isolation

### 5. ESP32-S3 Device
- Subscribes to MQTT topic
- Downloads audio via HTTPS
- Plays audio through speaker

## Data Flow

```
User → Flutter App → Firebase Storage → Cloud Function → MQTT Broker → ESP32 → Audio Playback
        (Record)     (Upload HTTPS)      (Trigger)       (Publish)     (Download)  (Play)
```

## Security Model

1. **No Public File Access**: Firebase Storage bucket is private
2. **Short-Lived URLs**: Signed URLs expire in 5 minutes
3. **Encrypted Communication**: MQTT over TLS, HTTPS downloads
4. **Authentication**: Anonymous auth for app, credentials for MQTT
5. **Topic ACL**: Device can only subscribe to its specific topic
6. **Optional HMAC**: Payload signing for additional verification

## Technology Stack

- **Mobile**: Flutter (Dart)
- **Backend**: Firebase Cloud Functions (Node.js)
- **Storage**: Firebase Storage
- **Messaging**: HiveMQ Cloud (MQTT)
- **Firmware**: ESP-IDF + ESP-ADF (C/C++)
