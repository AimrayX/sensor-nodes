#include <Arduino.h>
#include <Wire.h>

#include "sensors.h"
#include "connection_handler.h"


void setup() {
  Serial.begin(115200);
  Serial.println("Hello from the C6 on Arduino!");

  Wire.begin(SDA, SCL);
  sensors_init(Wire);
  connection_init();
}

void loop() {
  connection_loop();

  static unsigned long lastMsg = 0; 
  unsigned long now = millis();
  if (now - lastMsg > 30000) {
    lastMsg = now;
    Reading r;
    if (sensors_read(r)) publish(NODE_TOPIC, r); 

  }
}