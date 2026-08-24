#include "connection_handler.h"
#include "secrets.h"

espMqttClient mqttClient;

static unsigned long lastReconnect = 0;

static void onMqttConnect(bool sessionPresent) {
  Serial.println("MQTT connected");
}

static void onMqttDisconnect(espMqttClientTypes::DisconnectReason reason) {
  Serial.printf("MQTT lost, reason %u\n", static_cast<uint8_t>(reason));
  lastReconnect = millis();
}

void connection_init() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(250);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.setServer(MQTT_HOST, 1883);
  mqttClient.setCredentials(MQTT_USER, MQTT_PASS);
  mqttClient.setClientId(MQTT_USER);
  mqttClient.connect();
}

void connection_loop() {
  if (!mqttClient.connected() && millis() - lastReconnect > 5000) {
      lastReconnect = millis();
    if (WiFi.isConnected()) mqttClient.connect();
  }
}

bool publish(const char* topic, const Reading& r) {
  char payload[192];
  snprintf(payload, sizeof(payload),
            "{\"temp\":%.2f,\"hum\":%.2f,\"co2\":%u,\"pm25_ugm3\":%.2f}",
            r.temp, r.humidity, r.co2, r.pm25_ugm3);
  return mqttClient.publish(topic, 0, true, payload) != 0;
}