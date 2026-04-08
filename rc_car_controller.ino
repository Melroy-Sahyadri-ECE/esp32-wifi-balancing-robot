#include <WiFi.h>
#include <WebServer.h>

// ============================================
// WiFi Configuration
// ============================================
const char* ssid = "IIT_CAR";
const char* password = "12345678";

#define AP_MODE true
const char* router_ssid = "YourWiFiName";
const char* router_password = "YourWiFiPassword";

WebServer server(80);

// ============================================
// IBT-2 (BTS7960) Motor Driver Pin Definitions
// ============================================
#define RPWM_A 27  // Right Motor Reverse
#define LPWM_A 26  // Right Motor Forward
#define RPWM_B 33  // Left Motor Reverse
#define LPWM_B 25  // Left Motor Forward

// ============================================
// PWM Configuration
// ============================================
#define PWM_FREQ 1000
#define PWM_RESOLUTION 8

#define PWM_CHANNEL_LPWM_A 0
#define PWM_CHANNEL_RPWM_A 1
#define PWM_CHANNEL_LPWM_B 2
#define PWM_CHANNEL_RPWM_B 3

// ============================================
// FreeRTOS Dual Core Configuration
// ============================================
TaskHandle_t motorTaskHandle = NULL;
SemaphoreHandle_t motorMutex;

volatile int16_t targetX = 0;
volatile int16_t targetY = 0;
volatile bool newCommand = false;

#define SAFETY_TIMEOUT 500
volatile unsigned long lastCommandTime = 0;

// Forward declarations
void handleRoot();
void handleControl();
void handleStatus();
void driveMotors(int16_t x, int16_t y);
void setMotor(int lpwmChannel, int rpwmChannel, int speed);
void stopAllMotors();
void motorControlTask(void *parameter);

// ============================================
// Motor Control Functions
// ============================================
void setMotor(int lpwmChannel, int rpwmChannel, int speed) {
  speed = constrain(speed, -255, 255);
  
  if (speed > 0) {
    ledcWrite(lpwmChannel, speed);
    ledcWrite(rpwmChannel, 0);
  } else if (speed < 0) {
    ledcWrite(lpwmChannel, 0);
    ledcWrite(rpwmChannel, abs(speed));
  } else {
    ledcWrite(lpwmChannel, 0);
    ledcWrite(rpwmChannel, 0);
  }
}

void stopAllMotors() {
  ledcWrite(PWM_CHANNEL_LPWM_A, 0);
  ledcWrite(PWM_CHANNEL_RPWM_A, 0);
  ledcWrite(PWM_CHANNEL_LPWM_B, 0);
  ledcWrite(PWM_CHANNEL_RPWM_B, 0);
}

void driveMotors(int16_t x, int16_t y) {
  const int deadzone = 100;
  
  int16_t dzX = (abs(x) < deadzone) ? 0 : x;
  int16_t dzY = (abs(y) < deadzone) ? 0 : y;

  int16_t motorA = dzY + dzX;
  int16_t motorB = dzY - dzX;

  motorA = constrain(motorA, -1000, 1000);
  motorB = constrain(motorB, -1000, 1000);

  int16_t pwmA = map(motorA, -1000, 1000, -255, 255);
  int16_t pwmB = map(motorB, -1000, 1000, -255, 255);

  if (dzX != 0 || dzY != 0) {
    Serial.printf("Joystick X:%4d Y:%4d | Right:%4d Left:%4d\n", dzX, dzY, pwmA, pwmB);
  }

  setMotor(PWM_CHANNEL_LPWM_A, PWM_CHANNEL_RPWM_A, pwmA);
  setMotor(PWM_CHANNEL_LPWM_B, PWM_CHANNEL_RPWM_B, pwmB);
}

