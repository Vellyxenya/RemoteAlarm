# RemoteAlarm

A secure audio notification system that allows a Flutter mobile app to record short voice messages and automatically play them on an ESP32-S3 speaker device.

## Overview

RemoteAlarm enables remote voice notifications through a serverless architecture:
- **Mobile App** (Flutter): Record 1-5 second audio clips
- **Cloud Backend** (Firebase): Store audio and trigger notifications
- **Device** (ESP32-S3): Receive and play audio messages

## Architecture

```
User → Flutter App → Firebase Storage → Cloud Function → MQTT Broker → ESP32 → Speaker
```

### Components

1. **Mobile App** ([mobile/](mobile/))
   - Record short audio messages
   - Upload to Firebase Storage via HTTPS
   - Anonymous authentication

2. **Cloud Functions** ([functions/](functions/))
   - Triggered on audio upload
   - Generate signed download URLs
   - Publish MQTT notifications

3. **ESP32 Firmware** ([firmware/](firmware/))
   - Subscribe to MQTT topic
   - Download audio via HTTPS
   - Play through built-in speaker

4. **External Services**
   - Firebase Storage (private bucket)
   - Firebase Cloud Functions (serverless)
   - HiveMQ Cloud (MQTT broker with TLS)

## Security

- Private storage bucket (no public downloads)
- Short-lived signed URLs (5-minute expiration)
- MQTT over TLS with authentication
- Topic ACL for device isolation
- Optional HMAC message signing

See [docs/security.md](docs/security.md) for details.

## Getting Started

### Prerequisites
- Node.js and npm
- Flutter SDK
- ESP-IDF (v5.x) + ESP-ADF
- Firebase account
- HiveMQ Cloud account

### Quick Setup

1. **Firebase Setup**
   ```bash
   firebase login
   # Create project in Firebase Console
   # Enable Storage, Functions, Authentication
   ```

2. **Mobile App**
   ```bash
   cd mobile
   flutter create .
   flutter pub get
   ```

3. **Cloud Functions**
   ```bash
   cd functions
   firebase init functions
   npm install
   firebase deploy --only functions
   ```

4. **ESP32 Firmware**
   ```bash
   cd firmware
   idf.py menuconfig  # Configure WiFi and MQTT
   idf.py build flash monitor
   ```

See [docs/setup.md](docs/setup.md) for detailed instructions.

## Documentation

- [Architecture Overview](docs/architecture.md)
- [Setup Guide](docs/setup.md)
- [Security Considerations](docs/security.md)
- [API Reference](docs/api.md)

## Project Structure

```
RemoteAlarm/
├── mobile/           # Flutter mobile app
├── functions/        # Firebase Cloud Functions
├── firmware/         # ESP32-S3 firmware
│   ├── main/        # Main application
│   └── components/  # Custom components
├── docs/            # Documentation
└── .github/         # Project management
```

## Development Status

See [.github/tasks.md](.github/tasks.md) for current progress and planned features.

## License

See [LICENSE](LICENSE) file for details.