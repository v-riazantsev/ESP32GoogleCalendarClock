#include "event_manager.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "calendar_api.h"
#include "config.h"

EventManager::EventManager(CalendarApi& api)
    : _events(std::make_shared<const std::vector<Event>>()),
      _api(api),
      _taskHandle(nullptr),
      _isRunning(false) {}

std::shared_ptr<const std::vector<Event>> EventManager::getEvents() const {
  std::lock_guard<std::mutex> lock(_mutex);
  return _events;
}

void EventManager::update() {
  std::vector<Event> newEvents;
  while (_isRunning) {
    if (!_api.fetchEvents(newEvents)) {
      Serial.println("Failed to fetch events from Calendar API.");
      vTaskDelay(pdMS_TO_TICKS(1000));  // Wait before retrying
      continue;
    }

    auto newEventsPtr =
        std::make_shared<const std::vector<Event>>(std::move(newEvents));

    {
      std::lock_guard<std::mutex> lock(_mutex);
      _events = newEventsPtr;
    }

    Serial.print("Events updated. Total events fetched: ");
    Serial.println(newEventsPtr->size());
    vTaskDelay(pdMS_TO_TICKS(
        API_CALL_TIMEOUT_MS));  // Wait for the specified refresh interval
  }

  _taskHandle = nullptr;
  vTaskDelete(nullptr);  // Delete current task context
}

void EventManager::startTask() {
  if (_taskHandle != nullptr) return;  // Prevent creating duplicate tasks

  // Start the fetch loop in a separate task
  _isRunning = true;
  xTaskCreatePinnedToCore(
      [](void* param) {
        EventManager* manager = static_cast<EventManager*>(param);
        manager->update();
      },
      "EventFetchTask", 8192, this, 1, &_taskHandle, 0);
}

void EventManager::stopTask() {
  if (_taskHandle != nullptr) {
    _isRunning = false;  // Signals update to exit its loop and self-delete
  }
}