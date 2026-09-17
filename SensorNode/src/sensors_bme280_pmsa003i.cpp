#include "sensors.h"

#include <Adafruit_BME280.h>
#include <Adafruit_PM25AQI.h>

static Adafruit_BME280 bme;
static Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
static bool bmeOk = false;
static bool aqiOk = false;

// The PM sensor is duty-cycled: the fan is the wear item, so it runs for
// ~38 s out of every 5 min. Wake -> WarmingUp (fan stabilises, output is
// not trustworthy) -> Sampling (poll repeatedly, keep the last good frame)
// -> Sleeping.
enum class PmState { Sleeping, WarmingUp, Sampling };
static PmState pmState = PmState::WarmingUp;

static unsigned long pmCycleStart = 0;  // when the current 5 min period began
static unsigned long pmPhaseStart = 0;  // when the current phase began
static unsigned long pmLastPoll   = 0;  // last aqi.read() attempt

static constexpr unsigned long PM_INTERVAL_MS = 5UL * 60UL * 1000UL;
static constexpr unsigned long PM_WARMUP_MS   = 30UL * 1000UL;
static constexpr unsigned long PM_SAMPLE_MS   = 8UL * 1000UL;
static constexpr unsigned long PM_POLL_MS     = 1000UL;
static constexpr uint8_t       PM_MIN_FRAMES  = 2;

static PM25_AQI_Data pmLastFrame;
static uint8_t       pmGoodFrames = 0;

static float pmCache1 = NAN, pmCache25 = NAN, pmCache10 = NAN;
static unsigned long pmCacheAt = 0;
static bool  pmFresh  = false;

bool sensors_init(TwoWire &wire) {
  pinMode(PIN_PM_SET, OUTPUT);
  digitalWrite(PIN_PM_SET, HIGH);
  delay(100);

  Serial.println(F("Setting up BME280"));

  bmeOk = bme.begin(BME280_ADDRESS, &wire);
  if (!bmeOk) bmeOk = bme.begin(BME280_ADDRESS_ALTERNATE, &wire);
  if (bmeOk) {
    // Forced mode at 30 s intervals, so power is irrelevant - buy noise
    // reduction with oversampling instead. IIR stays OFF: in forced mode
    // the filter state persists between measurements, so with a 30 s gap
    // it would smear temperature over several minutes, and temperature is
    // a control input.
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X2,   // temperature
                    Adafruit_BME280::SAMPLING_X16,  // pressure
                    Adafruit_BME280::SAMPLING_X16,  // humidity
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

  // Start in WarmingUp so the first reading arrives ~38 s after boot
  pmCycleStart = millis();
  pmPhaseStart = millis();
  pmState = PmState::WarmingUp;

  // Degrade rather than fail: temperature/humidity/pressure alone is still
  // a useful node. PM availability is reported separately above.
  return bmeOk;
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
        pmGoodFrames = 0;
        pmLastPoll   = now - PM_POLL_MS;  // poll immediately on entry
        pmPhaseStart = now;
        pmState      = PmState::Sampling;
      }
      break;

    case PmState::Sampling:
      // The module emits a frame about once per second; polling faster just
      // re-reads the same buffer and hogs the I2C bus shared with the BME280.
      if (now - pmLastPoll >= PM_POLL_MS) {
        pmLastPoll = now;
        PM25_AQI_Data d;
        if (aqiOk && aqi.read(&d)) {   // read() validates the checksum
          pmLastFrame = d;
          pmGoodFrames++;
        }
      }

      if (now - pmPhaseStart >= PM_SAMPLE_MS) {
        if (pmGoodFrames >= PM_MIN_FRAMES) {
          // _env is the atmospheric-environment output (vs _standard, CF=1).
          // Note the naming: pm10_env is PM1.0, pm100_env is PM10.
          pmCache1  = pmLastFrame.pm10_env;
          pmCache25 = pmLastFrame.pm25_env;
          pmCache10 = pmLastFrame.pm100_env;
          pmCacheAt = now;
          pmFresh   = true;
        } else {
          Serial.printf("PMSA003I: only %u good frame(s), skipping cycle\n",
                        pmGoodFrames);
        }
        digitalWrite(PIN_PM_SET, LOW);
        pmPhaseStart = now;
        pmState      = PmState::Sleeping;
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

  // Offered until sensors_mark_sent() confirms delivery, but never longer
  // than one cycle - a value older than that is superseded, not retried.
  if (pmFresh) {
    if (millis() - pmCacheAt < PM_INTERVAL_MS) {
      out.pm1  = pmCache1;
      out.pm25 = pmCache25;
      out.pm10 = pmCache10;
      any = true;
    } else {
      pmFresh = false;
    }
  }

  return any;
}

void sensors_mark_sent() {
  pmFresh = false;
}