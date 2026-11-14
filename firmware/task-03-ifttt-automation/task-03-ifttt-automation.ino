#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define WLAN_SSID       "YOUR_WIFI_SSID"
#define WLAN_PASS       "YOUR_WIFI_PASSWORD"

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "satheesh"
#define AIO_KEY         "aio_xxxxxxxxxxxxxxxxxxxxxxxxxxxx"

#define ACTUATOR_PIN    2

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe automation_feed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/led-control");

void MQTT_connect();

void setup() {
  Serial.begin(115200);
  pinMode(ACTUATOR_PIN, OUTPUT);
  digitalWrite(ACTUATOR_PIN, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Connected! IP: " + WiFi.localIP().toString());

  mqtt.subscribe(&automation_feed);
}

void loop() {
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(200))) {
    if (subscription == &automation_feed) {
      char *payload = (char *)automation_feed.lastread;
      Serial.printf("[IFTTT Event] Payload: %s\n", payload);

      if (strcmp(payload, "ON") == 0 || strcmp(payload, "1") == 0) {
        digitalWrite(ACTUATOR_PIN, HIGH);
      } else if (strcmp(payload, "OFF") == 0 || strcmp(payload, "0") == 0) {
        digitalWrite(ACTUATOR_PIN, LOW);
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

  uint8_t retries = 5;
  while ((ret = mqtt.connect()) != 0) {
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) ESP.restart();
  }
}
