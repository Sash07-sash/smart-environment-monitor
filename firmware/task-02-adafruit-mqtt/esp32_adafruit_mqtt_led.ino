#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Wi-Fi Configuration
#define WLAN_SSID       "YOUR_WIFI_SSID"
#define WLAN_PASS       "YOUR_WIFI_PASSWORD"

// Adafruit IO Configuration
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // Standard unencrypted MQTT port
#define AIO_USERNAME    "satheesh"              // Your Adafruit IO Username
#define AIO_KEY         "aio_xxxxxxxxxxxxxxxxxxxxxxxxxxxx" // Your Adafruit IO Active Key
#define MQTT_PASS       AIO_KEY                // MQTT Password mapping for Adafruit IO

// Hardware Pin Definition
#define LED_PIN 2

// Create an ESP32 WiFiClient class to connect to the MQTT server
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server credentials
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Setup a feed called 'led-control' for subscribing to changes from Adafruit IO
Adafruit_MQTT_Subscribe led_feed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/led-control");

// Function prototype for ensuring continuous MQTT connection
void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n==========================================");
  Serial.println("  ESP32 Adafruit IO MQTT Client Starting   ");
  Serial.println("==========================================");

  // Initialize LED output pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Initial state: OFF

  // Connect to Wi-Fi access point
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(WLAN_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Connected successfully!");
  Serial.print("[WiFi] IP Address: ");
  Serial.println(WiFi.localIP());

  // Setup MQTT subscription for the LED feed
  mqtt.subscribe(&led_feed);
  Serial.println("[MQTT] Subscribed to topic: " AIO_USERNAME "/feeds/led-control");
}

void loop() {
  // Ensure the connection to the MQTT server is alive
  MQTT_connect();

  // Read subscription packets with a non-blocking 200ms timeout
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(200))) {
    if (subscription == &led_feed) {
      Serial.print("[MQTT Update] Received payload: ");
      char *payload = (char *)led_feed.lastread;
      Serial.println(payload);

      // Evaluate received payload and actuate hardware
      if (strcmp(payload, "ON") == 0 || strcmp(payload, "1") == 0) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("[ACTUATOR] GPIO 2 -> HIGH (LED ON)");
      } else if (strcmp(payload, "OFF") == 0 || strcmp(payload, "0") == 0) {
        digitalWrite(LED_PIN, LOW);
        Serial.println("[ACTUATOR] GPIO 2 -> LOW (LED OFF)");
      }
    }
  }

  // Ping the broker to maintain the keep-alive heartbeat connection
  if (!mqtt.ping()) {
    mqtt.disconnect();
  }
}

// Function to connect and reconnect as necessary to the MQTT broker
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to Adafruit IO MQTT Broker... ");

  uint8_t retries = 5;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for success
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds before retrying
    retries--;
    if (retries == 0) {
      Serial.println("MQTT connection failed after 5 retries. Resetting board...");
      ESP.restart();
    }
  }
  Serial.println("\n[MQTT] Connected to Adafruit IO successfully!");
}