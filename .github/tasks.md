# tasks.md

## Progress Board

### Completed
- **Bootstrap Project** (2026-02-09)
  -  Created folder structure for mobile, functions, firmware, and docs
  -  Added README files for each component
  -  Created comprehensive documentation
  -  Updated root README with project overview

- **Phase 0  Setup** (2026-02-09)
  -  Task 0.1: Created Firebase Project
    - Created Firebase project in Firebase Console
    - Enabled Firebase Storage, Cloud Functions, Authentication
  -  Task 0.2: Configured Authentication
    - Enabled Anonymous Auth for MVP
  -  Task 0.3: Locked Down Storage Rules
    - Deployed storage.rules with secure configuration
    - Only authenticated users can upload to /audio/*
    - No direct read access (signed URLs only)

### In Progress
- **Phase 1  Flutter App (Record + Upload)**
  - Task 1.1: Create Flutter App
  - Task 1.2: Record Audio (15 sec)
  - Task 1.3: Upload Audio to Firebase Storage

### Backlog
- Phase 1  Flutter App (Record + Upload)
- Task 1.1: Create Flutter App
- Initialize Flutter project
- Add Firebase dependencies:
- firebase_core: latest
- firebase_auth: latest
- firebase_storage: latest
- record: latest
- path_provider: latest
- Task 1.2: Record Audio (15 sec)
- Use record plugin
- Save locally as WAV (preferred)
- Example:
- Button: Start recording
- Button: Stop recording
- Task 1.3: Upload Audio to Firebase Storage
- Upload flow:
- Generate UUID filename
- Upload to: audio/<uuid>.wav
- Flutter upload snippet:
```
final ref = FirebaseStorage.instance.ref("audio/$fileName.wav");
await ref.putFile(audioFile);
```
- Goal: Upload triggers Cloud Function automatically

- Phase 2  Cloud Function (Trigger + MQTT Publish)
- Task 2.1: Initialize Firebase Functions
- firebase init functions
- Use Node.js runtime.
- Task 2.2: Trigger on Audio Upload
- Cloud Function should run on: Storage onFinalize
- Example:
```
exports.onAudioUpload =
functions.storage.object().onFinalize(async (object) => {
// handle new audio file
});
```
- Task 2.3: Generate Signed Download URL
- Create signed URL valid for ~5 minutes
- Goal: ESP32 can download securely without bucket access
- Task 2.4: Publish MQTT Message
- Topic: home/audio/<device-id>
- Payload example:
```
{
"file_url": "<signed_url>",
"timestamp": "<iso_time>"
}
```
- Use MQTT over TLS
- Broker: HiveMQ Cloud
- Port: 8883
- Task 2.5 (Optional): Add HMAC Signature
- Cloud Function signs payload:
```
{
"file_url": "...",
"sig": "HMAC(...)"
}
```
- ESP32 verifies before downloading.

- Phase 3  MQTT Broker Setup (HiveMQ)
- Task 3.1: Create HiveMQ Cloud Instance
- Free tier is enough
- Enable TLS (MQTTS)
- Task 3.2: Create Device Credentials
- Username/password for ESP32 device only
- Example:
- user: device123
- pass: strong_password
- Task 3.3: Topic ACL Restrictions
- Ensure device can only subscribe to: home/audio/device123
- No wildcard access.
- Goal: Random clients cannot listen in.

- Phase 4  ESP32 Firmware (Subscribe + Download + Play)
- Task 4.1: Choose Board + Framework
- Target board: Waveshare ESP32-S3 AI Smart RGB Speaker Dev Board
- Use: ESP-IDF + ESP-ADF
- Task 4.2: Connect to Wi-Fi
- WPA2 network credentials stored securely
- Task 4.3: Connect to MQTT Broker via TLS
- Subscribe: home/audio/device123
- Use: MQTTS (TLS)
- Username/password stored in encrypted NVS if possible
- Task 4.4: Parse MQTT Payload
- Extract: signed audio URL, timestamp, optional signature
- Task 4.5: Download Audio via HTTPS
- HTTPS GET signed URL
- Stream directly (no full file buffering)
- Task 4.6: Play Audio with ESP-ADF
- Playback pipeline: HTTP stream reader  WAV decoder  I2S output
- Goal: Play message immediately after notification
- Task 4.7: Prevent Replay / Spam
- Ignore duplicate timestamps
- Only accept messages with valid signature (optional)

- Phase 5  MVP Testing Checklist
- End-to-End Test
- Record audio in Flutter app
- Upload succeeds
- Cloud Function triggers
- MQTT message published
- ESP32 receives notification
- ESP32 downloads audio
- Speaker plays message

- Future Improvements
- Multiple devices (topic per device)
- Message queue + retry
- Local caching on SD card
- Add Text-to-Speech (TTS) instead of recorded audio
- OTA firmware updates + secure provisioning
- Replace HiveMQ with AWS IoT Core for certificate-based auth