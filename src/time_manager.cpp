#include "time_manager.h"

#include "config.h"

TimeManager::TimeManager() {}

void TimeManager::begin() {
  configTzTime(TIMEZONE, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);

  Serial.println("Time synchronization started.");
}

time_t TimeManager::now() const {
  time_t currentTime;
  time(&currentTime);

  return currentTime;
}

tm TimeManager::localTime(time_t timestamp) const {
  tm result;

  localtime_r(&timestamp, &result);
  return result;
}

float TimeManager::clock12hPct(time_t timestamp) const {
  tm local = localTime(timestamp);

  float seconds = local.tm_hour * 3600.0f + local.tm_min * 60.0f + local.tm_sec;

  const float twelveHours = 12.0f * 3600.0f;

  // Repeat every 12 hours.
  float position = fmod(seconds, twelveHours) / twelveHours;

  return position;
}

float TimeManager::nowHour() const {
  time_t currentTime = now();
  tm local = localTime(currentTime);

  float hour = local.tm_hour + local.tm_min / 60.0f + local.tm_sec / 3600.0f;

  return hour;
}