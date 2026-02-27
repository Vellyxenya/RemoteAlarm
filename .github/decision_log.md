- 2026-02-27 12:10:00 : Fixed compilation error in `audio_playback_task` where `i2s_slot_config_t` was used instead of the correct `i2s_std_slot_config_t` type for ESP-IDF v5.2.x.
- 2026-02-27 12:00:00 : Implemented a 32KB "Jitter Buffer" (50% of the expanded 64KB Ring Buffer) and suppressed redundant I2S log errors. The playback now waits until the buffer reaches a threshold before starting the I2S writer task, which eliminates micro-stuttering caused by network jitter and ensures smooth sequential audio playback.
- 2026-02-27 11:45:00 : Resolved the "half audio" (early cutoff) bug by adding a 1.2-second drain delay to the I2S writer task. This ensures the hardware DMA buffer, which can store up to ~1s of audio, fully plays before the channel is disabled. Cleaned up reconfiguration logic to be more robust across repeated playbacks.
- 2026-02-27 11:30:00 : Fixed a race condition and "spinlock_acquire" panic during audio playback. Implemented a proper task handoff using `TaskHandle_t` to ensure the Ring Buffer is only deleted after the I2S writer task has fully completed its execution. Replaced the "buffer free size" polling logic with a robust task-completion check.
- 2026-02-27 11:15:00 : Replaced "Download then Play" with "Ring-Buffered Streaming". Fixed OOM crash by using a fixed 48KB ring buffer instead of trying to buffer the entire file in internal RAM. Used a dual-task architecture: a high-priority downloader task (fills RB) and a very high-priority I2S writer task (drains RB). This eliminates network-induced stuttering and handles arbitrary file sizes.
- 2026-02-27 10:45:00 : Implemented "Download then Play" strategy to eliminate audio stuttering. Increased I2S DMA descriptors to 32 and frame depth to 480. Moved playback to a dedicated high-priority (P10) task and optimized the stereo-to-mono downmixing loop. Enabled PSRAM buffering (SPIRAM) for the audio files to avoid network gaps during playback.
- 2026-02-27 10:25:00 : Hardened `play_audio` by making I2S status checks more robust. Replaced `ESP_ERROR_CHECK(i2s_channel_disable)` with a manual check for `ESP_ERR_INVALID_STATE` so that playing a second audio doesn't crash the ESP32 (since the channel is disabled at the end of each playback).
- 2026-02-27 10:15:00 : Added a check to disable the I2S channel immediately after audio playback. This resolves the manual "machine-gun" clicking (I2S DMA buffer ghosting) by stopping the peripheral when not in use.
- 2026-02-27 10:00:00 : Updated I2S configuration to force Mono mode in both initialization and playback. This is to better support the Max98357A mono amplifier and reduce unnecessary data processing/power consumption on the 3.3V line. Added simple downmixing in the playback loop if the WAV file is stereo.
- 2026-02-10 13:54:14 : Resolved ESP-IDF compiler discovery issues by switching to v5.2, manually removing the clang compiler folder from C:\Espressif\tools\, and setting IDF_TOOLCHAIN to 'gcc' via setx. Fixed Kconfig parsing issues by stripping PowerShell BOM from Kconfig.projbuild.
- 2026-02-10 11:53:22 : Refactored ESP32 firmware to use Kconfig for all credentials (WiFi SSID/Pass, MQTT URL/User/Pass/Topic). This prevents sensitive data from being hardcoded in main.c and committed to the repo.
- 2026-02-10 11:51:13 : Finalized ESP32 firmware logic with ESP-ADF, including WiFi, MQTT (TLS), and audio pipeline (HTTP -> WAV -> I2S). Created CMake project structure.
# Decision Log

## Template
- Date:
- Time (UTC+8):
- Decision:
- Rationale:
- Alternatives:
- Impact:

## Entries (newest first)
- Date: 2026-02-10
  Time (UTC+8): 11:30
  Decision: Rotated Firebase API keys following a security leak detection.
  Rationale: GitHub/Google detected a publicly accessible API key in firebase_options.dart. Although Firebase keys are meant to be in the app, it is best practice to rotate them once flagged and apply strict application restrictions.
  Alternatives: None (security best practice).
  Impact: New keys committed to repo. Old keys must be deleted in Google Cloud Console. App requires redeploy with new keys. Security state improved with restricted key usage.

- Date: 2026-02-10
  Time (UTC+8): 11:15
  Decision: Complete Phase 3 - MQTT Broker Setup (HiveMQ).
  Rationale: Set up HiveMQ Cloud free tier cluster with MQTTS on port 8883. Created two separate users for security: firebase-function (publish only) and esp32-device1 (subscribe for device and monitoring). Configured topic restrictions for home/audio/device1. Tested message delivery successfully through web client.
  Alternatives: 1) Use single user for both publish and subscribe (causes connection conflicts). 2) Self-host Mosquitto broker (more complex, requires server management). 3) AWS IoT Core (certificate-based auth, overkill for MVP).
  Impact: Secure MQTT broker operational. Separate credentials prevent connection conflicts between publisher and subscribers. Free tier sufficient for MVP. Ready for ESP32 firmware to connect and receive notifications.

