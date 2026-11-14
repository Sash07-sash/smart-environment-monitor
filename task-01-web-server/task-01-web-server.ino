#include <WiFi.h>
#include <WebServer.h>

// Wi-Fi Credentials for local 2.4 GHz network
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Hardware Pin Definition (GPIO 2 has the onboard LED on most ESP32 boards)
#define LED_PIN 2

// Instantiate WebServer on standard HTTP port 80
WebServer server(80);

// Root route handler: Generates dynamic HTML user interface with live status
void handleRoot() {
  int ledState = digitalRead(LED_PIN);
  String html = "<!DOCTYPE html><html lang='en'><head>";
  html += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32 LED Web Controller</title>";
  html += "<style>";
  html += "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: #0f172a; color: #f8fafc; text-align: center; padding: 40px 20px; }";
  html += ".card { background: #1e293b; max-width: 420px; margin: 0 auto; padding: 30px; border-radius: 16px; border: 1px solid #334155; box-shadow: 0 10px 25px rgba(0,0,0,0.5); }";
  html += "h1 { font-size: 24px; margin-bottom: 8px; color: #38bdf8; }";
  html += ".status-box { font-size: 18px; margin: 24px 0; padding: 14px; border-radius: 10px; background: #090d16; font-weight: 600; }";
  html += ".status-on { color: #10b981; } .status-off { color: #ef4444; }";
  html += ".btn-group { display: flex; gap: 14px; justify-content: center; }";
  html += ".btn { flex: 1; padding: 14px 20px; font-size: 16px; font-weight: 700; border-radius: 10px; text-decoration: none; color: #fff; transition: transform 0.1s, opacity 0.2s; }";
  html += ".btn-on { background: #10b981; } .btn-off { background: #ef4444; }";
  html += ".btn:active { transform: scale(0.96); opacity: 0.9; }";
  html += ".meta { margin-top: 24px; font-size: 13px; color: #94a3b8; }";
  html += "</style></head><body>";
  html += "<div class='card'>";
  html += "<h1>ESP32 Web Server</h1>";
  html += "<p style='color:#94a3b8;margin:0;'>Local Hardware Control via Wi-Fi</p>";
  html += "<div class='status-box'>LED Status: ";
  if (ledState == HIGH) {
    html += "<span class='status-on'>ON (HIGH - 3.3V)</span>";
  } else {
    html += "<span class='status-off'>OFF (LOW - 0V)</span>";
  }
  html += "</div>";
  html += "<div class='btn-group'>";
  html += "<a href='/on' class='btn btn-on'>TURN ON</a>";
  html += "<a href='/off' class='btn btn-off'>TURN OFF</a>";
  html += "</div>";
  html += "<div class='meta'>IP: " + WiFi.localIP().toString() + " • Pin: GPIO 2</div>";
  html += "</div></body></html>";

  server.send(200, "text/html", html);
}

// Handler to actuate LED ON
void handleLEDOn() {
  digitalWrite(LED_PIN, HIGH);
  Serial.println("[HTTP] Route /on triggered -> GPIO 2 driven HIGH");
  server.sendHeader("Location", "/");
  server.send(303); // 303 See Other redirect back to root
}

// Handler to actuate LED OFF
void handleLEDOff() {
  digitalWrite(LED_PIN, LOW);
  Serial.println("[HTTP] Route /off triggered -> GPIO 2 driven LOW");
  server.sendHeader("Location", "/");
  server.send(303); // 303 See Other redirect back to root
}

// Handler for undefined routes (404 Not Found)
void handleNotFound() {
  String message = "404 Not Found\n\n";
  message += "URI: " + server.uri() + "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  server.send(404, "text/plain", message);
}

void setup() {
  // Initialize UART serial communications
  Serial.begin(115200);
  delay(500);
  Serial.println("\n==========================================");
  Serial.println("  ESP32 Web Server LED Control Initializing ");
  Serial.println("==========================================");

  // Configure hardware GPIO 2 as an output
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Start with LED OFF

  // Connect to the local Wi-Fi network in Station (STA) mode
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi SSID: ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n[WiFi] Connected successfully!");
  Serial.print("[WiFi] Assigned Local IP Address: http://");
  Serial.println(WiFi.localIP());

  // Register routing callback endpoints
  server.on("/", HTTP_GET, handleRoot);
  server.on("/on", HTTP_GET, handleLEDOn);
  server.on("/off", HTTP_GET, handleLEDOff);
  server.onNotFound(handleNotFound);

  // Start the HTTP server listener
  server.begin();
  Serial.println("[HTTP] Server listening on port 80.");
}

void loop() {
  // Listen for and service incoming client HTTP socket connections
  server.handleClient();
}