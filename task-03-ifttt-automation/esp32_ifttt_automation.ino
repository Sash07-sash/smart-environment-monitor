#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Wi-Fi Access Point Credentials
#define WLAN_SSID       "YOUR_WIFI_SSID"
#define WLAN_PASS       "YOUR_WIFI_PASSWORD"

// Adafruit IO Broker Credentials
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "satheesh"
#define AIO_KEY         "aio_xxxxxxxxxxxxxxxxxxxxxxxxxxxx"

// Pin configuration for Actuator Relay / LED
#define ACTUATOR_PIN    2

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe automation_feed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/led-control");

void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n==========================================");
  Serial.println("  ESP32 IFTTT Cloud Automation Receiver    ");
  Serial.println("==========================================");

  pinMode(ACTUATOR_PIN, OUTPUT);
  digitalWrite(ACTUATOR_PIN, LOW); // Safe default: Relay/LED OFF

  // Connect to Local Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Connected! IP: " + WiFi.localIP().toString());

  // Subscribe to Adafruit feed written by IFTTT
  mqtt.subscribe(&automation_feed);
  Serial.println("[MQTT] Ready for IFTTT automation commands on feed: led-control");
}

void loop() {
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(200))) {
    if (subscription == &automation_feed) {
      char *payload = (char *)automation_feed.lastread;
      Serial.print("[IFTTT Event Received] Payload: ");
      Serial.println(payload);

      if (strcmp(payload, "ON") == 0 || strcmp(payload, "1") == 0) {
        digitalWrite(ACTUATOR_PIN, HIGH);
        Serial.println("[AUTOMATION] Trigger matched: Actuator switched ON");
      } else if (strcmp(payload, "OFF") == 0 || strcmp(payload, "0") == 0) {
        digitalWrite(ACTUATOR_PIN, LOW);
        Serial.println("[AUTOMATION] Trigger matched: Actuator switched OFF");
      }
    }
  }

  if (!mqtt.ping()) {
    mqtt.disconnect();
  }
}

void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) return;

  Serial.print("Connecting to MQTT broker... ");
  uint8_t retries = 5;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      Serial.println("Rebooting ESP32 due to persistent network failure...");
      ESP.restart();
    }
  }
  Serial.println("\n[MQTT] Connected and listening.");
}