- Date: 2026-02-10
  Time (UTC+8): 11:00
  Decision: Complete Phase 2 - Cloud Function (Trigger + MQTT Publish).
  Rationale: Implemented Cloud Function that triggers on Firebase Storage uploads, generates signed URLs, and publishes to HiveMQ broker via MQTTS. Migrated to firebase-functions/params package instead of deprecated functions.config(). Fixed MQTT timeout issues by using QoS 0, unique client IDs, and improved timeout handling. Created separate HiveMQ users (firebase-function for publishing, esp32-device1 for subscribing) to prevent connection conflicts. End-to-end pipeline verified working: Flutter app -> Firebase Storage -> Cloud Function -> HiveMQ MQTT broker.
  Alternatives: 1) Use QoS 1 for guaranteed delivery (slower, timed out). 2) Use same MQTT credentials for function and monitoring (causes disconnects). 3) Keep deprecated functions.config() (would break in March 2026).
  Impact: Complete audio notification pipeline from mobile to MQTT. ESP32 firmware can now subscribe to receive audio URLs. Signed URLs provide secure time-limited access to audio files without exposing storage directly. Ready for Phase 4 ESP32 implementation.

- Date: 2026-02-10
  Time (UTC+8): 14:45
  Decision: Migrate Cloud Functions from deprecated functions.config() to params package.
  Rationale: Firebase deprecated functions.config() and Runtime Config service, scheduled for shutdown in March 2026. Migration required to prevent deployment failures. Using firebase-functions/params with defineString() provides type-safe, documented environment parameters with defaults.
  Alternatives: 1) Enable legacy commands temporarily. 2) Use environment variables directly via process.env.
  Impact: Functions code updated to use params API. Deployment command changed to include --set-params. Local development uses .env file. Documentation updated.

- Date: 2026-02-09
  Time (UTC+8): 16:30
  Decision: Complete Phase 1 - Flutter mobile app with audio recording and Firebase Storage upload.
  Rationale: Built fully functional Flutter app with AudioService for WAV recording (16kHz) and StorageService for Firebase uploads with anonymous auth. App tested successfully on Android emulator.
  Alternatives: Could have used different audio formats (AAC, MP3).
  Impact: Mobile app complete and ready for integration with Cloud Functions.

- Date: 2026-02-09
  Time (UTC+8): 14:00
  Decision: Complete Phase 0 - Firebase backend setup with Storage security rules.
  Rationale: Configured Firebase project with Storage, Authentication (Anonymous), and deployed security rules.
  Impact: Backend infrastructure ready for Phase 1 (Flutter app).

- Date: 2026-02-09
  Time (UTC+8): 12:00
  Decision: Bootstrap project with organized folder structure.
  Rationale: Establish clear separation of concerns (mobile, functions, firmware, docs).
  Impact: Clear project organization makes onboarding easier.

- Date: 2026-02-09
  Time (UTC+8): 00:00
        Decision: Standardize decision log timestamps to UTC+8.
        Rationale: Align timestamps with preferred timezone (Taipei).

- Date: 2026-02-09
        Time (UTC+8): 00:00
        Decision: Expand .gitignore for Flutter and Firebase.
        Rationale: Prevent build artifacts from being committed.

- Date: 2026-02-09
        Time (UTC+8): 00:00
        Decision: Use CODEOWNERS as the reviewer source.
        Rationale: Avoid hardcoding usernames in instructions.

- Date: 2026-02-09
        Time (UTC+8): 00:00
        Decision: Standardize AI branch/PR workflow.
        Rationale: Ensure traceability and consistent review.

- Date: 2026-02-09
        Time (UTC+8): 00:00
        Decision: Require updating tasks.md on completion.
        Rationale: Ensure progress tracking is maintained.

- Date: 2026-02-09
        Time (UTC+8): 00:00
        Decision: Merge workflow guidance into project instructions.
        Rationale: Ensure workflow rules always apply.

- Date: 2026-02-09
        Time (UTC+8): 00:00
        Decision: Update workflow artifacts list.
        Rationale: Keep documentation aligned with actual files.

- Date: 2026-02-09
        Time (UTC+8): 00:00
        Decision: Move full project plan into Backlog section.
        Rationale: Treat plan as initial backlog.

- Date: 2026-02-09
        Time (UTC+8): 00:00
        Decision: Reintroduce progress board sections in tasks.
        Rationale: Track backlog, in-progress, and completed work.

- Date: 2026-02-09
        Time (UTC+8): 00:00
        Decision: Populate tasks with full project plan.
        Rationale: Establishes a shared roadmap.

- Date: 2026-02-09
        Time (UTC+8): 00:00
        Decision: Store AI guidance in .github.
        Rationale: Aligns with VS Code and prompt file conventions.
