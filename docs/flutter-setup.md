# Flutter Setup Guide for Windows

This guide helps you install and configure Flutter for the RemoteAlarm mobile app.

## Prerequisites

- Windows 10/11 (64-bit)
- Git for Windows
- PowerShell 5.0 or newer
- At least 1.64 GB disk space (excluding IDE/tools)

## Step 1: Install Flutter SDK

### Download Flutter

1. Download the Flutter SDK from: https://flutter.dev/docs/get-started/install/windows
2. Extract the zip file to a location (e.g., `C:\src\flutter`)
3. **Important**: Do NOT install Flutter in `C:\Program Files\` (requires elevated privileges)

### Add Flutter to PATH

1. Open **Environment Variables**:
   - Press `Win + X`  System  Advanced system settings  Environment Variables
2. Under **User variables**, find `Path` and click **Edit**
3. Add new entry: `C:\src\flutter\bin` (adjust to your installation path)
4. Click **OK** to save

### Verify Installation

Open a new PowerShell window:

```powershell
flutter --version
flutter doctor
```

## Step 2: Install Required Tools

Flutter doctor will show what's missing. Install the following:

### 1. Visual Studio Code (Recommended)

```powershell
# Download from: https://code.visualstudio.com/
```

After installation, install Flutter extensions:
- Flutter (by Dart Code)
- Dart (by Dart Code)

### 2. Android Studio (for Android development)

```powershell
# Download from: https://developer.android.com/studio
```

During installation:
- Install Android SDK
- Install Android SDK Command-line Tools
- Install Android SDK Build-Tools
- Install Android Emulator

### 3. Accept Android Licenses

```powershell
flutter doctor --android-licenses
```

Type `y` to accept all licenses.

## Step 3: Verify Setup

Run Flutter doctor again:

```powershell
flutter doctor -v
```

You should see checkmarks for:
-  Flutter (SDK)
-  Android toolchain
-  VS Code
-  Connected device (optional for now)

## Step 4: Configure Firebase for Flutter

### Install FlutterFire CLI

```powershell
dart pub global activate flutterfire_cli
```

Add Dart global packages to PATH:
- Add `%LOCALAPPDATA%\Pub\Cache\bin` to your PATH environment variable

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

## Step 5: Initialize Flutter Project

From the repository root:

```powershell
cd c:\Users\guedd\source\repos\RemoteAlarm\mobile
flutter create . --org com.remotealarm
```

This creates the Flutter project structure while preserving existing files.

## Step 6: Install Dependencies

```powershell
flutter pub get
```

## Troubleshooting

### "flutter" not recognized

- Restart PowerShell/Terminal after adding to PATH
- Verify PATH includes Flutter bin directory
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