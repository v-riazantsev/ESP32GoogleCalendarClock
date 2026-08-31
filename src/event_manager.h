#pragma once

#include <Arduino.h>

#include <mutex>
#include <vector>

#include "calendar_api.h"
#include "event.h"

class EventManager {
 public:
  EventManager(CalendarApi& api, uint32_t refreshIntervalMs);
  std::vector<Event> getEvents() const;
  void update();
  void startTask();  // Start the fetch loop in a separate task
  void stopTask();   // Stop the fetch loop task

 private:
  CalendarApi& _api;
  uint32_t _refreshIntervalMs;

  std::vector<Event> _events;
  mutable std::mutex _mutex;
  TaskHandle_t _taskHandle;
  bool _isRunning;
  void fetchLoop();
};