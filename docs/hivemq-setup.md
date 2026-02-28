# HiveMQ Cloud MQTT Broker Setup

Guide to set up a HiveMQ Cloud MQTT broker for the RemoteAlarm project.

## Overview
HiveMQ Cloud provides a free tier MQTT broker with TLS support, perfect for IoT devices like ESP32.

## Step 1: Create HiveMQ Cloud Account

1. Go to [HiveMQ Cloud Console](https://console.hivemq.cloud/)
2. Click **Sign Up** (or **Login** if account exists)
3. Complete registration with email verification

## Step 2: Create a Cluster

1. After logging in, click **Create Cluster**
2. Select **Serverless** (Free tier)
3. Configure cluster:
   - **Name**: `remotealarm` (or preferred name)
   - **Region**: Choose closest to your location
   - **Plan**: Serverless (Free)
4. Click **Create Cluster**
5. Wait 2-3 minutes for cluster provisioning

## Step 3: Note Broker Connection Details

After cluster is created:

1. Click on your cluster name
2. Note down the **Connection Details**:
   - **Host**: e.g., `abc123def456.s2.eu.hivemq.cloud`
   - **Port (MQTTS)**: `8883` (TLS encrypted)
   - **Port (WebSockets)**: `8884` (optional)

**Example Broker URL**: `mqtts://abc123def456.s2.eu.hivemq.cloud:8883`

## Step 4: Create Device Credentials

1. In your cluster dashboard, go to **Access Management** tab
2. Click **Add Credentials**
3. Create credentials for your ESP32 device:
   - **Username**: `esp32-device1` (or `device123`)
   - **Password**: Generate a strong password (e.g., `Str0ngP@ssw0rd!2026`)
   - **Description**: "ESP32 RemoteAlarm Device"
4. Click **Add**
5. **Important**: Save these credentials securely - they are needed for:
   - Firebase Functions environment variables
   - ESP32 firmware configuration

## Step 5: Configure Topic Permissions (ACL)

1. Still in **Access Management**, find your credentials
2. Click **Edit** or **Permissions**
3. Add topic restrictions:
   - **Subscribe**: `home/audio/device1` (device can listen)
   - **Publish**:  Deny (device should only receive, not send)
4. Save permissions

**Security**: This ensures the device can only subscribe to its own topic and cannot publish or access other topics.

## Step 6: Test Connection (Optional)

Test the broker using MQTT client tools:

### Using MQTT.fx or MQTTX
1. Download [MQTTX](https://mqttx.app/) (cross-platform GUI client)
2. Create new connection:
   - Host: `<BROKER_URL>`
   - Port: `8883`
   - Protocol: `mqtts://`
   - Username: Your created username
   - Password: Your created password
   - Enable SSL/TLS
3. Connect and test publish to `home/audio/device1`

### Using mosquitto_pub (Command Line)
```bash
mosquitto_pub -h <BROKER_URL> \
  -p 8883 \
  -t "home/audio/device1" \
  -m '{"file_url":"test","timestamp":"2026-02-09T10:00:00Z"}' \
  -u "esp32-device1" \
  -P "<PASSWORD>" \
  --capath /etc/ssl/certs/
```

## Step 7: Configure Firebase Functions

Set environment variables in Firebase:

```bash
cd functions
firebase functions:config:set \
  mqtt.broker_url="mqtts://<BROKER_URL>" \
  mqtt.username="esp32-device1" \
  mqtt.password="<PASSWORD>" \
  mqtt.device_topic="home/audio/device1"
```

Verify configuration:
```bash
firebase functions:config:get
```

## Step 8: Deploy Firebase Functions

Now that MQTT broker is configured:

```bash
firebase deploy --only functions
```

This deploys the `onAudioUpload` Cloud Function which will publish to your HiveMQ broker when audio files are uploaded.

## Troubleshooting

### Connection Refused
- Verify usage of port `8883` for MQTTS
- Check username/password are correct
- Ensure SSL/TLS is enabled in your client

### Topic Permission Denied
- Verify ACL permissions in Access Management
- Check topic name matches exactly (case-sensitive)

### Function Cannot Connect
- Run `firebase functions:config:get` to verify environment variables
- Check Firebase Functions logs: `firebase functions:log`
- Ensure broker URL includes protocol: `mqtts://`

## Security Best Practices

 **Do:**
- Use unique credentials per device
- Enable TLS (port 8883) always
- Restrict topics with ACL
- Use strong passwords
- Rotate credentials periodically

 **Don't:**
- Use default/weak passwords
- Share credentials across devices
- Allow wildcard topic access
- Use unencrypted MQTT (port 1883)
- Commit credentials to git

## Next Steps

After setup:
1. Update ESP32 firmware with broker credentials (Phase 4)
2. Test end-to-end: Flutter app  Firebase  MQTT  ESP32
3. Monitor broker dashboard for connection logs

## Resources

- [HiveMQ Cloud Documentation](https://docs.hivemq.com/hivemq-cloud/)
- [MQTT Protocol Overview](https://mqtt.org/)
- [MQTTX Client](https://mqttx.app/)