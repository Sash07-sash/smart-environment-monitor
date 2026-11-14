#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DHTesp.h"

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* firestoreEndpoint = "https://firestore.googleapis.com/v1/projects/smart-environment-monito-3c43f/databases/(default)/documents/sensorData";

#define DHT_PIN      4
#define LDR_PIN      34
#define RELAY_PIN    18

DHTesp dht;
String operationalMode = "automatic";
int lightThreshold = 500;
unsigned long lastTelemetryTime = 0;
const unsigned long telemetryInterval = 5000;

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  dht.setup(DHT_PIN, DHTesp::DHT11);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Connected! IP: " + WiFi.localIP().toString());
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastTelemetryTime >= telemetryInterval) {
    lastTelemetryTime = currentMillis;

    float temperature = dht.getTemperature();
    float humidity = dht.getHumidity();
    int ldrRaw = analogRead(LDR_PIN);
    int lightLux = map(4095 - ldrRaw, 0, 4095, 0, 1000);

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("[ERROR] Failed to read from DHT11!");
      return;
    }

    String bulbState = "OFF";
    if (operationalMode == "automatic") {
      if (lightLux < (lightThreshold - 30)) {
        digitalWrite(RELAY_PIN, HIGH);
        bulbState = "ON";
      } else if (lightLux > (lightThreshold + 30)) {
        digitalWrite(RELAY_PIN, LOW);
        bulbState = "OFF";
      }
    }

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(firestoreEndpoint);
      http.addHeader("Content-Type", "application/json");

      StaticJsonDocument<512> doc;
      JsonObject fields = doc.createNestedObject("fields");

      JsonObject tObj = fields.createNestedObject("temperature");
      tObj["doubleValue"] = temperature;

      JsonObject hObj = fields.createNestedObject("humidity");
      hObj["doubleValue"] = humidity;

      JsonObject lObj = fields.createNestedObject("light");
      lObj["integerValue"] = lightLux;

      JsonObject bObj = fields.createNestedObject("bulbStatus");
      bObj["stringValue"] = bulbState;

      JsonObject mObj = fields.createNestedObject("mode");
      mObj["stringValue"] = operationalMode;

      JsonObject tsObj = fields.createNestedObject("timestamp");
      tsObj["integerValue"] = millis();

      String requestBody;
      serializeJson(doc, requestBody);

      int httpResponseCode = http.POST(requestBody);
      Serial.printf("[Cloud Firestore] Log Response: %d\n", httpResponseCode);
      http.end();
    }
  }
}
