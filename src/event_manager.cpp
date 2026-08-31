#include "event_manager.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "calendar_api.h"

EventManager::EventManager(CalendarApi& api, uint32_t refreshIntervalMs)
    : _api(api),
      _refreshIntervalMs(refreshIntervalMs),
      _taskHandle(nullptr),
      _isRunning(false) {}

std::vector<Event> EventManager::getEvents() const {
  std::lock_guard<std::mutex> lock(_mutex);
  return _events;
}

void EventManager::update() {
  std::vector<Event> newEvents;

  if (!_api.fetchEvents(newEvents)) {
    Serial.println("Failed to fetch events from Calendar API.");
    return;
  }

  std::lock_guard<std::mutex> lock(_mutex);
  _events = std::move(newEvents);

  Serial.print("Events updated. Total events fetched: ");
  Serial.println(_events.size());
}

void EventManager::startTask() {
  if (_taskHandle != nullptr) return;  // Prevent creating duplicate tasks

  // Start the fetch loop in a separate task
  _isRunning = true;
  xTaskCreatePinnedToCore(
      [](void* param) {
        EventManager* manager = static_cast<EventManager*>(param);
        manager->fetchLoop();
      },
      "EventFetchTask",  // Task name
      8192,              // Stack size
      this,              // Pass the object instance
      1,                 // Task priority
      &_taskHandle,      // Task handle
      0                  // Core ID
  );
}

void EventManager::stopTask() {
  if (_taskHandle != nullptr) {
    _isRunning = false;  // Signals fetchLoop to exit its loop and self-delete
  }
}

void EventManager::fetchLoop() {
  while (_isRunning) {
    update();
    vTaskDelay(pdMS_TO_TICKS(_refreshIntervalMs));
  }

  _taskHandle = nullptr;
  vTaskDelete(NULL);  // Delete current task context
}