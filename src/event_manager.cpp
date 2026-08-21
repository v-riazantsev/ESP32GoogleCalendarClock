#include "event_manager.h"

#include <esp32-hal.h>

#include "calendar_api.h"

EventManager::EventManager(int refreshIntervalMs)
    : _refreshIntervalMs(refreshIntervalMs) {
  fetchLoop();
}

std::vector<Event> EventManager::getEvents() const {
  std::lock_guard<std::mutex> lock(_mutex);
  return _events;
}

void EventManager::updateFromApi() {
  std::lock_guard<std::mutex> lock(_mutex);
  CalendarApi api;
  _events = api.fetchEvents();
}

void EventManager::fetchLoop() {
  while (true) {
    updateFromApi();
    delay(_refreshIntervalMs);
  }
}