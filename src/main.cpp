#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <SerialCommand.h>

#define RX_PIN 20
#define TX_PIN 21
#define CH1_PIN 1
#define CH2_PIN 2
#define CH3_PIN 3
#define CH4_PIN 4
#define CH5_PIN 5
#define CH6_PIN 6
#define CH7_PIN 7
#define CH8_PIN 8
#define CH_ON LOW
#define CH_OFF HIGH

SerialCommand sCmd;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient* lastClient = nullptr;
Preferences prefs;

struct OnboardLed {
  uint8_t pin;
  bool on = false;
  unsigned long lastBlinkTime = 0;
  int blinkCount = 0;
  int blinkInterval = 0;
  bool blinking = false;

  void begin(uint8_t p) {
    pin = p;
    pinMode(pin, OUTPUT);
  }

  void update() {
    digitalWrite(pin, on ? HIGH : LOW);
  }
  
  void set(bool state) {
    on = state;
    update();
  }
  
  void blink(int times, int interval) {
    blinkCount = times * 2; // *2 because we need on/off for each blink
    blinkInterval = interval;
    blinking = true;
    lastBlinkTime = millis();
    on = true;
    update();
  }
  
  void updateBlink() {
    if (!blinking) return;
    
    if (millis() - lastBlinkTime >= blinkInterval) {
      lastBlinkTime = millis();
      on = !on;
      update();
      blinkCount--;
      
      if (blinkCount <= 0) {
        blinking = false;
        on = false;
        update();
      }
    }
  }
  
  // Convenience methods for common patterns
  void errorBlink() {
    blink(3, 500); // 3 blinks with 500ms interval
  }
  
  void warningBlink() {
    blink(2, 1000); // 2 blinks with 1 second interval
  }
  
  void successBlink() {
    blink(1, 200); // 1 quick blink
  }
  
  void heartbeat() {
    blink(1, 5000); // 1 blink every 5 seconds
  }
};

OnboardLed onboard_led;

bool wifi_connected = false;
bool filesystem_available = false;
bool server_initialized = false;

const char* WIFI_SSID = "your_ssid";
const char* WIFI_PASS = "wlan_pass";

int pwmValues[4] = {0};

void sendAck(const String& msg) {
  if (lastClient && lastClient->status() == WS_CONNECTED) {
    JsonDocument doc;
    doc["ack"] = msg;
    String json;
    serializeJson(doc, json);
    lastClient->text(json);
  }
}

void handlePwm(int channel, const char* label) {
  char* arg = sCmd.next();
  if (arg) {
    int val = atoi(arg);
    if (val >= 0 && val <= 255) {
      pwmValues[channel] = val;
      ledcWrite(channel, val);
      prefs.putUChar(label, val);
      Serial1.printf("%s=%d OK\r\n", label, val);
      sendAck(String(label) + "=" + val);
      return;
    }
  }
  Serial1.printf("%s=%s FAIL\r\n", label, arg ? arg : "NULL");
  sendAck(String(label) + "=FAIL");
}

void handleOnOff(int pin, const char* label) {
  char* arg = sCmd.next();
  if (arg && (strcmp(arg, "ON") == 0 || strcmp(arg, "OFF") == 0)) {
    digitalWrite(pin, strcmp(arg, "ON") == 0 ? CH_ON : CH_OFF);
    prefs.putBool(label, strcmp(arg, "ON") == 0);
    Serial1.printf("%s=%s OK\r\n", label, arg);
    sendAck(String(label) + "=" + arg);
  } else {
    Serial1.printf("%s=%s FAIL\r\n", label, arg ? arg : "NULL");
    sendAck(String(label) + "=FAIL");
  }
}

void restoreState() {    
  digitalWrite(CH1_PIN, prefs.getBool("CH1", false) ? CH_ON : CH_OFF);
  digitalWrite(CH2_PIN, prefs.getBool("CH2", false) ? CH_ON : CH_OFF);
  digitalWrite(CH3_PIN, prefs.getBool("CH3", false) ? CH_ON : CH_OFF);
  digitalWrite(CH4_PIN, prefs.getBool("CH4", false) ? CH_ON : CH_OFF);
  pwmValues[0] = prefs.getUChar("CH5", 0);
  pwmValues[1] = prefs.getUChar("CH6", 0);
  pwmValues[2] = prefs.getUChar("CH7", 0);
  pwmValues[3] = prefs.getUChar("CH8", 0);
  ledcWrite(0, pwmValues[0]);
  ledcWrite(1, pwmValues[1]);
  ledcWrite(2, pwmValues[2]);
  ledcWrite(3, pwmValues[3]);
}

