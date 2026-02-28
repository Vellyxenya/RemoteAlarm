# tasks.md

## Progress Board

### Completed
- **Bootstrap Project** (2026-02-09)
  -  Created folder structure for mobile, functions, firmware, and docs
  -  Added README files for each component
  -  Created comprehensive documentation
  -  Updated root README with project overview

- **Phase 0 - Firebase Setup** (2026-02-09)
  -  Task 0.1: Created Firebase Project
    - Created Firebase project in Firebase Console
    - Enabled Firebase Storage, Cloud Functions, Authentication
  -  Task 0.2: Configured Authentication
    - Enabled Anonymous Auth for MVP
  -  Task 0.3: Locked Down Storage Rules
    - Deployed storage.rules with secure configuration
    - Only authenticated users can upload to /audio/*
    - No direct read access (signed URLs only)

- **Phase 1 - Flutter App (Record + Upload)** (2026-02-09)
  -  Task 1.1: Create Flutter App
    - Initialized Flutter project with Firebase dependencies
    - Added firebase_core, firebase_auth, firebase_storage
    - Added record, path_provider, uuid, permission_handler
  -  Task 1.2: Record Audio (15 sec)
    - Implemented AudioService with record plugin
    - WAV format, 16kHz sample rate
    - Microphone permission handling
  -  Task 1.3: Upload Audio to Firebase Storage
    - Implemented StorageService with Firebase Storage
    - Anonymous authentication
    - UUID-based filenames (audio/<uuid>.wav)
    - Tested successfully on Android emulator

- **Phase 2 - Cloud Function (Trigger + MQTT Publish)** (2026-02-10)
  -  Task 2.1-2.4: Implement Cloud Function with MQTT Publishing
    - Created onAudioUpload function triggered by Storage onFinalize
    - Generates signed URLs (10 min expiration) for secure downloads
    - Publishes MQTT messages to HiveMQ Cloud broker via TLS
    - Migrated to firebase-functions/params for configuration
    - Separate HiveMQ users: firebase-function (publish), esp32-device1 (subscribe)
    - Unique client IDs per function invocation to prevent conflicts
    - End-to-end pipeline verified: Flutter â†’ Storage â†’ Function â†’ MQTT

- **Phase 3 - MQTT Broker Setup (HiveMQ)** (2026-02-10)
  -  Task 3.1: Create HiveMQ Cloud Instance
    - Free tier HiveMQ Cloud cluster created
    - TLS/MQTTS enabled on port 8883
  -  Task 3.2: Create Device Credentials
    - Created firebase-function user for Cloud Function publishing
    - Created esp32-device1 user for device subscription and monitoring
  -  Task 3.3: Topic ACL Restrictions
    - Configured access controls for topic home/audio/device1
    - Tested and verified message delivery

### In Progress
- Phase 4 - ESP32 Firmware (Subscribe + Download + Play)
  - Task 4.1: Choose Board + Framework
    - Target board: Waveshare ESP32-S3 AI Smart RGB Speaker Dev Board
    - Use: ESP-IDF v5.2+ (Transitioned from ESP-ADF for better stability)
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
  - Task 4.6: Audio Playback (ESP-IDF v5.2 Standard I2S Mode)
    - Playback pipeline: HTTPS stream reader -> Ring Buffer -> I2S Standard Mode Output
    - Goal: Stream and play audio immediately without full file buffering.
  - Task 4.7: Prevent Replay / Spam
    - Ignore duplicate timestamps
    - Only accept messages with valid signature (optional)
  - Task 4.8: Audio Pipeline Optimization (Completed 2026-02-27)
    - Forced Mono Mode for Max98357A (Initialization and Playback)
    - Fixed Post-Playback Clicking (DMA buffer clearing + I2S Disable)
    - Implemented Dual-Task Ring-Buffered Streaming (64KB RB)
    - Added Jitter Buffer (32KB start threshold)
    - Fixed Race Conditions and Panics in Ring Buffer Cleanup
    - Added 1.2s DMA Drain Delay to prevent early audio cutoff
    - Resolved `i2s_std_slot_config_t` compilation error
    - Added WAV Format Validation (Fixed 16/32-bit check)

- Phase 5 - MVP Testing Checklist
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

### Backlog
- Phase 2 - Cloud Function (Trigger + MQTT Publish)
  - Task 2.1: Initialize Firebase Functions
    - firebase init functions
    - Use Node.js runtime.
  - Task 2.2: Trigger on Audio Upload
    - Cloud Function should run on: Storage onFinalize
    - Example:
      `
      exports.onAudioUpload =
      functions.storage.object().onFinalize(async (object) => {
        // handle new audio file
      });
      `
  - Task 2.3: Generate Signed Download URL
    - Create signed URL valid for ~5 minutes
    - Goal: ESP32 can download securely without bucket access
  - Task 2.4: Publish MQTT Message
    - Topic: home/audio/<device-id>
    - Payload example:
      `
      {
        \"file_url\": \"<signed_url>\",
        \"timestamp\": \"<iso_time>\"
      }
      `
    - Use MQTT over TLS
    - Broker: HiveMQ Cloud
    - Port: 8883
  - Task 2.5 (Optional): Add HMAC Signature
    - Cloud Function signs payload:
      `
      {
        \"file_url\": \"...\",
        \"sig\": \"HMAC(...)\"
      }
      `
    - ESP32 verifies before downloading.

- Phase 3 - MQTT Broker Setup (HiveMQ)
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

