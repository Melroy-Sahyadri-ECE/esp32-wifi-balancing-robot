# ESP32 WiFi Balancing Robot & RC Car Controller

This repository contains the firmware for an ESP32-based RC car / balancing robot. The project leverages both cores of the ESP32 to provide an ultra-responsive web-based joystick control.

## Features
- **Dual-Core Architecture**:
  - **Core 0**: Dedicated Motor Control loop running at 100Hz with built-in safety timeout (shuts down motors on connection loss).
  - **Core 1**: Dedicated WiFi stack and Web Server hosting the graphical remote control interface.
- **Web UI**: Modern graphical interface with a virtual thumbstick, accessible via a web browser when connected to the ESP32.
- **Motor Control**: Controls DC motors via PWM with logic compatible with an IBT-2 or standard H-Bridge motor driver.

---

## 🚀 Getting Started: From Cloning to Uploading

Follow these steps to deploy this codebase to your own ESP32 using VS Code and PlatformIO.

### Prerequisites
- **Visual Studio Code (VS Code)** installed.
- **PlatformIO IDE** extension installed in VS Code.
- An **ESP32 Development Board**.
- A USB data cable.

### 1. Clone the Repository
Open a terminal (or Command Prompt / PowerShell) and clone this repository to your machine:
```bash
git clone https://github.com/Melroy-Sahyadri-ECE/esp32-wifi-balancing-robot.git
```
Then, navigate into the downloaded folder:
```bash
cd esp32-wifi-balancing-robot
```

### 2. Open in VS Code
Open the project directory in VS Code:
```bash
code .
```
*(Alternatively, open VS Code, go to **File -> Open Folder**, and select the `esp32-wifi-balancing-robot` folder).*

### 3. Connect the ESP32
Connect your ESP32 board to your computer via your USB cable.

### 4. Build and Upload using PlatformIO

**Option A: Using VS Code UI**
1. Once the project opens, PlatformIO will automatically initialize and read the `platformio.ini` file.
2. Allow a few seconds for PlatformIO to download all the necessary toolchains for the ESP32 in the background.
3. Look for the **blue status bar** at the bottom of your VS Code window.
4. Click the **Upload** button (the small **Right Arrow** icon `→` on the bottom bar).
5. PlatformIO will now automatically compile the code and flash it to your ESP32.

**Option B: Using the Command Line**
If you prefer running commands, you can use the PlatformIO CLI from your terminal inside the project folder:
- **Build the project:**
  ```bash
  pio run
  ```
- **Upload to the ESP32:**
  ```bash
  pio run -t upload
  ```
- **Open Serial Monitor:**
  ```bash
  pio device monitor --baud 115200
  ```

### 5. Access the Web Controller
1. Open the **Serial Monitor** in PlatformIO (the **Plug** icon on the bottom blue bar).
2. Note the IP address printed on the console (e.g., `192.168.4.1`).
3. Connect your phone or laptop to the WiFi network broadcasted by the ESP32 (SSID: `IIT_CAR`, Password: `12345678`).
4. Type the IP address into your web browser to start controlling the robot!
