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
};