#pragma once
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include <map>
#include <vector>

#include "event.h"
#include "event_manager.h"
#include "time_manager.h"

enum class LedMode { None, Events };

class LedController {
 public:
  uint16_t MAX_LEDS;
  LedController(uint16_t numPixels, uint8_t pin, EventManager& eventManager,
                TimeManager& timeManager);

  void setBrightness(uint8_t brightness);
  void startTask();
  void switchMode(LedMode mode) {
    _mode = mode;
    _lastRenderMs = 0;
  };

 private:
  Adafruit_NeoPixel _strip;
  LedMode _mode = LedMode::None;
  EventManager& _eventManager;
  TimeManager& _timeManager;
  uint32_t* _buf;
  bool _isRunning;
  TaskHandle_t _taskHandle;
  uint32_t _lastRenderMs;

  std::map<LedMode, uint32_t> refreshIntervals = {{LedMode::Events, 1000},
                                                  {LedMode::None, 60000}};

  void clear();    // clears framebuffer
  void refresh();  // refreshes the strip with the framebuffer
  void update();
  void addRangePct(float startPct, float endPct, uint32_t color);  // 0..100 %
  void fillEvents(const std::vector<Event>& events, time_t currentTimestamp,
                  uint32_t fadeDistance);
  float wrap(float val, float length);
  void showBlank() {
    clear();
    refresh();
  };
};
