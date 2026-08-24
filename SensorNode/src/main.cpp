#include <Arduino.h>
#include <Wire.h>
#include "sensors.h"
#include "connection_handler.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Hello from the C6 on Arduino!");
  sensors_init();
  connection_init();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Reading r;

  

  delay(30000);
  
  

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    if (sensors_read(r)) publish(NODE_TOPIC, r); 

  }
}