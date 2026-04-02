# ChompSafe Food Scanner v1.0

![License](https://img.shields.io/badge/license-Proprietary-red)
![Platform](https://img.shields.io/badge/platform-Arduino%20MKR%20WiFi%201010-blue)

This repository houses the embedded software for the **ChompSafe Food Scanner**, an iGEM project device that uses optical sensing to detect food contamination. The device provides real-time analysis through a wireless web interface accessible via smartphone or computer.

**Made possible through the Crocker Fellowship at BYU.**

---

## 📋 Project Overview

The ChompSafe Food Scanner is an Arduino-based IoT device that:
- Reads analog sensor data from a light sensor (A5)
- Compares readings against a calibration threshold
- Provides visual feedback via red/green LEDs
- Hosts a web server for wireless control and monitoring
- Uses AJAX for real-time "no-refresh" scanning

---

## 🏗️ Repository Structure

```
Crocker-Fellowship-iGem/
├── src/
│   ├── main.cpp          # Main controller logic (hardware, WiFi, server)
│   └── website.h         # Frontend HTML/CSS/JS stored in Flash memory
├── include/
│   ├── header.h          # Hardware pin assignments & configuration
│   └── README            # PlatformIO include directory documentation
├── docs/
│   ├── architecture.md   # System architecture documentation
│   └── requirements.md   # Project requirements
├── .env                  # Local environment variables (NOT in git)
├── .env.example          # Template for environment setup
├── .gitignore            # Git exclusions
├── platformio.ini        # PlatformIO build configuration
├── LICENSE               # Proprietary license
└── README.md             # This file
```

---

## 🚀 Getting Started

### Prerequisites

- **PlatformIO** (recommended) or Arduino IDE
- **Arduino MKR WiFi 1010** board
- Light sensor connected to analog pin A5
- Red LED connected to A2, Green LED connected to A1
- WiFi network (2.4GHz)

### Installation

1. **Clone the repository:**
   ```bash
   git clone https://github.com/your-org/Crocker-Fellowship-iGem.git
   cd Crocker-Fellowship-iGem
   ```

2. **Configure environment variables:**
   ```bash
   cp .env.example .env
   ```
   Edit `.env` with your WiFi credentials:
   ```env
   WIFI_SSID=YourNetworkName
   WIFI_PASSWORD=YourNetworkPassword
   SENSOR_THRESHOLD=700
   ```

3. **Load environment variables (Windows PowerShell):**
   ```powershell
   Get-Content .env | ForEach-Object {
       if ($_ -match '^([^=]+)=(.*)$') {
           [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process')
       }
   }
   ```

   **Or on Linux/Mac:**
   ```bash
   export $(cat .env | xargs)
   ```

4. **Build and upload:**
   ```bash
   pio run --target upload
   ```

5. **Monitor serial output:**
   ```bash
   pio device monitor
   ```
   The device will print its IP address once connected to WiFi.

6. **Access the web interface:**
   Open a browser and navigate to the IP address shown in the serial monitor.

---

## 📦 Dependencies

All dependencies are automatically managed by PlatformIO via `platformio.ini`:

- **arduino-libraries/Arduino_MKRIoTCarrier** @ ^2.1.0
- **arduino-libraries/WiFiNINA** @ ^1.8.14

Platform: `atmelsam`  
Framework: `arduino`  
Board: `mkrwifi1010`

---

## ⚙️ Configuration

### Hardware Pins (defined in `include/header.h`)
| Component      | Pin | Description                    |
|---------------|-----|--------------------------------|
| Red LED       | A2  | "Unsafe" indicator             |
| Green LED     | A1  | "Safe" indicator               |
| Light Sensor  | A5  | Analog input for contamination detection |

### Sensor Calibration
The `SENSOR_THRESHOLD` value (default: 700) determines the cutoff between safe and unsafe readings:
- **Above threshold**: Red LED activates (unsafe)
- **Below threshold**: Green LED activates (safe)

Adjust this value in `.env` based on your specific sensor and lighting conditions.

---

## 🌐 Web Interface

The device hosts a lightweight web server on port 80. The interface includes:
- **Visual branding** (ChompSafe logo, color scheme)
- **Scan button** with holographic animation effects
- **AJAX-based scanning** (no page refresh required)
- **Real-time results** displayed instantly

Frontend code is stored in Flash memory to preserve limited SRAM on the Arduino.

---

## 🔒 Security Notes

- The `.env` file contains sensitive WiFi credentials and is **excluded from version control**
- Always use `.env.example` as a template when setting up new environments
- For production deployments, consider implementing authentication on the web interface

---

## 🤝 Contributing

### Workflow Rules

- **Do NOT commit/push directly to `main`**
- **Prefer pull requests (PRs) to `dev` branch**
- You may commit small, certain-to-work changes directly to `dev`
- All major features should go through PR review

### Branch Strategy
```
main       (production-ready code)
  ↑
dev        (integration branch)
  ↑
feature/*  (individual features)
```

---

## 📄 License

Copyright (c) 2026 Crocker Holdings. All rights reserved.

This repository and its contents are proprietary. See [LICENSE](LICENSE) for details.

---

## 🙏 Acknowledgments

This project is made possible through the **Crocker Fellowship at Brigham Young University**.

---

## 📞 Support

For questions or issues, please contact the development team or open an issue in the repository tracker.

