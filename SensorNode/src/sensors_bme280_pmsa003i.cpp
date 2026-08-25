#include "sensors.h"

#include <Adafruit_BME280.h>
#include <Adafruit_PM25AQI.h>

static Adafruit_BME280 bme;
static Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

bool sensors_init() {
  Serial.println(F("Setting up BME280"));

  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1) delay(10);
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

  if(!bme.readPressure()) {
    Serial.println("Could not read from BME280");
    
    delay(500);
    return false;
  }

  out.pm10_standard = data.pm10_standard;
  out.pm25_standard = data.pm25_standard;
  out.pm100_standard = data.pm100_standard;
  out.temp = bme.readTemperature();
  out.humidity = bme.readHumidity();
  out.pressure = bme.readPressure();

  return true;
}
