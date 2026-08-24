#pragma once
#include <math.h>

struct Reading {
    float pm25_ugm3 = NAN;
    float co2 = NAN;
    float temp = NAN;
    float humidity = NAN;

};

bool sensors_init();
bool sensors_read(Reading& out);