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
void sensors_tick();

// Fills `out` with whatever is currently available. Duty-cycled values
// (PM, CO2) stay cached and will be offered again on the next call until
// sensors_mark_sent() confirms they actually left the board.
bool sensors_read(Reading& out);

// Call only after a successful publish. Clears the cached duty-cycled
// values so they are not republished on the following cycle.
void sensors_mark_sent();
#endif