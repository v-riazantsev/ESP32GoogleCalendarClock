#include "alarm_manager.h"

#include <Arduino.h>
#include <config.h>

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

  touchAttachInterruptArg(
      TOUCH_PIN,
      [](void* param) {
        auto* manager = static_cast<AlarmManager*>(param);

        if (manager) {
          manager->_acknowledgeRequested = true;
        }
      },
      this, TOUCH_THRESHOLD);
}

void AlarmManager::stop() { _isRunning = false; }

void AlarmManager::update() {
  while (_isRunning) {
    if (_acknowledgeRequested) {
      _acknowledgeRequested = false;
      acknowledge();
    }

    // Check for active events and update _activeEventId accordingly
    auto events = _eventManager.getEvents();

    time_t now = _timeManager.now();

    std::string recentEventId;
    time_t smallestOffset = std::numeric_limits<time_t>::max();

    // Find the most recent active event
    for (const Event& event : *events) {
      if (now >= event.startTimestamp && now <= event.endTimestamp &&
          now - event.startTimestamp < smallestOffset) {
        recentEventId = event.id;
        smallestOffset = now - event.startTimestamp;
      }
    }

    if (recentEventId.empty()) {
      _activeEventId.clear();
      _acknowledged = false;  // Reset acknowledgment when no event is active
    } else if (recentEventId != _activeEventId) {
      _activeEventId = recentEventId;
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
  if (_acknowledged) return {};

  return _activeEventId;
}