#include <unity.h>
#include <string.h>
#include "payload.h"

void setUp(void) {}
void tearDown(void) {}

void test_balcony_fields(void) {
  Reading r;
  r.temp = 25.73f; r.humidity = 46.48f; r.pressure = 96543.0f;
  r.pm1 = 10.0f; r.pm25 = 11.0f; r.pm10 = 12.0f;

  char buf[192];
  int n = build_payload(buf, sizeof(buf), r);

  TEST_ASSERT_GREATER_THAN(0, n);
  TEST_ASSERT_EQUAL_STRING(
    "{\"temp\":25.73,\"hum\":46.48,\"pressure\":96543,\"pm1\":10.0,\"pm25\":11.0,\"pm10\":12.0}",
    buf);
  TEST_ASSERT_EQUAL_INT((int)strlen(buf), n);
}

void test_bedroom_fields(void) {
  Reading r;
  r.temp = 21.4f; r.humidity = 52.0f; r.pressure = 96500.0f; r.co2 = 780.0f;

  char buf[192];
  TEST_ASSERT_GREATER_THAN(0, build_payload(buf, sizeof(buf), r));
  TEST_ASSERT_NOT_NULL(strstr(buf, "\"co2\":780"));
  TEST_ASSERT_NULL(strstr(buf, "pm25"));
}

void test_all_nan_gives_empty_object(void) {
  Reading r;                       // every field NAN
  char buf[192];
  TEST_ASSERT_GREATER_THAN(0, build_payload(buf, sizeof(buf), r));
  TEST_ASSERT_EQUAL_STRING("{}", buf);
}

void test_never_emits_nan(void) {
  Reading r; r.temp = 20.0f;
  char buf[192];
  build_payload(buf, sizeof(buf), r);
  TEST_ASSERT_NULL(strstr(buf, "nan"));
  TEST_ASSERT_NULL(strstr(buf, "NaN"));
}

void test_truncation_is_detected(void) {
  Reading r;
  r.temp = 25.0f; r.humidity = 46.0f; r.pressure = 96500.0f;
  char buf[20];                    // deliberately too small
  TEST_ASSERT_LESS_THAN(0, build_payload(buf, sizeof(buf), r));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_balcony_fields);
  RUN_TEST(test_bedroom_fields);
  RUN_TEST(test_all_nan_gives_empty_object);
  RUN_TEST(test_never_emits_nan);
  RUN_TEST(test_truncation_is_detected);
  return UNITY_END();
}