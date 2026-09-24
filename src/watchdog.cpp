#include <esp_task_wdt.h>
#include <Arduino.h>

void watchdog_init() {
  esp_task_wdt_config_t cfg = {
    .timeout_ms = 30000, .idle_core_mask = 0, .trigger_panic = true,
  };
  esp_err_t err = esp_task_wdt_init(&cfg);
  if (err == ESP_ERR_INVALID_STATE) err = esp_task_wdt_reconfigure(&cfg);
  if (err == ESP_OK) esp_task_wdt_add(NULL);
  else Serial.printf("TWDT setup failed: %d\n", err);
}