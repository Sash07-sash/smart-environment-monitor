#include <WiFi.h>
#include <WebServer.h>

// Wi-Fi Credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

#define LED_PIN 2

WebServer server(80);

void handleRoot() {
  int ledState = digitalRead(LED_PIN);
  String html = "<!DOCTYPE html><html lang='en'><head>";
  html += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32 LED Controller</title>";
  html += "<style>body{font-family:sans-serif;text-align:center;background:#0d1527;color:#fff;padding-top:40px;}";
  html += ".btn{display:inline-block;padding:14px 30px;font-size:18px;margin:10px;border-radius:10px;text-decoration:none;font-weight:bold;color:#fff;}";
  html += ".btn-on{background:#10b981;}.btn-off{background:#ef4444;}";
  html += ".status{font-size:22px;margin:20px;font-weight:600;}</style></head><body>";
  html += "<h1>ESP32 Web Server</h1>";
  html += "<p class='status'>Current LED State: " + String(ledState ? "<span style='color:#10b981'>ON</span>" : "<span style='color:#ef4444'>OFF</span>") + "</p>";
  html += "<a href='/on' class='btn btn-on'>TURN ON</a>";
  html += "<a href='/off' class='btn btn-off'>TURN OFF</a>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleLEDOn() {
  digitalWrite(LED_PIN, HIGH);
  Serial.println("[HTTP] Route /on triggered -> GPIO 2 driven HIGH");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleLEDOff() {
  digitalWrite(LED_PIN, LOW);
  Serial.println("[HTTP] Route /off triggered -> GPIO 2 driven LOW");
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/on", HTTP_GET, handleLEDOn);
  server.on("/off", HTTP_GET, handleLEDOff);
  server.begin();
}

void loop() {
  server.handleClient();
}
