#pragma once

#include <mutex>
#include <vector>

#include "event.h"

class EventManager {
 public:
  EventManager(int refreshIntervalMs);
  std::vector<Event> getEvents() const;
  void updateFromApi();

 private:
  std::vector<Event> _events;
  int _refreshIntervalMs;
  mutable std::mutex _mutex;
  void fetchLoop();
};