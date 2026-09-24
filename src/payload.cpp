#include "payload.h"
#include <stdio.h>

int build_payload(char* buf, size_t len, const Reading& r) {
  int n = 0;
  bool first = true;

  auto add = [&](const char* key, float v, int dec) {
    if (isnan(v) || n < 0) return;
    size_t room = len - n;
    int w = snprintf(buf + n, room, "%s\"%s\":%.*f",
                     first ? "" : ",", key, dec, v);
    if (w < 0 || (size_t)w >= room) { n = -1; return; }
    n += w;
    first = false;
  };

  n = snprintf(buf, len, "{");
  add("temp", r.temp, 2);
  add("hum", r.humidity, 2);
  add("pressure", r.pressure, 0);
  add("co2", r.co2, 0);
  add("pm1", r.pm1, 1);
  add("pm25", r.pm25, 1);
  add("pm10", r.pm10, 1);

  if (n < 0 || (size_t)n >= len - 1) return -1;
  buf[n++] = '}';
  buf[n] = '\0';

  return n;
}