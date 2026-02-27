# ESP32 (ESP-IDF v5.2+) Setup Guide

This guide covers the installation of the ESP-IDF development framework required for building the RemoteAlarm firmware on Windows.

## 1. Install ESP-IDF (v5.2 or later)

The RemoteAlarm firmware is optimized for ESP-IDF v5.2 features (like the new Standard I2S driver).

### 1.1 Automatic Installation (Recommended)
1. **Download**: Get the [ESP-IDF Windows Installer](https://dl.espressif.com/dl/esp-idf/) (Universal Online Installer).
2. **Run Installer**: 
   - Select **ESP-IDF v5.2.x**.
   - The installer will handle downloading Python, Git, and the cross-compilers.
   - It will suggest a directory like `C:\Espressif`.
3. **Verify**: Open the **"ESP-IDF 5.2 PowerShell"** shortcut created on your desktop. Run:
   ```powershell
   idf.py --version
   ```

### 1.2 Troubleshooting Compiler Selection (GCC/Clang)
If you encounter errors where CMake cannot find a compiler (`No CMAKE_C_COMPILER could be found`), follow these steps which resolved the environment on this machine:
1. **Framework Version**: Ensure you are using ESP-IDF **v5.2.x**.
2. **Clean Tools**: If a previous "Clang" installation is interfering, navigate to `C:\Espressif\tools\` and delete the `clang` compiler folder.
3. **Set Toolchain Variable**: Explicitly tell the system to use the GCC toolchain by running:
   ```powershell
   setx IDF_TOOLCHAIN gcc
   ```
4. **Restart**: Restart VS Code/Terminal to ensure the new environment variable is loaded.
5. **Targeting**: Initialize the target in the `firmware/` folder:
   ```powershell
   idf.py set-target esp32s3
   ```

## 2. (Optional) Install ESP-ADF

While the current firmware uses plain ESP-IDF, you can still install ESP-ADF for future expansion:

1. **Clone the Repository**:
   Open a terminal and navigate to where you want to keep the frameworks (e.g., `C:\Espressif`):
   ```powershell
   cd C:\Espressif
   git clone --recursive https://github.com/espressif/esp-adf.git
   ```
2. **Set ADF_PATH**:
   You must tell the build system where ADF is located.
   - **Temporary (Current Session)**:
     ```powershell
     $env:ADF_PATH = "C:\Espressif\esp-adf"
     ```
   - **Permanent**:
     1. Search for "Edit the system environment variables" in Windows.
     2. Click "Environment Variables".
     3. Under "User variables", click "New".
     4. Variable name: `ADF_PATH`, Variable value: `C:\Espressif\esp-adf`.

## 3. Configure the Project

Once the tools are installed, you can configure your specific credentials.

1. **Open the ESP-IDF Terminal**: Use the desktop shortcut mentioned in step 1.
2. **Navigate to the workspace**:
   ```powershell
   cd C:\Users\guedd\source\repos\RemoteAlarm\firmware
   ```
3. **Open Menuconfig**:
   ```powershell
   idf.py menuconfig
   ```
   *Note: This will open a text-based menu (TUI). It looks old, but it is the standard way to configure ESP32 projects.*

### 3.1 Navigating Menuconfig
- **Movement**: Use the **Arrow Keys** to move up and down.
- **Select/Enter**: Press **Enter** to enter a sub-menu or edit a field.
- **Go Back**: Press **Esc** twice to go back to the previous menu.
- **Search**: Press `/` to search for a specific setting.
- **Save**: Press **S**.
- **Quit**: Press **Q**.

### 3.2 Required Settings
1. Navigate to **"Remote Alarm Configuration"** (usually at the bottom of the list).
2. Enter your details for:
   - **WiFi SSID & Password**: Your home network.
   - **MQTT Broker URL**: Found in your HiveMQ Cloud console (starts with `mqtts://`).
   - **MQTT Username/Password**: The credentials you created for the device.
   - **MQTT Topic**: Set to `home/audio/device1`.
3. Press **S** to save (keep the default filename `sdkconfig`), then **Q** to exit.

## 4. Build and Flash

1. **Build**:
   ```powershell
   idf.py build
   ```
2. **Connect Device**: Plug your ESP32-S3 speaker board into your computer.
3. **Identify Port**: Check Device Manager to see which COM port it is using (e.g., `COM3`).
4. **Flash**:
   ```powershell
   idf.py -p COM3 flash monitor
   ```

## Troubleshooting

- **idf.py not found**: Make sure you are using the specific **ESP-IDF PowerShell** shortcut, or you have run the export script (`. C:\Espressif\frameworks\esp-idf-v5.x\export.ps1`).
- **Build Errors**: Ensure you have configured your WiFi and MQTT credentials in `Kconfig.projbuild` or via `idf.py menuconfig`.
- **I2S Configuration**: Pin definitions (BCLK, WS, DOUT) are located in `firmware/main/main.c`. Verify these match your physical wiring.
