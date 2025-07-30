# ESP32-C3 Power Hub

A sophisticated ESP32-C3-based power management system with a modern web interface for remote control of 8 channels (4 digital ON/OFF + 4 PWM channels).

## 🚀 Features
=======
A sophisticated ESP32-C3-based power management system with a web interface for remote control of 8 channels (4 digital ON/OFF + 4 PWM channels).

## Features
### **Hardware Control**
- **4 Digital Channels**: ON/OFF control for relays or digital outputs
- **4 PWM Channels**: 0-255 dimming control for LED strips, motors, or other PWM devices
- **Serial Interface**: External command control via UART
- **State Persistence**: Remembers settings across reboots using ESP32 Preferences

### **Web Interface**
- **Responsive Design**: Works on desktop and mobile devices
- **Real-time Control**: WebSocket communication for instant feedback
- **Dark/Light Theme**: User preference with persistence
- **Emergency Mode**: Fallback interface when filesystem is unavailable
- **Dynamic UI**: Automatically generates controls based on channel configuration

### **Error Handling**
- **Graceful WiFi Failures**: Continues operation without network
- **Filesystem Recovery**: Emergency web interface when filesystem fails
- **LED Status Indicators**: Visual feedback for system state
- **Automatic Reconnection**: Attempts WiFi reconnection every 5 minutes

## Hardware Requirements
=======
## Hardware Requirements

### **ESP32-C3 Development Board**
- ESP32-C3-DevKitM-1 or compatible
- Built-in LED for status indication
- WiFi connectivity

### **ESP32-C3 Development Board**
- 8-channel MOSFET control board
https://aliexpress.ru/item/1005008202397815.html

>>>>>>> 264600953066aa8cba19736c7a74e368a5f4b35d
### **Output Channels**
```
Digital Channels (ON/OFF):
- CH1: Pin 2
- CH2: Pin 3  
- CH3: Pin 4
- CH4: Pin 5

PWM Channels (0-255):
- CH5: Pin 10
- CH6: Pin 12
- CH7: Pin 13
- CH8: Pin 14

Serial Interface:
- RX: Pin 20
- TX: Pin 21
```

<<<<<<< HEAD
## 📋 Dependencies
=======
## Dependencies
>>>>>>> 264600953066aa8cba19736c7a74e368a5f4b35d

### **PlatformIO Libraries**
```ini
lib_deps = 
    ayushsharma82/ElegantOTA@^3.1.2
    bblanchon/ArduinoJson@^7.4.2
    ESP Async WebServer=https://github.com/ESP32Async/ESPAsyncWebServer.git
    AsyncTCP=https://github.com/ESP32Async/AsyncTCP.git
```

### **Core Libraries**
- `WiFi.h` - Network connectivity
- `AsyncTCP.h` - Asynchronous TCP for WebSocket
- `ESPAsyncWebServer.h` - Async web server
- `LittleFS.h` - File system for web files
- `Preferences.h` - Persistent storage
- `ArduinoJson.h` - JSON parsing
- `SerialCommand.h` - Serial command parsing

<<<<<<< HEAD
## 🔧 Installation
=======
## Installation
>>>>>>> 264600953066aa8cba19736c7a74e368a5f4b35d

### **1. Clone the Repository**
```bash
git clone <repository-url>
cd Power-Hub
```

### **2. Configure WiFi**
Edit `src/main.cpp` and update WiFi credentials:
```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
```

### **3. Upload Filesystem**
```bash
pio run -t uploadfs
```

### **4. Build and Upload**
```bash
pio run -t upload
```

<<<<<<< HEAD
## 🌐 Web Interface
=======
## Web Interface
>>>>>>> 264600953066aa8cba19736c7a74e368a5f4b35d

### **Access**
Once connected to WiFi, access the web interface at:
```
http://[ESP32_IP_ADDRESS]
```

### **Features**
- **Channel Control**: Individual ON/OFF and PWM controls
- **Real-time Updates**: Instant feedback via WebSocket
- **Theme Toggle**: Dark/light mode with persistence
- **Status Display**: Current channel states
- **Emergency Mode**: Basic interface when filesystem unavailable

<<<<<<< HEAD
## 🔌 API Endpoints
=======
## API Endpoints
>>>>>>> 264600953066aa8cba19736c7a74e368a5f4b35d

### **GET /api**
Returns JSON configuration of all channels:
```json
{
  "commands": [
    {
      "command": "CH1",
      "description": "Turn channel 1 ON or OFF",
      "type": "digital",
      "value": "OFF"
    },
    {
      "command": "CH5",
      "description": "Set PWM value (0-255) for channel 5",
      "type": "pwm",
      "value": 128
    }
  ]
}
```

### **WebSocket /ws**
- **Incoming**: `{"command": "CH1=ON"}`
- **Outgoing**: Acknowledgments and status updates

