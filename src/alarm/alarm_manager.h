#pragma once

#include <Arduino.h>

#include <atomic>
#include <mutex>
#include <string>

#include "buzzer_controller.h"

class EventManager;
class TimeManager;

class AlarmManager {
 public:
  AlarmManager(EventManager& eventManager, TimeManager& timeManager,
               BuzzerController& buzzerController)
      : _eventManager(eventManager),
        _timeManager(timeManager),
        _buzzerController(buzzerController) {}

  void begin();
  void stop();

  void acknowledge();

  std::string getUnacknowledgedActiveEventId() const;

 private:
  void update();

  EventManager& _eventManager;
  TimeManager& _timeManager;
  BuzzerController& _buzzerController;

  TaskHandle_t _taskHandle;
  std::atomic<bool> _isRunning;

  mutable std::mutex _mutex;

  std::string _activeEventId;
  bool _acknowledged;
};