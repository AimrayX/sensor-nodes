#pragma once
#include <math.h>

struct Reading {
    float pm10_standard = NAN;
    float pm25_standard = NAN;
    float pm100_standard = NAN;
    float co2 = NAN;
    float temp = NAN;
    float humidity = NAN;
    float pressure = NAN;

};

bool sensors_init();
bool sensors_read(Reading& out);