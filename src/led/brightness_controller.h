#pragma once

#include <Arduino.h>

#include "led_controller.h"

class BrightnessController {
 public:
  BrightnessController(LedController& ledController, TimeManager& timeManager);
  void startTask();
  void stopTask();

 private:
  LedController& _ledController;
  TimeManager& _timeManager;
  bool _isRunning;
  TaskHandle_t _taskHandle;
  void update();
};