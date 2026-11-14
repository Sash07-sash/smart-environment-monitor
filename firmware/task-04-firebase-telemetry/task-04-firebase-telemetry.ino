#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* firestoreEndpoint = "https://firestore.googleapis.com/v1/projects/smart-environment-monito-3c43f/databases/(default)/documents/sensorData/latest";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(firestoreEndpoint);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> doc;
    JsonObject fields = doc.createNestedObject("fields");

    JsonObject tObj = fields.createNestedObject("temperature");
    tObj["doubleValue"] = 25.4;

    JsonObject hObj = fields.createNestedObject("humidity");
    hObj["doubleValue"] = 62.0;

    JsonObject lObj = fields.createNestedObject("light");
    lObj["integerValue"] = 430;

    String requestBody;
    serializeJson(doc, requestBody);

    int httpCode = http.PATCH(requestBody);
    Serial.printf("[Firestore Update] Response: %d\n", httpCode);
    http.end();
  }
  delay(5000);
}
