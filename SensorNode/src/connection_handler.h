#pragma once

#include <WiFi.h>
#include <espMqttClient.h>

#include "sensors.h"

extern espMqttClient mqttClient;

void setup_wifi();
void connection_init();
void connection_loop();
void callback(char* topic, byte* message, unsigned int length);
void reconnect();
bool publish(const char* topic, const Reading& r);