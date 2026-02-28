# Flutter Setup Guide for Windows

Guide for Flutter installation and configuration for the RemoteAlarm mobile app.

## Prerequisites

- Windows 10/11 (64-bit)
- Visual Studio Code
- Git for Windows
- PowerShell 5.0 or newer
- At least 1.64 GB disk space (excluding IDE/tools)

## Step 1: Install Flutter Extension in VS Code

1. Open Visual Studio Code
2. Go to the Extensions view (or press `Ctrl+Shift+X`)
3. Search for "Flutter"
4. Click **Install** on the Flutter extension (by Dart Code)
   - This will also install the Dart extension automatically

## Step 2: Install Flutter SDK via VS Code

1. Open the command palette in VS Code:
   - Go to **View > Command Palette** or press `Ctrl + Shift + P`

2. Type `flutter` and select **Flutter: New Project**

3. VS Code prompts to locate the Flutter SDK
   - Select **Download SDK**

4. When the **Select Folder for Flutter SDK** dialog displays, choose the install location
   - Recommended: `C:\src\flutter`
   - Click **Clone Flutter**

5. Wait for the download to complete
   - VS Code displays: "Downloading the Flutter SDK. This may take a few minutes."
   - If the download seems stuck, click **Cancel** and restart

6. Click **Add SDK to PATH** when prompted

7. When successful, the message "The Flutter SDK was added to your PATH" will appear.

8. If prompted about Google Analytics, click **OK** to agree.

9. **Important**: To ensure Flutter is available in all terminals:
   - Close and reopen all terminal windows
   - Restart VS Code

### Verify Installation

Open a new PowerShell terminal in VS Code:

```powershell
flutter --version
flutter doctor
```

## Step 3: Install Additional Tools

Flutter doctor will show what's missing. Install the following:

### Android Studio (for Android development)

1. Download from: https://developer.android.com/studio
2. During installation, make sure to install:
   - Android SDK
   - Android SDK Command-line Tools
   - Android SDK Build-Tools
   - Android Emulator

### Accept Android Licenses

```powershell
flutter doctor --android-licenses
```

Type `y` to accept all licenses.

## Step 4: Verify Setup

Run Flutter doctor again:

```powershell
flutter doctor -v
```

Checkmarks for the following should appear:
-  Flutter (SDK)
-  Android toolchain
-  VS Code
-  Connected device (optional for now)

## Step 5: Configure Firebase for Flutter

### Install FlutterFire CLI

```powershell
dart pub global activate flutterfire_cli
```

Add Dart global packages to PATH:
- Add `%LOCALAPPDATA%\Pub\Cache\bin` to your PATH environment variable
- Restart terminal after adding to PATH

### Configure Firebase Project

From the `mobile/` directory:

```powershell
cd mobile
flutterfire configure
```

This will:
1. List your Firebase projects
2. Select `remotealarm-be4d7`
3. Generate `firebase_options.dart` with your configuration
4. Update platform-specific files (Android/iOS)

## Step 6: Initialize Flutter Project

From the repository root:

```powershell
cd c:\Users\guedd\source\repos\RemoteAlarm\mobile
flutter create . --org com.remotealarm
```

This creates the Flutter project structure while preserving existing files.

## Step 7: Install Dependencies

```powershell
flutter pub get
```

## Troubleshooting

### "flutter" not recognized

- Restart PowerShell/Terminal after installation
- Restart VS Code
- Verify Flutter was added to PATH during installation
- Try: `where.exe flutter`

### Android licenses not accepted

```powershell
flutter doctor --android-licenses
```

### Flutter doctor shows issues

Run with verbose flag:
```powershell
flutter doctor -v
```

### Cannot run on device

For testing without physical device:
```powershell
# Create Android emulator
flutter emulators --create

# List available emulators
flutter emulators

# Launch emulator
flutter emulators --launch <emulator_id>
```

### Download seems stuck

- Click Cancel in VS Code
- Try again: `Ctrl+Shift+P`  `Flutter: New Project`  `Download SDK`

## Next Steps

After setup:
1. Configure Firebase (see mobile/README.md)
2. Run the app: `flutter run`
3. Start development

## Resources

- [Flutter Documentation](https://flutter.dev/docs)
- [Flutter Codelab](https://flutter.dev/docs/get-started/codelab)
- [Firebase for Flutter](https://firebase.flutter.dev/)
- [Dart Language Tour](https://dart.dev/guides/language/language-tour)