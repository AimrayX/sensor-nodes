#pragma once

#include <WiFi.h>
#include <PubSubClient.h>

#include "sensors.h"

const char* ssid = "REPLACE_WITH_SSID";
const char* password = "REPLACE_WITH_PASSWORD";

const char* mqtt_server = "MQTT_BROKER_IP_ADDRESS";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

const int ledPin = 4;

void setup_wifi();
void connection_init();
void callback(char* topic, byte* message, unsigned int length);
void reconnect();
//void publish(NODE_TOPIC, Reading& r);