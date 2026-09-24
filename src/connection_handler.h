#pragma once

#include <WiFi.h>
#include <espMqttClient.h>

#include "sensors.h"

extern espMqttClient mqttClient;

void connection_init();
void connection_loop();
bool publish(const char* topic, const Reading& r);