#include "sensors.h"

#include <Adafruit_BME680.h>
#include <SensirionI2cScd4x.h>

static Adafruit_BME680 bme(&Wire);
static SensirionI2cScd4x scd;

static bool bmeOk = false;
static bool scdOk = false;

static float co2Cache = NAN;
static bool  co2Fresh = false;

static unsigned long lastScdPoll = 0;
static constexpr unsigned long SCD_POLL_MS = 1000;

bool sensors_init(TwoWire &wire) {
  Serial.println(F("Setting up BME688"));

  bmeOk = bme.begin(BME68X_DEFAULT_ADDRESS, true);   // 0x77
  if (!bmeOk) bmeOk = bme.begin(0x76, true);         // common alternate
  if (!bmeOk) {
    Serial.println("BME688 not found, continuing without it");
  } else {
    bme.setTemperatureOversampling(BME680_OS_1X);
    bme.setHumidityOversampling(BME680_OS_1X);
    bme.setPressureOversampling(BME680_OS_1X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_0);
    bme.setGasHeater(0, 0);
  }

  Serial.println("BME688 setup finished");
  Serial.println();

  Serial.println(F("Setting up SCD41"));

  scdOk = true;
  scd.begin(wire, 0x62);
  delay(30);

  scd.wakeUp();
  delay(30);

  if (scd.stopPeriodicMeasurement()) {
    Serial.println("SCD41 stopPeriodicMeasurement returned an error (ok if idle)");
  }
  delay(500);   // datasheet: stop takes up to 500 ms to complete

  if (scd.reinit()) {
    Serial.println("SCD41 reinit() failed");
    scdOk = false;
  }

  uint64_t serialNumber = 0;
  if (scd.getSerialNumber(serialNumber)) {
    Serial.println("SCD41 getSerialNumber() failed");
    scdOk = false;
  } else {
    Serial.printf("SCD41 serial number: %04X%04X%04X\n",
                  (unsigned)((serialNumber >> 32) & 0xFFFF),
                  (unsigned)((serialNumber >> 16) & 0xFFFF),
                  (unsigned)(serialNumber & 0xFFFF));
  }

  if (scd.startPeriodicMeasurement()) {
    Serial.println("SCD41 startPeriodicMeasurement() failed");
    scdOk = false;
  }

  Serial.println("SCD41 setup finished");
  Serial.println();

  return bmeOk && scdOk;
}

void sensors_tick() {
  if (!scdOk) return;

  const unsigned long now = millis();
  if (now - lastScdPoll < SCD_POLL_MS) return;   // don't hammer the I2C bus
  lastScdPoll = now;

  bool ready = false;
  // Sensirion's generated drivers return int16_t, 0 == NO_ERROR.
  if (scd.getDataReadyStatus(ready) != 0 || !ready) return;

  uint16_t co2 = 0;
  float t = 0.0f, rh = 0.0f;
  if (scd.readMeasurement(co2, t, rh) == 0 && co2 != 0) {
    co2Cache = co2;      // a CO2 of 0 means the sample is invalid
    co2Fresh = true;
  }
}

bool sensors_read(Reading& out) {
  bool any = false;

  if (bmeOk) {
    if (bme.performReading()) {
      out.temp     = bme.temperature;
      out.humidity = bme.humidity;
      out.pressure = bme.pressure;
      any = true;
    } else {
      Serial.println("BME688 performReading() failed");
    }
  }

  if (co2Fresh) {
    out.co2  = co2Cache;
    co2Fresh = false;
    any = true;
  }

  return any;
}