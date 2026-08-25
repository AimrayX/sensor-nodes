#include "sensors.h"

#include <Adafruit_BME280.h>
#include <Adafruit_PM25AQI.h>

static Adafruit_BME280 bme;
static Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
static bool bmeOk = false;
static bool aqiOk  = false;

bool sensors_init() {
  Serial.println(F("Setting up BME280"));

  bmeOk = bme.begin();
  if (bmeOk) {
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::FILTER_OFF);
  } else {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
  }

  Serial.println("BME280 setup finished");
  Serial.println();

  Serial.println(F("Setting up PMSA_003I"));
  if (!aqi.begin_I2C()) {
    Serial.println("Could not find PMSA_003I sensor, check wiring!");
    while (1) delay(10);
  }

  Serial.println("PMSA_003I setup finished");
  Serial.println();

  return true;
}

bool sensors_read(Reading& out) {
  PM25_AQI_Data data;

  if (!aqi.read(&data)) {
    Serial.println("Could not read from AQI");
    
    delay(500);
    return false;
  }

  if (bmeOk && bme.takeForcedMeasurement()) {
  state.temperature = bme.readTemperature();
  state.humidity    = bme.readHumidity();
  state.pressure    = bme.readPressure();
  } else {
    state.temperature.valid = false;
    state.humidity.valid    = false;
    state.pressure.valid    = false;
  }

  out.pm1 = data.pm10_standard;
  out.pm25 = data.pm25_standard;
  out.pm10 = data.pm100_standard;
  bme.performreading();
  out.temp = bme.temperature;
  out.humidity = bme.humidity;
  out.pressure = bme.pressure;

  return true;
}
