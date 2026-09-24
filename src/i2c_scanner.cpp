#include <Arduino.h>
#include <Wire.h>

void setup() {
    Serial.begin(115200);
    delay(2000);
    Wire.begin(SDA, SCL);
}

void loop() {
    int found = 0;
    for (uint8_t a = 1; a < 127; a++) {
        Wire.beginTransmission(a);
        if (Wire.endTransmission() == 0) { Serial.printf("found 0x%02X\n", a); found++;}
    }
    Serial.printf("%d device(s)\n\n", found);
    delay(3000);
}