void cmdStatus() {
  JsonDocument doc;
  doc["CH1"] = digitalRead(CH1_PIN) == CH_ON ? "ON" : "OFF";
  doc["CH2"] = digitalRead(CH2_PIN) == CH_ON ? "ON" : "OFF";
  doc["CH3"] = digitalRead(CH3_PIN) == CH_ON ? "ON" : "OFF";
  doc["CH4"] = digitalRead(CH4_PIN) == CH_ON ? "ON" : "OFF";
  doc["CH5"] = pwmValues[0];
  doc["CH6"] = pwmValues[1];
  doc["CH7"] = pwmValues[2];
  doc["CH8"] = pwmValues[3];

  String out;
  serializeJson(doc, out);
  Serial1.println(out);

  if (lastClient && lastClient->status() == WS_CONNECTED) {
    lastClient->text(out);
  }
}



void cmdError() {
  Serial1.print("ERR\r\n");
  sendAck("ERR");
}

void cmdCh1() { handleOnOff(CH1_PIN, "CH1"); }
void cmdCh2() { handleOnOff(CH2_PIN, "CH2"); }
void cmdCh3() { handleOnOff(CH3_PIN, "CH3"); }
void cmdCh4() { handleOnOff(CH4_PIN, "CH4"); }
void cmdCh5() { handlePwm(0, "CH5"); }
void cmdCh6() { handlePwm(1, "CH6"); }
void cmdCh7() { handlePwm(2, "CH7"); }
void cmdCh8() { handlePwm(3, "CH8"); }


// ----------------------------------------------------------------------------
// HTTP Server initialization
// ----------------------------------------------------------------------------


void onRootRequest(AsyncWebServerRequest *request) {
  request->send(LittleFS, "/index.html", "text/html");
}

void onPageNotFound(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                  "] HTTP GET request of " + request->url());
  request->send(404, "text/plain", "Not found");
}


// ----------------------------------------------------------------------------
// WebSocket initialization
// ----------------------------------------------------------------------------


void notifyClients(String data) {
    ws.textAll(data);
}

void handleWebSocketMessage(String payload) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.printf("JSON Error: %s\n", error.c_str());
    return;
  }

  if (doc.containsKey("command")) {
    String cmd = doc["command"].as<String>();
    if (cmd == "RESET") {
      prefs.clear();
      restoreState();
      sendAck("RESET");
      return;
    }
    if (cmd == "STATUS") {
      cmdStatus();
      return;
    }
    int chIndex = -1;
    if (cmd.startsWith("CH") && cmd.indexOf('=') > 2) {
      int eq = cmd.indexOf('=');
      String ch = cmd.substring(2, eq);
      String val = cmd.substring(eq + 1);

      int chNum = ch.toInt();
      if (chNum >= 1 && chNum <= 4) {
        int pin = CH1_PIN + (chNum - 1);
        if (val == "ON" || val == "OFF") {
          digitalWrite(pin, val == "ON" ? CH_ON : CH_OFF);
          String key = "CH" + String(chNum);
          prefs.putBool(key.c_str(), val == "ON");
          sendAck(cmd);
        }
      } else if (chNum >= 5 && chNum <= 8) {
        int index = chNum - 5;
        int valNum = val.toInt();
        if (valNum >= 0 && valNum <= 255) {
          pwmValues[index] = valNum;
          ledcWrite(index, valNum);
          String key = "CH" + String(chNum);
          prefs.putUChar(key.c_str(), valNum);
          sendAck(cmd);
        }
      }
    }
  }
}

void onEvent(AsyncWebSocket       *server,
             AsyncWebSocketClient *client,
             AwsEventType          type,
             void                 *arg,
             uint8_t              *data,
             size_t                len) {
    switch (type) {
        case WS_EVT_CONNECT:
            lastClient = client;
            Serial.printf("WebSocket client #%u connected from %s\r\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            if (lastClient == client) {
                lastClient = nullptr;
            }
            Serial.printf("WebSocket client #%u disconnected\r\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage((char*)data);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            Serial.println("Websocket Error event");
            break;
    }
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    Serial.print(".");
    delay(500);
    attempts++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi connection failed - continuing without network");
    wifi_connected = false;
    // Blink LED to indicate WiFi error
    onboard_led.warningBlink();
  } else {
    Serial.printf("Connected to %s\n", WiFi.localIP().toString().c_str());
    wifi_connected = true;
    onboard_led.successBlink();
  }
}



void initLittleFS(){
  int retries = 0;
  while(!LittleFS.begin() && retries < 3){
    Serial.printf("LittleFS mount failed, retry %d/3\n", retries + 1);
    delay(1000);
    retries++;
  }
  if(retries >= 3) {
    Serial.println("LittleFS mount failed permanently - continuing without file system");
    filesystem_available = false;
    // Blink LED to indicate filesystem error
    onboard_led.errorBlink();
  } else {
    Serial.println("LittleFS mounted successfully");
    filesystem_available = true;
  }
}

