#pragma once
#include <stddef.h>
#include <math.h>

struct Reading {
    float pm1      = NAN; // µg/m³
    float pm25     = NAN; // µg/m³
    float pm10     = NAN; // µg/m³
    float co2      = NAN; // ppm
    float temp     = NAN; // °C
    float humidity = NAN; // %
    float pressure = NAN; // Pa

};

#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>

bool sensors_init(TwoWire &wire);
bool sensors_read(Reading& out);
#endif
