#include "ota_handler.h"
#include "secrets.h"
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

static bool otaActive = false;

bool ota_in_progress() { return otaActive; }

void ota_init(const char* hostname) {
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.setPassword(OTA_PASS);

  ArduinoOTA.onStart([]() {
    otaActive = true;
    esp_task_wdt_delete(NULL);
    Serial.println("OTA start");
  });

  ArduinoOTA.onEnd([]() {
    otaActive = false;          // board reboots immediately after this
    Serial.println("\nOTA done");
  });

  ArduinoOTA.onProgress([](unsigned int p, unsigned int t) {
    Serial.printf("OTA %u%%\r", (p * 100) / t);
  });

  ArduinoOTA.onError([](ota_error_t e) {
    otaActive = false;
    esp_task_wdt_add(NULL);
    Serial.printf("OTA error %u\n", e);
  });

  ArduinoOTA.begin();
}

void ota_loop() { ArduinoOTA.handle(); }