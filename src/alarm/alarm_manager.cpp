#include "alarm_manager.h"

#include <Arduino.h>

#include "event_manager.h"
#include "time_manager.h"

void AlarmManager::begin() {
  if (_taskHandle != nullptr) return;

  _isRunning = true;

  xTaskCreatePinnedToCore(
      [](void* param) {
        auto* manager = static_cast<AlarmManager*>(param);
        manager->update();
      },
      "AlarmUpdateTask", 8192, this, 1, &_taskHandle, 0);
}

void AlarmManager::stop() { _isRunning = false; }

void AlarmManager::update() {
  while (_isRunning) {
    // Check for active events and update _activeEventId accordingly
    auto events = _eventManager.getEvents();

    time_t now = _timeManager.now();

    const Event* recentEvent = nullptr;
    time_t smallestOffset = std::numeric_limits<time_t>::max();

    // Find the most recent active event
    for (const Event& event : *events) {
      if (now >= event.startTimestamp && now <= event.endTimestamp &&
          now - event.startTimestamp < smallestOffset) {
        recentEvent = &event;
        smallestOffset = now - event.startTimestamp;
      }
    }

    if (!recentEvent) {
      _activeEventId.clear();
      _acknowledged = false;  // Reset acknowledgment when no event is active
    } else if (recentEvent && _activeEventId != recentEvent->id) {
      _activeEventId = recentEvent->id;
      _acknowledged =
          false;  // Reset acknowledgment when a new event becomes active
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay for 1 second
  }

  _taskHandle = nullptr;
  vTaskDelete(nullptr);
}

void AlarmManager::acknowledge() {
  if (!_activeEventId.empty()) _acknowledged = true;
}

std::string AlarmManager::getUnacknowledgedActiveEventId() const {
  if (!_acknowledged) return _activeEventId;

  return nullptr;
}