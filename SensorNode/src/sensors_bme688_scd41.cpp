#include "sensors.h"

#include <Adafruit_BME680.h>
#include <SensirionI2cScd4x.h>

Adafruit_BME680 bme;
SensirionI2cScd4x scd = SensirionI2cScd4x();
// TO-DO
bool sensors_init() {

    return true;
}
// TO-DO
bool sensors_read(Reading& out) {
  out.pm25_ugm3 = NAN;

  return true;
}
