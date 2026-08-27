#include "sensors.h"

#include <Adafruit_BME680.h>
#include <SensirionI2cScd4x.h>

static Adafruit_BME680 bme;
static SensirionI2cScd4x scd = SensirionI2cScd4x();
static bool bmeOk = false;
static bool scdOk  = false;

bool sensors_init(TwoWire &wire) {
  Serial.println(F("Setting up BME688"));

  bmeOk = bme.begin(BME68X_DEFAULT_ADDRESS, &wire);
  if (!bmeOk) {
    Serial.println("BME688 not found, continuing without it");
  }

  Serial.println("BME688 setup finished");
  Serial.println();

  Serial.println(F("Setting up SCD41"));

  scdOk = true;
  scd.begin(wire, 0x62);
  uint64_t serialNumber = 0;
  delay(30);
  // Ensure sensor is in clean state
  if (scd.wakeUp()) {
    Serial.print("Error trying to execute wakeUp()");
    scdOk = false;
  }
  if (scd.stopPeriodicMeasurement()) {
    Serial.print("Error trying to execute stopPeriodicMeasurement()");
  }
  if (scd.reinit()) {
    Serial.print("Error trying to execute reinit()");
    scdOk = false;
  }

  if (scd.getSerialNumber(serialNumber)) {
    Serial.print("Error trying to execute getSerialNumber()");
    scdOk = false;
  }
  Serial.print("serial number: ");
  Serial.println();

  if (scd.startPeriodicMeasurement()) {
    Serial.print("Error trying to execute startPeriodicMeasurement()");
    scdOk = false;
  }

  Serial.println("SCD41 setup finished");
  Serial.println();

  if (!bmeOk || !scdOk) return false;

  return true;
}

bool sensors_read(Reading& out) {
  if (bmeOk) {
    bme.performReading();
    out.temp = bme.temperature;
    out.humidity = bme.humidity;
    out.pressure = bme.pressure;
  } else {
    Serial.print("Skipping bme688 due to error or no new data");
  }

  bool dataReady = false;
  if (scdOk && scd.getDataReadyStatus(dataReady) && dataReady) {
    uint16_t co2Concentration = 0;
    float temperature = 0.0;
    float relativeHumidity = 0.0;
    scd.readMeasurement(co2Concentration, temperature, relativeHumidity);
    out.co2 = co2Concentration;
  } else {
    Serial.print("Skipping scd41 due to error or no new data");
  }

  if(!bmeOk && (!scdOk || !dataReady)) {
    return false;
  }
  
  return true;
}
