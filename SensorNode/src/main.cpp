#include <Arduino.h>
#include <Wire.h>
#include <esp_task_wdt.h>
#include <esp_system.h>

#include "sensors.h"
#include "connection_handler.h"
#include "ota_handler.h"
#include "watchdog.h"

static constexpr unsigned long PUBLISH_INTERVAL_MS = 30000;
static constexpr unsigned long MAX_SILENCE_MS = 30UL * 60UL * 1000UL;

static unsigned long lastGoodPublish = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) delay(10);
  delay(500);
  Serial.printf("%s booting, reset reason %d\n", NODE_NAME, esp_reset_reason());

  Wire.begin(SDA, SCL);
  if (!sensors_init(Wire)) Serial.println("sensors_init reported failure");

  connection_init();
  ota_init(NODE_NAME);
  watchdog_init();
  lastGoodPublish = millis();

  Serial.printf("TWDT subscribed: %s\n",
                esp_task_wdt_status(NULL) == ESP_OK ? "yes" : "NO");
}

void loop() {
  ota_loop();
  if (ota_in_progress()) return;

  esp_task_wdt_reset();
  connection_loop();
  sensors_tick();

  static unsigned long lastMsg = 0;
  unsigned long now = millis();

  if (now - lastMsg > PUBLISH_INTERVAL_MS) {
    lastMsg = now;
    Reading r;
    if (sensors_read(r) && publish(NODE_TOPIC, r)) lastGoodPublish = now;
  }

  if (now - lastGoodPublish > MAX_SILENCE_MS) {
    Serial.println("30 min without a publish - rebooting");
    Serial.flush();
    delay(100);
    ESP.restart();
  }
}