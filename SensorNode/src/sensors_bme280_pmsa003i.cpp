#include "sensors.h"

#include <Adafruit_BME280.h>
#include <Adafruit_PM25AQI.h>

static Adafruit_BME280 bme;
static Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
static bool bmeOk = false;
static bool aqiOk = false;

enum class PmState { Sleeping, WarmingUp };
static PmState pmState = PmState::WarmingUp;

static unsigned long pmCycleStart = 0;  // when the current 5 min period began
static unsigned long pmPhaseStart = 0;  // when the current phase began

static constexpr unsigned long PM_INTERVAL_MS = 5UL * 60UL * 1000UL;
static constexpr unsigned long PM_WARMUP_MS   = 30UL * 1000UL;

static float pmCache1 = NAN, pmCache25 = NAN, pmCache10 = NAN;
static bool  pmFresh  = false;

bool sensors_init(TwoWire &wire) {
  pinMode(PIN_PM_SET, OUTPUT);
  digitalWrite(PIN_PM_SET, HIGH);
  delay(100);

  Serial.println(F("Setting up BME280"));

  bmeOk = bme.begin(BME280_ADDRESS, &wire);            // 0x77
  if (!bmeOk) bmeOk = bme.begin(BME280_ADDRESS_ALTERNATE, &wire);  // 0x76
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

  Serial.println(F("Setting up PMSA003I"));
  aqiOk = aqi.begin_I2C(&wire);
  if (!aqiOk) {
    Serial.println("Could not find PMSA003I sensor, check wiring!");
  }
  Serial.println("PMSA003I setup finished");
  Serial.println();

  // Start in WarmingUp so the first reading arrives ~30 s after boot rather
  // than 5 minutes in.
  pmCycleStart = millis();
  pmPhaseStart = millis();
  pmState = PmState::WarmingUp;

  return bmeOk && aqiOk;
}

void sensors_tick() {
  const unsigned long now = millis();

  switch (pmState) {
    case PmState::Sleeping:
      if (now - pmCycleStart >= PM_INTERVAL_MS) {
        digitalWrite(PIN_PM_SET, HIGH);
        pmCycleStart = now;
        pmPhaseStart = now;
        pmState = PmState::WarmingUp;
      }
      break;

    case PmState::WarmingUp:
      if (now - pmPhaseStart >= PM_WARMUP_MS) {
        PM25_AQI_Data data;
        if (aqiOk && aqi.read(&data)) {
          pmCache1  = data.pm10_standard;
          pmCache25 = data.pm25_standard;
          pmCache10 = data.pm100_standard;
          pmFresh   = true;
        } else {
          Serial.println("PMSA003I read failed");
        }
        digitalWrite(PIN_PM_SET, LOW);
        pmPhaseStart = now;
        pmState = PmState::Sleeping;
      }
      break;
  }
}

bool sensors_read(Reading& out) {
  bool any = false;

  if (bmeOk) {
    if (bme.takeForcedMeasurement()) {
      out.temp     = bme.readTemperature();
      out.humidity = bme.readHumidity();
      out.pressure = bme.readPressure();
      any = true;
    } else {
      Serial.println("BME280 forced measurement failed");
    }
  }

  if (pmFresh) {
    out.pm1  = pmCache1;
    out.pm25 = pmCache25;
    out.pm10 = pmCache10;
    pmFresh  = false;
    any = true;
  }

  return any;
}