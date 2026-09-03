#pragma once

#include <Arduino.h>
#include <time.h>

class TimeManager {
 public:
  TimeManager();

  void begin();

  // Current Unix timestamp (UTC)
  time_t now() const;

  // Convert Unix timestamp to local time
  tm localTime(time_t timestamp) const;

  // Convert timestamp to position on a 12-hour clock.
  float clock12hPct(time_t timestamp) const;

  float nowHour() const;

  bool synchronized() const {
    return now() > 1609459200;
  }  // 2021-01-01 00:00:00 UTC
};