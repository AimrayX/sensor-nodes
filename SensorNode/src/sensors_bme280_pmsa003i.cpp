#include "sensors.h"

#include <Adafruit_BME280.h>
#include <Adafruit_PM25AQI.h>

static Adafruit_BME280 bme;
static Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

bool sensors_init() {

  return true;
}

bool sensors_read(Reading& out) {
  out.co2 = NAN;

  return true;
}
