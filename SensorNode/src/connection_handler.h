#pragma once

#include <WiFi.h>
#include <PubSubClient.h>

#include "sensors.h"
#include "secrets.h"

extern WiFiClient espClient;
extern PubSubClient client;
extern unsigned long lastMsg;
extern char msg[64];
extern int value;

extern int ledPin;

void setup_wifi();
void connection_init();
void callback(char* topic, byte* message, unsigned int length);
void reconnect();
bool publish(const char* topic, const Reading& r);