# Project Context

## Summary
- Purpose:
  Build a small audio notification system where a Flutter mobile app records a short voice message (1–5 seconds), uploads it to the cloud, and an ESP32-S3 speaker device automatically plays it when notified.

- Users:
  - Primary user: the mobile app owner sending voice notifications
  - Device: a single ESP32-S3 audio-capable board acting as a notification speaker

- Constraints:
  - ESP32 device has no user interface, so credentials must be handled securely
  - Low maintenance cost (serverless preferred)
  - Secure communication (no public file access, no open MQTT broker)
  - Audio messages are short and lightweight (WAV or MP3)

---

## Architecture

- Components:
  - Flutter Mobile App
    - Records short audio clips
    - Uploads audio to Firebase Storage via HTTPS
    - Does NOT connect directly to MQTT

  - Firebase Storage (Private)
    - Stores uploaded audio files (UUID filenames)
    - Triggers Cloud Function on upload

  - Firebase Cloud Function (Serverless Trigger)
    - Runs automatically when a new audio file is uploaded
    - Generates a short-lived signed download URL
    - Publishes an MQTT message to the ESP32 device topic

  - MQTT Broker (HiveMQ Cloud)
    - Secure MQTT over TLS (MQTTS)
    - Requires username/password authentication
    - Topic ACL restricts device access

  - ESP32-S3 Audio Speaker Board (e.g., Waveshare AI Smart Speaker Board)
    - Subscribes to its MQTT topic
    - Receives signed URL
    - Downloads audio over HTTPS
    - Plays audio via ESP-ADF pipeline

---

- Data flow:
  1. User records a short voice message in Flutter app
  2. App uploads audio file to Firebase Storage (HTTPS)
  3. Firebase Cloud Function triggers on upload
  4. Cloud Function generates signed URL (expires quickly)
  5. Cloud Function publishes MQTT notification with signed URL
  6. ESP32 receives MQTT message
  7. ESP32 downloads audio file via HTTPS signed URL
  8. ESP32 plays audio through onboard speaker

---

- External services:
  - Firebase Storage (file storage)
  - Firebase Cloud Functions (serverless trigger + logic)
  - HiveMQ Cloud MQTT Broker (device notification channel)

---

## Environment
- OS: Windows
- Languages:
  - Dart (Flutter app)
  - JavaScript/Node.js (Firebase Cloud Functions)
  - C/C++ (ESP32 firmware using ESP-IDF)
- Frameworks:
  - Flutter
  - Firebase SDK
  - ESP-IDF + ESP-ADF (audio playback)
  - MQTT client libraries

---

## Notes

- Security approach:
  - Mobile app never stores MQTT credentials
  - Firebase Storage bucket is private (no public downloads)
  - Cloud Function generates signed URLs with short expiration
  - MQTT uses TLS encryption + username/password
  - Topic ACL ensures only the intended ESP32 device can subscribe
  - Optional future improvement: HMAC signing of MQTT payloads

---

## Text Architecture Diagram

┌────────────────────────────────────────────┐
│              Mobile App (Flutter)          │
│                                            │
│  - Record 1–5 sec voice message            │
│  - Encode WAV (or MP3)                     │
│  - Upload file via HTTPS                   │
│  - No MQTT credentials inside the app      │
└───────────────────────┬────────────────────┘
                        │ HTTPS Upload (secure)
                        ▼
┌────────────────────────────────────────────┐
│          Firebase Storage (Private)        │
│                                            │
│  - Stores audio file                       │
│  - Filename = UUID (unguessable)           │
│  - Storage Rules prevent public access     │
│  - Upload triggers Cloud Function          │
└───────────────────────┬────────────────────┘
                        │ Trigger: onFinalize()
                        ▼
┌────────────────────────────────────────────┐
│     Firebase Cloud Function (Serverless)   │
│                                            │
│  On new audio upload:                      │
│   1. Generate signed download URL          │
│      (expires in e.g. 1–5 min)             │
│   2. Publish MQTT message                  │
│      Topic: home/audio/<device-id>         │
│   3. Optional: Add HMAC signature          │
└───────────────────────┬────────────────────┘
                        │ MQTT Publish (TLS)
                        ▼
┌────────────────────────────────────────────┐
│         MQTT Broker (HiveMQ Cloud)         │
│                                            │
│  - MQTTS encryption (TLS, port 8883)       │
│  - Username/password required              │
│  - Topic ACLs:                             │
│      Device can ONLY read its own topic    │
│      home/audio/<device-id>                │
└───────────────────────┬────────────────────┘
                        │ MQTT Subscribe (TLS)
                        ▼
┌────────────────────────────────────────────┐
│   ESP32-S3 Audio Speaker Dev Board         │
│   (Waveshare AI Smart Speaker Board)       │
│                                            │
│  - Connect Wi-Fi (WPA2)                    │
│  - Subscribe to MQTT topic                 │
│  - Receives signed URL                     │
│  - Optional: Verify HMAC payload           │
│  - Download audio via HTTPS                │
│  - Play using ESP-ADF audio pipeline       │
│  - No permanent storage bucket access      │
└───────────────────────┬────────────────────┘
                        │ Audio Output
                        ▼
                 ┌───────────────┐
                 │    Speaker    │
                 └───────────────┘
