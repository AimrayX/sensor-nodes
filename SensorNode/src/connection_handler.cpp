#include "connection_handler.h"
#include "secrets.h"
#include "payload.h"

#include <esp_system.h>

espMqttClient mqttClient;

static unsigned long lastReconnect = 0;
static volatile bool statusPending = false;

static void onMqttConnect(bool sessionPresent) {
  statusPending = true;
}

static void onMqttDisconnect(espMqttClientTypes::DisconnectReason reason) {
  Serial.printf("MQTT lost, reason %u\n", static_cast<uint8_t>(reason));
  lastReconnect = millis();
}

void connection_init() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) delay(250);

  if (!WiFi.isConnected()) {
    Serial.printf("WiFi failed, status=%d\n", WiFi.status());
  } else {
    Serial.printf("WiFi ok, ip=%s rssi=%d\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
  }

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.setServer(MQTT_HOST, 1883);
  mqttClient.setCredentials(MQTT_USER, MQTT_PASS);
  mqttClient.setClientId(NODE_NAME);
}

void connection_loop() {
  mqttClient.loop();

  static unsigned long lastWifiTry = 0;
  if (!WiFi.isConnected()) {
    if (millis() - lastWifiTry > 10000) {
      lastWifiTry = millis();
      WiFi.disconnect(false, false);
      WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
    return;
  }

  if (statusPending && mqttClient.connected()) {
    statusPending = false;
    char buf[96];
    snprintf(buf, sizeof(buf), "{\"reset\":%d,\"uptime\":%lu}",
             (int)esp_reset_reason(), millis() / 1000);
    mqttClient.publish("flat/" NODE_NAME "/status", 0, true, buf);
  }

  if (!mqttClient.connected() && millis() - lastReconnect > 5000) {
    lastReconnect = millis();
    Serial.println("mqtt connect attempt");
    mqttClient.connect();
  }
}

bool publish(const char* topic, const Reading& r) {
  char payload[192];
  int len = build_payload(payload, sizeof(payload), r);
  if (len < 0) { Serial.println("build_payload failed or empty"); return false; }

  if (!mqttClient.connected()) {
    Serial.println("publish skipped, mqtt not connected");
    return false;
  }

  Serial.printf("pub %s -> %s\n", topic, payload);
  return mqttClient.publish(topic, 0, true, payload) != 0;
}