// ============================================
// CORE 0: Motor Control Task
// ============================================
void motorControlTask(void *parameter) {
  int16_t currentX = 0, currentY = 0;
  
  Serial.println("[Core 0] Motor control task started");
  
  while (true) {
    if (xSemaphoreTake(motorMutex, portMAX_DELAY)) {
      if (newCommand) {
        currentX = targetX;
        currentY = targetY;
        newCommand = false;
      }
      
      if (millis() - lastCommandTime > SAFETY_TIMEOUT) {
        if (currentX != 0 || currentY != 0) {
          Serial.println("[Core 0] Safety timeout - stopping");
          currentX = 0;
          currentY = 0;
        }
      }
      xSemaphoreGive(motorMutex);
    }
    
    driveMotors(currentX, currentY);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// ============================================
// Setup
// ============================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n========================================");
  Serial.println("DUAL CORE RC Car Controller");
  Serial.println("Core 0: Motor Control (100Hz)");
  Serial.println("Core 1: WiFi");
  Serial.println("========================================");

  motorMutex = xSemaphoreCreateMutex();

  // Configure PWM for motors
  ledcSetup(PWM_CHANNEL_LPWM_A, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LPWM_A, PWM_CHANNEL_LPWM_A);
  ledcSetup(PWM_CHANNEL_RPWM_A, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(RPWM_A, PWM_CHANNEL_RPWM_A);
  ledcSetup(PWM_CHANNEL_LPWM_B, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LPWM_B, PWM_CHANNEL_LPWM_B);
  ledcSetup(PWM_CHANNEL_RPWM_B, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(RPWM_B, PWM_CHANNEL_RPWM_B);

  stopAllMotors();
  Serial.println("[Core 1] Motor PWM initialized");

  xTaskCreatePinnedToCore(motorControlTask, "MotorTask", 4096, NULL, 2, &motorTaskHandle, 0);
  Serial.println("[Core 0] Motor task created");

  if (AP_MODE) {
    WiFi.softAP(ssid, password);
    Serial.println("\n[Core 1] Access Point Mode");
    Serial.print("   SSID: ");
    Serial.println(ssid);
    Serial.print("   IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    WiFi.begin(router_ssid, router_password);
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 30) {
      delay(500);
      timeout++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("\n[Core 1] Connected, IP: ");
      Serial.println(WiFi.localIP());
    } else {
      WiFi.softAP(ssid, password);
      Serial.print("\n[Core 1] Fallback AP, IP: ");
      Serial.println(WiFi.softAPIP());
    }
  }

  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.on("/status", handleStatus);
  server.begin();
  
  lastCommandTime = millis();
  Serial.println("\n[Core 1] Web server started");
  Serial.println("========================================\n");
}

void loop() {
  server.handleClient();
}

// ============================================
// Web Handlers
// ============================================
void handleControl() {
  int16_t x = server.arg("x").toInt();
  int16_t y = server.arg("y").toInt();
  
  Serial.printf("[Web] Motor X:%d Y:%d\n", x, y);
  
  if (xSemaphoreTake(motorMutex, portMAX_DELAY)) {
    targetX = x;
    targetY = y;
    newCommand = true;
    lastCommandTime = millis();
    xSemaphoreGive(motorMutex);
  }
  
  server.send(200, "text/plain", "X:" + String(x) + " Y:" + String(y));
}

void handleStatus() {
  String status = "Motors X:" + String(targetX) + " Y:" + String(targetY);
  server.send(200, "text/plain", status);
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>RC Car Controller</title>
    <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; touch-action: none; -webkit-user-select: none; user-select: none; }
        body { font-family: 'Segoe UI', sans-serif; background: linear-gradient(135deg, #1a1a2e, #16213e, #0f3460); min-height: 100vh; display: flex; flex-direction: column; align-items: center; justify-content: center; padding: 15px; color: #fff; overflow-x: hidden; }
        h1 { font-size: 24px; margin-bottom: 5px; background: linear-gradient(90deg, #00d4ff, #7b2cbf); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
        .subtitle { color: #888; margin-bottom: 10px; font-size: 11px; }
        .dual-core { background: #00d4ff33; padding: 4px 12px; border-radius: 15px; font-size: 10px; color: #00d4ff; margin-bottom: 20px; }
        
        .motor-section { text-align: center; margin-top: 5px; }
        .motor-label { font-size: 11px; color: #888; margin-bottom: 8px; }
        .joystick { width: 220px; height: 220px; border: 3px solid rgba(0,212,255,0.3); border-radius: 50%; background: radial-gradient(circle, rgba(0,212,255,0.1), transparent 70%); position: relative; box-shadow: 0 0 20px rgba(0,212,255,0.2), inset 0 0 20px rgba(0,212,255,0.1); margin: 0 auto; }
        .joystick::before { content: ''; position: absolute; top: 50%; left: 0; right: 0; height: 1px; background: linear-gradient(90deg, transparent, rgba(0,212,255,0.3), transparent); }
        .joystick::after { content: ''; position: absolute; left: 50%; top: 0; bottom: 0; width: 1px; background: linear-gradient(180deg, transparent, rgba(0,212,255,0.3), transparent); }
        .knob { width: 60px; height: 60px; background: linear-gradient(145deg, #00d4ff, #0099cc); border-radius: 50%; position: absolute; top: 80px; left: 80px; cursor: grab; box-shadow: 0 4px 15px rgba(0,212,255,0.5); }
        
        .values { display: flex; gap: 20px; margin: 10px 0; justify-content: center; }
        .value-box { background: rgba(0,212,255,0.1); border: 1px solid rgba(0,212,255,0.3); border-radius: 8px; padding: 8px 15px; text-align: center; min-width: 70px; }
        .value-label { font-size: 9px; color: #888; }
        .value-num { font-size: 18px; font-weight: bold; color: #00d4ff; font-family: monospace; }
        
        .stop-btn { background: linear-gradient(145deg, #ff4757, #c92a2a); color: white; border: none; padding: 10px 30px; font-size: 14px; font-weight: bold; border-radius: 20px; cursor: pointer; margin-top: 8px; text-transform: uppercase; }
        .status { margin-top: 10px; padding: 5px 12px; background: rgba(0,0,0,0.3); border-radius: 12px; font-size: 11px; color: #888; }
    </style>
</head>
<body>
    <h1>RC Car Controller</h1>
    <p class="subtitle">ESP32 + Motors</p>
    <div class="dual-core">DUAL CORE ACTIVE</div>
    
    <div class="motor-section">
        <div class="motor-label">MOTOR CONTROL</div>
        <div class="joystick" id="joystick">
            <div class="knob" id="knob"></div>
        </div>
        
        <div class="values">
            <div class="value-box"><div class="value-label">STEER</div><div class="value-num" id="valX">0</div></div>
            <div class="value-box"><div class="value-label">THROTTLE</div><div class="value-num" id="valY">0</div></div>
        </div>
        
        <button class="stop-btn" onclick="stopMotors()">STOP</button>
    </div>
    
    <div class="status" id="status">Ready</div>

    <script>
        var joystick = document.getElementById('joystick');
        var knob = document.getElementById('knob');
        var isDragging = false, motorX = 0, motorY = 0, lastSend = 0;
        var joySize = 220, knobSize = 60, centerPos = (joySize - knobSize) / 2, maxDist = centerPos;

        function startDrag(e) { isDragging = true; e.preventDefault(); drag(e); }
        
        function drag(e) {
            if (!isDragging) return;
            var rect = joystick.getBoundingClientRect();
            var clientX = e.touches ? e.touches[0].clientX : e.clientX;
            var clientY = e.touches ? e.touches[0].clientY : e.clientY;
            var x = clientX - rect.left - joySize/2;
            var y = clientY - rect.top - joySize/2;
            var dist = Math.sqrt(x*x + y*y);
            if (dist > maxDist) { x = (x/dist)*maxDist; y = (y/dist)*maxDist; }
            knob.style.left = (centerPos + x) + 'px';
            knob.style.top = (centerPos + y) + 'px';
            motorX = Math.round((x/maxDist)*1000);
            motorY = Math.round((-y/maxDist)*1000);
            document.getElementById('valX').textContent = motorX;
            document.getElementById('valY').textContent = motorY;
            if (Date.now() - lastSend > 40) { sendControl(); lastSend = Date.now(); }
        }
        
        function stopDrag() {
            if (!isDragging) return;
            isDragging = false;
            knob.style.left = centerPos + 'px';
            knob.style.top = centerPos + 'px';
            motorX = 0; motorY = 0;
            document.getElementById('valX').textContent = '0';
            document.getElementById('valY').textContent = '0';
            sendControl();
        }
        
        function sendControl() {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/control?x=' + motorX + '&y=' + motorY, true);
            xhr.onreadystatechange = function() { if (xhr.readyState === 4 && xhr.status === 200) document.getElementById('status').textContent = xhr.responseText; };
            xhr.send();
        }
        
        function stopMotors() { isDragging = false; motorX = 0; motorY = 0; document.getElementById('valX').textContent = '0'; document.getElementById('valY').textContent = '0'; knob.style.left = centerPos + 'px'; knob.style.top = centerPos + 'px'; sendControl(); }
        
        knob.addEventListener('mousedown', startDrag);
        knob.addEventListener('touchstart', startDrag);
        joystick.addEventListener('mousedown', startDrag);
        joystick.addEventListener('touchstart', startDrag);
        document.addEventListener('mousemove', drag);
        document.addEventListener('touchmove', drag);
        document.addEventListener('mouseup', stopDrag);
        document.addEventListener('touchend', stopDrag);
        document.addEventListener('contextmenu', function(e) { e.preventDefault(); });
        setInterval(function() { if (isDragging) sendControl(); }, 150);
    </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}
