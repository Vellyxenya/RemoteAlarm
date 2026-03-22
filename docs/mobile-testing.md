# Mobile App Testing Guide

This guide explains how to run and test the RemoteAlarm mobile app on emulators and physical devices.

## 1. Android

### Running on an Emulator
1.  **Open Android Studio**.
2.  Click on **More Actions** > **Virtual Device Manager**.
3.  Click **Create device**.
4.  Select a hardware profile (e.g., Pixel 6) and click **Next**.
5.  Select a system image (recommended: latest stable release) and click **Next**.
6.  Click **Finish**.
7.  Click the **Play** button in the device manager to start the emulator.
8.  In VS Code, check the bottom right corner to ensure the emulator is selected (or click to select it).
9.  Run the app:
    ```bash
    cd mobile
    flutter run
    ```

### Running on a Physical Device
1.  **Enable Developer Options**:
    -   Go to **Settings** > **About phone**.
    -   Tap **Build number** 7 times until you see a message "You are now a developer!".
2.  **Enable USB Debugging**:
    -   Go to **Settings** > **System** > **Developer options**.
    -   Toggle **USB debugging** to ON.
3.  **Connect via USB**:
    -   Connect your phone to your computer.
    -   On your phone, a prompt "Allow USB debugging?" will appear. Check "Always allow..." and tap **Allow**.
4.  **Verify Connection**:
    -   Run `flutter devices` in your terminal. You should see your device listed.
5.  **Run the App**:
    ```bash
    cd mobile
    flutter run
    ```

## 2. iOS (macOS only)

### Running on IOS Simulator
1.  Open **Simulator** app (via Spotlight search).
2.  In VS Code, select the simulator from the device selector.
3.  Run:
    ```bash
    cd mobile
    flutter run
    ```

### Running on Physical iPhone
1.  Open **Xcode**.
2.  Sign in with your Apple ID in **Settings > Accounts**.
3.  Connect your iPhone via USB.
4.  Trust the computer on your iPhone.
5.  In Xcode, open `ios/Runner.xcworkspace`.
6.  Select your team in the **Signing & Capabilities** tab.
7.  Run the app from VS Code or Xcode.

## 3. Troubleshooting

-   **Device not found**: Try unplugging and replugging the USB cable. Ensure USB debugging is on.
-   **Permission denied**: Ensure you accepted the prompt on your phone.
-   **Build errors**: Run `flutter clean` and `flutter pub get` to reset dependencies.
