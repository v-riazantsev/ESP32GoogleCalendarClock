#pragma once

#include <event_manager.h>
#include <time_manager.h>

#include <string>

class AlarmManager {
 public:
  AlarmManager(EventManager& eventManager, TimeManager& timeManager)
      : _eventManager(eventManager), _timeManager(timeManager) {}

  void begin();

  void update();

  void stop();

  void acknowledge();

  std::string getUnacknowledgedActiveEventId() const;

 private:
  EventManager& _eventManager;
  TimeManager& _timeManager;

  bool _isRunning;
  TaskHandle_t _taskHandle;
  std::string _activeEventId;
  bool _acknowledged = false;
};