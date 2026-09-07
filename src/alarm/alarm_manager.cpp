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

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        if (manager && manager->_taskHandle != nullptr) {
          vTaskNotifyGiveFromISR(manager->_taskHandle,
                                 &xHigherPriorityTaskWoken);
        }

        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
       switch should be performed to ensure the interrupt returns directly to
       the highest priority task. */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      },
      this, TOUCH_THRESHOLD);
}

void AlarmManager::stop() { _isRunning = false; }

void AlarmManager::update() {
  while (_isRunning) {
    // Wait for touch OR wake up every second
    uint32_t notification = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000));

    if (!_isRunning) {
      break;
    }

    // Check for active events
    auto events = _eventManager.getEvents();

    time_t now = _timeManager.now();

    std::string recentEventId;
    time_t smallestOffset = std::numeric_limits<time_t>::max();

    for (const Event& event : *events) {
      if (now >= event.startTimestamp && now <= event.endTimestamp &&
          now - event.startTimestamp < smallestOffset) {
        recentEventId = event.id;
        smallestOffset = now - event.startTimestamp;
      }
    }

    if (recentEventId.empty()) {
      std::lock_guard<std::mutex> lock(_mutex);

      if (!_activeEventId.empty()) {
        _activeEventId.clear();
        _acknowledged = false;

        _buzzerController.play(BuzzerPreset::EventEnd);
      }

    } else {
      std::lock_guard<std::mutex> lock(_mutex);

      if (recentEventId != _activeEventId) {
        _activeEventId = recentEventId;
        _acknowledged = false;

        _buzzerController.play(BuzzerPreset::EventStart);
      }
    }

    // Touch happened?
    if (notification > 0) {
      acknowledge();
    }
  }

  _taskHandle = nullptr;
  vTaskDelete(nullptr);
}

void AlarmManager::acknowledge() {
  std::lock_guard<std::mutex> lock(_mutex);

  if (!_activeEventId.empty() && !_acknowledged) {
    _acknowledged = true;

    _buzzerController.play(BuzzerPreset::Acknowledge);
  }
}

std::string AlarmManager::getUnacknowledgedActiveEventId() const {
  std::lock_guard<std::mutex> lock(_mutex);

  if (_acknowledged) {
    return {};
  }

  return _activeEventId;
}