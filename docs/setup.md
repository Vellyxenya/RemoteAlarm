# Setup Guide

## Prerequisites

- Node.js and npm
- Flutter SDK
- ESP-IDF (v5.x)
- ESP-ADF
- Firebase CLI
- Firebase account
- HiveMQ Cloud account

## Phase 0: Firebase Setup

### 1. Create Firebase Project
```bash
# Install Firebase CLI
npm install -g firebase-tools

# Login to Firebase
firebase login

# Create new project in Firebase Console
# https://console.firebase.google.com
```

### 2. Enable Firebase Services
- Firebase Storage
- Firebase Cloud Functions
- Firebase Authentication (Anonymous)

### 3. Configure Storage Rules
Navigate to Firebase Console → Storage → Rules:
```js
rules_version = '2';
service firebase.storage {
  match /b/{bucket}/o {
    match /audio/{allPaths=**} {
      allow write: if request.auth != null;
      allow read: if false;
    }
  }
}
```

## Phase 1: Flutter App Setup

```bash
cd mobile
flutter create .
flutter pub add firebase_core firebase_auth firebase_storage record path_provider
```

Configure Firebase:
1. Download `google-services.json` (Android) and `GoogleService-Info.plist` (iOS)
2. Place in appropriate directories
3. Update app configuration

## Phase 2: Cloud Functions Setup

```bash
cd functions
firebase init functions
npm install mqtt
```

Set environment variables:
```bash
firebase functions:config:set \
  mqtt.broker="<hivemq-url>" \
  mqtt.username="<username>" \
  mqtt.password="<password>" \
  mqtt.topic="home/audio/<device-id>"
```

Deploy:
```bash
firebase deploy --only functions
```

## Phase 3: MQTT Broker Setup

1. Create HiveMQ Cloud free cluster
2. Enable TLS (port 8883)
3. Create credentials for ESP32 device
4. Configure topic ACL:
   - Device can subscribe to: `home/audio/<device-id>`
   - Cloud Function can publish to: `home/audio/#`

## Phase 4: ESP32 Firmware Setup

```bash
cd firmware
idf.py menuconfig
```

Configure:
- WiFi SSID and password
- MQTT broker URL
- MQTT credentials
- Device topic

Build and flash:
```bash
idf.py build
idf.py -p <PORT> flash monitor
```

## Testing

1. Launch ESP32 device (should connect to WiFi and MQTT)
2. Open Flutter app
3. Record short audio message
4. Upload via app
5. Verify ESP32 receives notification and plays audio
