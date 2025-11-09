# ESP32 WiFi Balancing Robot

A self-balancing robot based on ESP32 that can be controlled via Android over WiFi.

This self-balancing robot is based on [B-Robot](https://www.jjrobots.com/much-more-than-a-self-balancing-robot/), which is open sourced on jjrobots. Thanks to the article by [ghmartin77](https://www.jjrobots.com/community/migrated-forums-4-jjrobots-b-robot/esp32-port-of-b-robot_evo2-code-3/) posted on the B-Robot forum.

## Hardware Requirements

- ESP32 Development Board
- MPU6050 Gyroscope/Accelerometer
- 2x DC Motors with encoders
- Motor Driver (L298N or similar)
- Battery Pack
- Chassis and wheels
- OLED Display (SSD1306, optional)

## Software Requirements

- [PlatformIO IDE](https://platformio.org/) (VS Code extension recommended)
- USB drivers for ESP32

## Setup Instructions

### 1. Install PlatformIO

**Option A: VS Code Extension (Recommended)**
1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Open VS Code
3. Go to Extensions (Ctrl+Shift+X)
4. Search for "PlatformIO IDE"
5. Click Install

**Option B: PlatformIO Core CLI**
```bash
pip install platformio
```

### 2. Clone the Repository

```bash
git clone https://github.com/Melroy-Sahyadri-ECE/esp32-wifi-balancing-robot.git
cd esp32-wifi-balancing-robot
```

### 3. Open Project in PlatformIO

**Using VS Code:**
1. Open VS Code
2. Click on PlatformIO icon in the sidebar
3. Click "Open Project"
4. Select the cloned repository folder

**Using CLI:**
```bash
cd esp32-wifi-balancing-robot
pio project init --ide vscode
```

### 4. Configure Upload Port

Edit `platformio.ini` and change the COM port to match your ESP32:

```ini
upload_port = COM3    ; Change to your port (COM3, COM4, etc. on Windows)
monitor_port = COM3   ; Or /dev/ttyUSB0, /dev/ttyACM0 on Linux/Mac
```

To find your port:
- **Windows:** Check Device Manager > Ports (COM & LPT)
- **Linux/Mac:** Run `ls /dev/tty*` in terminal

### 5. Upload Code to ESP32

**Using VS Code:**
1. Connect ESP32 to your computer via USB
2. Click the PlatformIO icon in the sidebar
3. Under "PROJECT TASKS", expand your environment (esp32dev)
4. Click "Upload"

**Using CLI:**
```bash
pio run --target upload
```

### 6. Monitor Serial Output

**Using VS Code:**
- Click "Monitor" under PROJECT TASKS

**Using CLI:**
```bash
pio device monitor
```

## WiFi Configuration

After uploading, the ESP32 will create a WiFi access point:
- **SSID:** ESP32_Robot (or as configured in code)
- **Password:** Check the code for default password
- Connect your Android device to this network
- Access the web interface at: `http://192.168.4.1`

## Troubleshooting

### Upload Failed
- Check if the correct COM port is selected
- Press and hold the BOOT button on ESP32 during upload
- Install CH340/CP2102 USB drivers if needed

### Compilation Errors
- Run `pio lib install` to ensure all dependencies are installed
- Clean the build: `pio run --target clean`

### Robot Not Balancing
- Calibrate the MPU6050 sensor
- Adjust PID parameters in the code
- Check motor connections and polarity

## Project Structure

```
esp32-wifi-balancing-robot/
├── src/                    # Main source files
│   ├── main.cpp           # Main program
│   ├── Control.cpp/h      # PID control logic
│   ├── Motors.cpp/h       # Motor control
│   ├── MPU6050.cpp/h      # Sensor interface
│   └── globals.cpp/h      # Global variables
├── platformio.ini         # PlatformIO configuration
└── README.md             # This file
```

## Credits

Based on the B-Robot project by JJRobots and ESP32 port by ghmartin77.

## License

See LICENSE file for details.
