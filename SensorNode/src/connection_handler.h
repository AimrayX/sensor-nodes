#pragma once

#include <WiFi.h>
#include <PubSubClient.h>

#include "sensors.h"
#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

const char* mqtt_server = MQTT_HOST;
const uint16_t mqtt_port = MQTT_PORT;

WiFiClient espClient;
extern PubSubClient client;
long lastMsg = 0;
char msg[50];
int value = 0;

const int ledPin = 4;

void setup_wifi();
void connection_init();
void callback(char* topic, byte* message, unsigned int length);
void reconnect();
bool publish(const char* topic, const Reading& r);