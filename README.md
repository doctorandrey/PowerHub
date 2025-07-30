# ESP32-C3 Power Hub

A sophisticated ESP32-C3-based power management system with a web interface for remote control of 8 channels (4 digital ON/OFF + 4 PWM channels).

## 🚀 Features

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

### **Robust Error Handling**
- **Graceful WiFi Failures**: Continues operation without network
- **Filesystem Recovery**: Emergency web interface when filesystem fails
- **LED Status Indicators**: Visual feedback for system state
- **Automatic Reconnection**: Attempts WiFi reconnection every 5 minutes

## 🛠️ Hardware Requirements

### **ESP32-C3 Development Board**
- ESP32-C3-DevKitM-1 or compatible
- Built-in LED for status indication
- WiFi connectivity

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

## 📋 Dependencies

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

## 🔧 Installation

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

## 🌐 Web Interface

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

## 🔌 API Endpoints

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

## 🔌 Serial Commands

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

## 💡 LED Status Indicators

### **Blink Patterns**
- **Error (3 blinks)**: Filesystem or critical errors
- **Warning (2 blinks)**: WiFi connection issues
- **Success (1 blink)**: Successful operations
- **Heartbeat (1 blink/5s)**: Normal operation

### **Status Meanings**
- **Filesystem Error**: 3 blinks every 30 seconds
- **WiFi Error**: 2 blinks every 30 seconds
- **Normal Operation**: 1 blink every 5 seconds

## 🛡️ Error Handling

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
- Proper WebSocket client cleanup
- State persistence across reboots

## ⚙️ Configuration

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

## 📁 Project Structure

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

## 🚀 Usage Examples

### **IoT Projects**
- Automated control systems - Power Hub for astrophoto setup
- PWM controller for Dew Heater of Flat Box

## 🔮 Future Enhancements

- [ ] OTA (Over-The-Air) updates
- [ ] Multiple WiFi network support
- [ ] Configuration web interface

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- ESP32 Arduino Core
- ESPAsyncWebServer library
- ArduinoJson library
- PlatformIO development environment

---

**Built with ❤️ for the ESP32 community**
