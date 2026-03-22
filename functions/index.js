const {onObjectFinalized} = require("firebase-functions/v2/storage");
const {onCall, HttpsError} = require("firebase-functions/v2/https");
const {setGlobalOptions} = require("firebase-functions/v2");
const {defineString} = require("firebase-functions/params");
const admin = require("firebase-admin");
const logger = require("firebase-functions/logger");
const mqtt = require("mqtt");

// Define environment parameters with defaults
const mqttBrokerUrl = defineString("MQTT_BROKER_URL", {
  description: "MQTT broker URL (mqtts://...)",
  default: "",
});

const mqttUsername = defineString("MQTT_USERNAME", {
  description: "MQTT broker username",
  default: "",
});

const mqttPassword = defineString("MQTT_PASSWORD", {
  description: "MQTT broker password",
  default: "",
});

const mqttDeviceTopic = defineString("MQTT_DEVICE_TOPIC", {
  description: "MQTT topic for device notifications",
  default: "home/audio/device1",
});

// Initialize Firebase Admin
admin.initializeApp();

setGlobalOptions({maxInstances: 10});

/**
 * Cloud Function triggered when audio file is uploaded to Firebase Storage
 * Generates signed URL and publishes MQTT notification to ESP32 device
 */
exports.onAudioUpload = onObjectFinalized({
  bucket: "remotealarm-be4d7.firebasestorage.app",
}, async (event) => {
  const filePath = event.data.name;
  logger.info("Audio file uploaded", {filePath});

  // Only process files in the audio/ directory
  if (!filePath.startsWith("audio/")) {
    logger.info("Ignoring non-audio file", {filePath});
    return null;
  }

  try {
    // Generate signed URL valid for 10 minutes
    const bucket = admin.storage().bucket(event.data.bucket);
    const file = bucket.file(filePath);

    const [url] = await file.getSignedUrl({
      action: "read",
      expires: Date.now() + 10 * 60 * 1000, // 10 minutes
    });

    logger.info("Generated signed URL", {filePath, url});

    // Prepare MQTT payload
    const payload = {
      file_url: url,
      timestamp: new Date().toISOString(),
      filename: filePath.split("/").pop(),
    };

    // Publish to MQTT broker
    await publishToMQTT(payload);

    logger.info("MQTT message published successfully", {payload});
    return null;
  } catch (error) {
    logger.error("Error processing audio upload", {error, filePath});
    throw error;
  }
});

/**
 * Cloud Function to replay the last uploaded audio message (or a specific file)
 * Can be called directly from the client application.
 */
exports.replayLastMessage = onCall({
  cors: true,
}, async (request) => {
  // Check if authenticated (optional, but good practice)
  // if (!request.auth) {
  //   throw new HttpsError('failed-precondition',
  //  'The function must be called while authenticated.');
  // }

  const bucketName = "remotealarm-be4d7.firebasestorage.app";
  const bucket = admin.storage().bucket(bucketName);

  // Safely access data, defaulting to empty object if null
  const data = request.data || {};
  let filePath = data.filePath;

  try {
    // If no file path provided, find the most recent file in audio/
    if (!filePath) {
      const [files] = await bucket.getFiles({
        prefix: "audio/",
        maxResults: 100, // Limit to reasonable number to sort
      });

      if (files.length === 0) {
        logger.info("No audio files found to replay");
        return {success: false, message: "No audio files found"};
      }

      // Sort by timeCreated descending with safety check
      files.sort((a, b) => {
        const timeA = (a.metadata && a.metadata.timeCreated) ?
            new Date(a.metadata.timeCreated).getTime() : 0;
        const timeB = (b.metadata && b.metadata.timeCreated) ?
            new Date(b.metadata.timeCreated).getTime() : 0;
        return timeB - timeA;
      });

      filePath = files[0].name;
      logger.info("Found latest file to replay", {filePath});
    }

    // Reuse logic to generate URL and publish MQTT
    await processAudioNotification(bucket, filePath);

    return {success: true, message: "Replay triggered", filePath};
  } catch (error) {
    logger.error("Error replaying message", {error});
    // Return specific error message to client
    throw new HttpsError(
        "internal",
        error.message || "Unable to replay message",
        {details: error.toString()},
    );
  }
});

/**
 * Shared logic to generate signed URL and publish MQTT notification
 * @param {Bucket} bucket - Storage bucket instance
 * @param {string} filePath - Path to the audio file
 */
async function processAudioNotification(bucket, filePath) {
  const file = bucket.file(filePath);

  // Check if file exists
  const [exists] = await file.exists();
  if (!exists) {
    throw new Error(`File ${filePath} does not exist`);
  }

  // Generate signed URL valid for 10 minutes
  const [url] = await file.getSignedUrl({
    action: "read",
    expires: Date.now() + 10 * 60 * 1000,
  });

  logger.info("Generated signed URL for replay", {filePath, url});

  // Prepare MQTT payload
  const payload = {
    file_url: url,
    timestamp: new Date().toISOString(),
    filename: filePath.split("/").pop(),
  };

  // Publish to MQTT broker
  await publishToMQTT(payload);
}

/**
 * Publish message to MQTT broker
 * @param {Object} payload - Message payload to publish
 * @return {Promise} - Resolves when message is published
 */
async function publishToMQTT(payload) {
  return new Promise((resolve, reject) => {
    // Get MQTT configuration from params
    const brokerUrl = mqttBrokerUrl.value();
    const username = mqttUsername.value();
    const password = mqttPassword.value();
    const topic = mqttDeviceTopic.value();

    if (!brokerUrl || !username || !password) {
      const error =
        new Error("MQTT configuration missing. Check defineString params.");
      logger.error("MQTT configuration not set", {
        hasBrokerUrl: !!brokerUrl,
        hasUsername: !!username,
        hasPassword: !!password,
      });
      reject(error);
      return;
    }

    logger.info("Connecting to MQTT broker", {brokerUrl, topic});

    // Connect to MQTT broker with TLS
    const client = mqtt.connect(brokerUrl, {
      username,
      password,
      protocol: "mqtts", // Use TLS
      port: 8883,
      rejectUnauthorized: true,
      reconnectPeriod: 0, // Disable reconnect for cloud functions
      connectTimeout: 5000,
      clientId: `cloud-function-${Date.now()}`, // Unique ID per invocation
    });

    // Timeout after 15 seconds
    const timeoutId = setTimeout(() => {
      logger.error("MQTT operation timed out");
      client.end(true); // Force close
      reject(new Error("MQTT operation timeout"));
    }, 15000);

    client.on("connect", () => {
      logger.info("Connected to MQTT broker");

      // Publish message with QoS 0 for faster delivery
      client.publish(topic, JSON.stringify(payload), {qos: 0}, (error) => {
        clearTimeout(timeoutId);
        client.end();

        if (error) {
          logger.error("Failed to publish MQTT message", {error});
          reject(error);
        } else {
          logger.info("MQTT message published", {topic, payload});
          resolve();
        }
      });
    });

    client.on("error", (error) => {
      clearTimeout(timeoutId);
      logger.error("MQTT connection error", {error: error.message});
      client.end();
      reject(new Error(`MQTT Connection Error: ${error.message}`));
    });
  });
}
