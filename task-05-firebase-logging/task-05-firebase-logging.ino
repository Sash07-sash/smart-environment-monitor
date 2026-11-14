#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DHTesp.h"

// Wi-Fi Configuration
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Firebase REST API Endpoint
const char* firestoreEndpoint = "https://firestore.googleapis.com/v1/projects/smart-environment-monito-3c43f/databases/(default)/documents/sensorData";

// Hardware Pin Assignments
#define DHT_PIN      4
#define LDR_PIN      34   // ADC1 Channel 6
#define RELAY_PIN    18

DHTesp dht;

// Operational Configuration
String operationalMode = "automatic";
int lightThreshold = 500;
unsigned long lastTelemetryTime = 0;
const unsigned long telemetryInterval = 5000; // Log every 5 seconds

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n=======================================================");
  Serial.println("  ESP32 Smart Environment Multi-Sensor Logger Running  ");
  Serial.println("=======================================================");

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Start with light OFF

  // Initialize DHT11 sensor
  dht.setup(DHT_PIN, DHTesp::DHT11);

  // Initialize Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Connected successfully! IP: " + WiFi.localIP().toString());
}

void loop() {
  unsigned long currentMillis = millis();

  // Execute telemetry cycle at designated sampling interval
  if (currentMillis - lastTelemetryTime >= telemetryInterval) {
    lastTelemetryTime = currentMillis;

    // Read physical sensors
    float temperature = dht.getTemperature();
    float humidity = dht.getHumidity();
    int ldrRaw = analogRead(LDR_PIN);
    // Convert 12-bit ADC (0-4095) to approximate illuminance lux
    int lightLux = map(4095 - ldrRaw, 0, 4095, 0, 1000);

    // Check sensor integrity
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("[ERROR] Failed to read from DHT11 sensor!");
      return;
    }

    Serial.printf("[SENSORS] Temp: %.1f °C | Humidity: %.0f %% | Light: %d lux\n", temperature, humidity, lightLux);

    // Automatic Threshold Control Logic
    String bulbState = "OFF";
    if (operationalMode == "automatic") {
      if (lightLux < lightThreshold) {
        digitalWrite(RELAY_PIN, HIGH);
        bulbState = "ON";
      } else {
        digitalWrite(RELAY_PIN, LOW);
        bulbState = "OFF";
      }
    }

    // Stream telemetry to Cloud Firestore via REST
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(firestoreEndpoint);
      http.addHeader("Content-Type", "application/json");

      // Build JSON document for Firestore REST API
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
      Serial.printf("[Cloud Firestore] HTTP Response: %d\n", httpResponseCode);
      http.end();
    }
  }
}