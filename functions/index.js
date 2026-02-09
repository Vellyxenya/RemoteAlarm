const {onObjectFinalized} = require("firebase-functions/v2/storage");
const {setGlobalOptions} = require("firebase-functions/v2");
const admin = require("firebase-admin");
const logger = require("firebase-functions/logger");
const mqtt = require("mqtt");

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
 * Publish message to MQTT broker
 * @param {Object} payload - Message payload to publish
 * @return {Promise} - Resolves when message is published
 */
async function publishToMQTT(payload) {
  return new Promise((resolve, reject) => {
    // Get MQTT configuration from environment variables
    const brokerUrl = process.env.MQTT_BROKER_URL;
    const username = process.env.MQTT_USERNAME;
    const password = process.env.MQTT_PASSWORD;
    const topic = process.env.MQTT_DEVICE_TOPIC || "home/audio/device1";

    if (!brokerUrl || !username || !password) {
      const error = new Error("MQTT configuration missing");
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
    });

    client.on("connect", () => {
      logger.info("Connected to MQTT broker");

      // Publish message
      client.publish(topic, JSON.stringify(payload), {qos: 1}, (error) => {
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
      logger.error("MQTT connection error", {error});
      client.end();
      reject(error);
    });

    // Timeout after 10 seconds
    setTimeout(() => {
      if (client.connected) {
        client.end();
      }
      reject(new Error("MQTT publish timeout"));
    }, 10000);
  });
}