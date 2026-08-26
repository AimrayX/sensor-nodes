#include "sensors.h"

#include <Adafruit_BME280.h>
#include <Adafruit_PM25AQI.h>

static Adafruit_BME280 bme;
static Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
static bool bmeOk = false;
static bool aqiOk  = false;

bool sensors_init(TwoWire &wire) {
  Serial.println(F("Setting up BME280"));

  bmeOk = bme.begin(BME280_ADDRESS, &wire);
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
  aqiOk = aqi.begin_I2C();
  if (!aqiOk) {
    Serial.println("Could not find PMSA_003I sensor, check wiring!");
  }

  Serial.println("PMSA_003I setup finished");
  Serial.println();

  return true;
}

bool sensors_read(Reading& out) {
  PM25_AQI_Data data;

  if (aqi.read(&data)) {
    out.pm1 = data.pm10_standard;
    out.pm25 = data.pm25_standard;
    out.pm10 = data.pm100_standard;
  } else {
    Serial.print("Skipping PMSA003I due to error or no new data");
  }

  if (bmeOk && bme.takeForcedMeasurement()) {
  out.temp = bme.readTemperature();
  out.humidity = bme.readHumidity();
  out.pressure = bme.readPressure();
  } else {
    Serial.print("Skipping bme280 due to error or no new data");
  }

  

  return true;
}