<<<<<<< HEAD
## 🔌 Serial Commands
=======
## Serial Commands
>>>>>>> 264600953066aa8cba19736c7a74e368a5f4b35d

### **Digital Channels**
```
CH1=ON    # Turn channel 1 ON
CH1=OFF   # Turn channel 1 OFF
CH2=ON    # Turn channel 2 ON
CH2=OFF   # Turn channel 2 OFF
# ... similar for CH3, CH4
```

### **PWM Channels**
```
CH5=128   # Set channel 5 to PWM value 128 (0-255)
CH6=255   # Set channel 6 to PWM value 255
CH7=0     # Set channel 7 to PWM value 0
CH8=64    # Set channel 8 to PWM value 64
```

### **Status Commands**
```
STATUS    # Get status of all channels
RESET     # Reset all channels to default state
```

<<<<<<< HEAD
## 💡 LED Status Indicators
=======
## LED Status Indicators
>>>>>>> 264600953066aa8cba19736c7a74e368a5f4b35d

### **Blink Patterns**
- **Error (3 blinks)**: Filesystem or critical errors
- **Warning (2 blinks)**: WiFi connection issues
- **Success (1 blink)**: Successful operations
- **Heartbeat (1 blink/5s)**: Normal operation

### **Status Meanings**
- **Filesystem Error**: 3 blinks every 30 seconds
- **WiFi Error**: 2 blinks every 30 seconds
- **Normal Operation**: 1 blink every 5 seconds

<<<<<<< HEAD
## 🛡️ Error Handling
=======
## Error Handling
>>>>>>> 264600953066aa8cba19736c7a74e368a5f4b35d

### **WiFi Failures**
- Continues operation without network
- Attempts reconnection every 5 minutes
- Serial commands remain functional
- LED indicates WiFi status

### **Filesystem Failures**
- Emergency web interface provided
- Basic channel control still available
- LED indicates filesystem status
- Serial interface unaffected

### **Memory Management**
<<<<<<< HEAD
- JSON document reservation to prevent fragmentation
- Proper WebSocket client cleanup
- State persistence across reboots

## ⚙️ Configuration
=======
- Proper WebSocket client cleanup
- State persistence across reboots

## Configuration
>>>>>>> 264600953066aa8cba19736c7a74e368a5f4b35d

### **Pin Assignments**
```cpp
#define CH1_PIN 2
#define CH2_PIN 3
#define CH3_PIN 4
#define CH4_PIN 5
#define CH5_PIN 10
#define CH6_PIN 12
#define CH7_PIN 13
#define CH8_PIN 14
#define RX_PIN 20
#define TX_PIN 21
```

### **PWM Configuration**
```cpp
ledcSetup(0, 5000, 8);  // Channel 0, 5kHz, 8-bit resolution
ledcSetup(1, 5000, 8);  // Channel 1, 5kHz, 8-bit resolution
// ... for all 4 PWM channels
```

<<<<<<< HEAD
## 📁 Project Structure
=======
## Project Structure
>>>>>>> 264600953066aa8cba19736c7a74e368a5f4b35d

```
Power Hub/
├── src/
│   └── main.cpp              # Main ESP32 code
├── data/
│   ├── index.html            # Web interface
│   ├── style.css             # Styling
│   └── script.js             # JavaScript logic
├── platformio.ini            # PlatformIO configuration
└── README.md                 # This file
```

<<<<<<< HEAD
## 🚀 Usage Examples

### **Home Automation**
- Control LED strips with PWM dimming
- Manage relay switches for lights/appliances
- Create automated lighting scenes

### **Industrial Control**
- Motor speed control via PWM
- Relay control for equipment
- Status monitoring and remote control

### **IoT Projects**
- Smart home integration
- Remote monitoring systems
- Automated control systems

## 🔮 Future Enhancements

- [ ] OTA (Over-The-Air) updates
- [ ] MQTT integration
- [ ] Multiple WiFi network support
- [ ] Configuration web interface
- [ ] Advanced scheduling features
- [ ] Security authentication
- [ ] HTTPS support
- [ ] Mobile app companion

## 🤝 Contributing
=======
## Usage Examples

### **IoT Projects**
- Automated control systems - Power Hub for astrophoto setup
- PWM controller for Dew Heater of Flat Box

## Future Enhancements

- [ ] OTA (Over-The-Air) updates
- [ ] Multiple WiFi network support
- [ ] Configuration web interface

## Contributing
>>>>>>> 264600953066aa8cba19736c7a74e368a5f4b35d

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

<<<<<<< HEAD
## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments
=======
## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments
>>>>>>> 264600953066aa8cba19736c7a74e368a5f4b35d

- ESP32 Arduino Core
- ESPAsyncWebServer library
- ArduinoJson library
- PlatformIO development environment

---

<<<<<<< HEAD
**Built with ❤️ for the ESP32 community** 
=======
**Built with ❤️ for the ESP32 community**
>>>>>>> 264600953066aa8cba19736c7a74e368a5f4b35d
