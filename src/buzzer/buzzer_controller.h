#pragma once

#include <Arduino.h>

#include "buzzer_presets.h"

class BuzzerController {
 public:
  BuzzerController();

  void begin();
  void stop();

  void play(BuzzerPreset preset);

 private:
  void update();
  void playTone(const BuzzerTone& buzzerTone);

  QueueHandle_t _queue;
  TaskHandle_t _taskHandle;

  bool _isRunning;
};