void initWebSocket(){
  if (!wifi_connected) {
    Serial.println("WebSocket disabled - WiFi not connected");
    return;
  }
  
  if (server_initialized) {
    Serial.println("WebSocket already initialized");
    return;
  }
  
  server.addHandler(&ws);
  ws.onEvent(onEvent);
  Serial.println("WebSocket initialized successfully");
}

void initWebServer(){
  if (!wifi_connected) {
    // No web server when WiFi is not available
    Serial.println("Web server disabled - WiFi not connected");
    return;
  }
  
  if (server_initialized) {
    Serial.println("Web server already initialized");
    return;
  }
  
  if (filesystem_available) {
    server.on("/", HTTP_GET, onRootRequest);
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    server.onNotFound(onPageNotFound);
  } else {
    // Fallback when filesystem is not available
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      String html = "<!DOCTYPE html><html><head>";
      html += "<meta charset=\"UTF-8\">";
      html += "<title>Power Hub - Emergency Mode</title>";
      html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
      html += "<style>";
      html += "body { font-family: Arial, sans-serif; background: #1e1e1e; color: #f0f0f0; padding: 20px; }";
      html += ".error { background: #ff4444; color: white; padding: 15px; border-radius: 5px; margin: 20px 0; }";
      html += ".warning { background: #ffaa00; color: black; padding: 15px; border-radius: 5px; margin: 20px 0; }";
      html += "button { padding: 10px 20px; margin: 5px; border: none; border-radius: 5px; cursor: pointer; }";
      html += ".on { background: #4caf50; color: white; }";
      html += ".off { background: #f44336; color: white; }";
      html += "input[type=range] { width: 100%; margin: 10px 0; }";
      html += "</style></head><body>";
      html += "<h1>Power Hub - Emergency Mode</h1>";
      html += "<div class=\"error\">";
      html += "<strong>⚠️ Filesystem Error:</strong> Web interface files could not be loaded. ";
      html += "This is a fallback interface for basic control.";
      html += "</div>";
      html += "<div class=\"warning\">";
      html += "<strong>Note:</strong> Some features may be limited. Check serial output for details.";
      html += "</div>";
      html += "<h2>Channel Control</h2>";
      html += "<div id=\"channels\">";
      html += "<div><h3>Digital Channels</h3>";
      html += "<button class=\"on\" onclick=\"sendCommand('CH1=ON')\">CH1 ON</button>";
      html += "<button class=\"off\" onclick=\"sendCommand('CH1=OFF')\">CH1 OFF</button><br>";
      html += "<button class=\"on\" onclick=\"sendCommand('CH2=ON')\">CH2 ON</button>";
      html += "<button class=\"off\" onclick=\"sendCommand('CH2=OFF')\">CH2 OFF</button><br>";
      html += "<button class=\"on\" onclick=\"sendCommand('CH3=ON')\">CH3 ON</button>";
      html += "<button class=\"off\" onclick=\"sendCommand('CH3=OFF')\">CH3 OFF</button><br>";
      html += "<button class=\"on\" onclick=\"sendCommand('CH4=ON')\">CH4 ON</button>";
      html += "<button class=\"off\" onclick=\"sendCommand('CH4=OFF')\">CH4 OFF</button>";
      html += "</div>";
      html += "<div><h3>PWM Channels</h3>";
      html += "<label>CH5: <input type=\"range\" min=\"0\" max=\"255\" value=\"128\" onchange=\"sendCommand('CH5=' + this.value)\"></label><br>";
      html += "<label>CH6: <input type=\"range\" min=\"0\" max=\"255\" value=\"128\" onchange=\"sendCommand('CH6=' + this.value)\"></label><br>";
      html += "<label>CH7: <input type=\"range\" min=\"0\" max=\"255\" value=\"128\" onchange=\"sendCommand('CH7=' + this.value)\"></label><br>";
      html += "<label>CH8: <input type=\"range\" min=\"0\" max=\"255\" value=\"128\" onchange=\"sendCommand('CH8=' + this.value)\"></label>";
      html += "</div></div>";
      html += "<script>";
      html += "let ws;";
      html += "function sendCommand(cmd) {";
      html += "  if (ws && ws.readyState === WebSocket.OPEN) {";
      html += "    ws.send(JSON.stringify({command: cmd}));";
      html += "  }";
      html += "}";
      html += "window.onload = function() {";
      html += "  ws = new WebSocket('ws://' + location.hostname + '/ws');";
      html += "  ws.onopen = function() {";
      html += "    console.log('WebSocket connected');";
      html += "  };";
      html += "  ws.onmessage = function(msg) {";
      html += "    console.log('Response:', msg.data);";
      html += "  };";
      html += "};";
      html += "</script></body></html>";
      request->send(200, "text/html", html);
    });
    
    server.onNotFound([](AsyncWebServerRequest *request){
      request->send(404, "text/plain", "Not found - Emergency mode active");
    });
  }

  server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("[HTTP] /api requested");

    JsonDocument doc;
    JsonArray cmds = doc.createNestedArray("commands");

    for (int i = 1; i <= 4; i++) {
      JsonObject r = cmds.createNestedObject();
      r["command"] = "CH" + String(i);
      r["description"] = "Turn channel " + String(i) + " ON or OFF";
      r["type"] = "digital";
      r["value"] = digitalRead(CH1_PIN + (i - 1)) == CH_ON ? "ON" : "OFF";
    }

    for (int i = 5; i <= 8; i++) {
      JsonObject p = cmds.createNestedObject();
      p["command"] = "CH" + String(i);
      p["description"] = "Set PWM value (0-255) for channel " + String(i);
      p["type"] = "pwm";
      p["value"] = pwmValues[i - 5];
    }

    // JsonObject s = cmds.createNestedObject();
    // s["command"] = "STATUS";
    // s["description"] = "Get status of all channels";

    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

    server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){
    prefs.clear();
    restoreState();
    request->send(200, "application/json", "{\"ack\":\"RESET\"}");
  });

  server.begin();
  server_initialized = true;
  Serial.println("Web server initialized successfully");
}

// ----------------------------------------------------------------------------
// END WebSocket init
// ----------------------------------------------------------------------------


void setup() {
  onboard_led.begin(LED_BUILTIN);

  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  prefs.begin("hub", false);

  sCmd.begin(Serial1);
  pinMode(CH1_PIN, OUTPUT);
  pinMode(CH2_PIN, OUTPUT);
  pinMode(CH3_PIN, OUTPUT);
  pinMode(CH4_PIN, OUTPUT);
  digitalWrite(CH1_PIN, CH_OFF);
  digitalWrite(CH2_PIN, CH_OFF);
  digitalWrite(CH3_PIN, CH_OFF);
  digitalWrite(CH4_PIN, CH_OFF);

  ledcSetup(0, 5000, 8); ledcAttachPin(CH5_PIN, 0);
  ledcSetup(1, 5000, 8); ledcAttachPin(CH6_PIN, 1);
  ledcSetup(2, 5000, 8); ledcAttachPin(CH7_PIN, 2);
  ledcSetup(3, 5000, 8); ledcAttachPin(CH8_PIN, 3);

  restoreState();

  sCmd.addWriteCommand("CH1", cmdCh1);
  sCmd.addWriteCommand("CH2", cmdCh2);
  sCmd.addWriteCommand("CH3", cmdCh3);
  sCmd.addWriteCommand("CH4", cmdCh4);
  sCmd.addWriteCommand("CH5", cmdCh5);
  sCmd.addWriteCommand("CH6", cmdCh6);
  sCmd.addWriteCommand("CH7", cmdCh7);
  sCmd.addWriteCommand("CH8", cmdCh8);
  sCmd.addReadCommand("STATUS", cmdStatus);
  sCmd.addError(cmdError);

  initLittleFS();
  initWiFi();
  initWebSocket();
  initWebServer();

  Serial.flush();
}
  
void loop() {
  sCmd.loop();
  
  // Update LED blinking
  onboard_led.updateBlink();

  // Only cleanup WebSocket clients if WiFi is connected
  if (wifi_connected && millis() % 120000 < 50) 
  {
    ws.cleanupClients();
  }
  
  // Status indicator and WiFi reconnection attempts
  static unsigned long lastStatusCheck = 0;
  static unsigned long lastWiFiCheck = 0;
  
  if (millis() - lastStatusCheck > 30000) {
    lastStatusCheck = millis();
    
    // Report status
    if (!filesystem_available) {
      Serial.println("Status: Running in emergency mode - filesystem unavailable");
      onboard_led.errorBlink();
    }
    
    if (!wifi_connected) {
      Serial.println("Status: Running in offline mode - WiFi unavailable");
      onboard_led.warningBlink();
    }
    
    // Heartbeat when everything is working
    if (wifi_connected && filesystem_available) {
      onboard_led.heartbeat();
    }
  }
  
  // Try to reconnect WiFi every 5 minutes if not connected
  if (!wifi_connected && millis() - lastWiFiCheck > 300000) { // 5 minutes
    lastWiFiCheck = millis();
    Serial.println("Attempting WiFi reconnection...");
    
    WiFi.disconnect();
    delay(1000);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      Serial.print(".");
      delay(500);
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("\nWiFi reconnected! IP: %s\n", WiFi.localIP().toString().c_str());
      wifi_connected = true;
      // Start web server and WebSocket if they weren't started before
      if (!server_initialized) {
        initWebSocket();
        initWebServer();
      }
    } else {
      Serial.println("\nWiFi reconnection failed");
    }
  }
}
