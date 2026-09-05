#pragma once
#define FASTLED_INTERNAL  // Suppress FastLED build pragma banners
#include <Arduino.h>
#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP32_RAW_PIN_DRIVE 1
#include <FastLED.h>

#include <map>
#include <vector>

#include "event.h"
#include "event_manager.h"
#include "math_utils.h"
#include "time_manager.h"

enum class LedMode { None, Events };

class LedController {
 public:
  uint16_t MAX_LEDS;
  LedController(EventManager& eventManager, TimeManager& timeManager);

  void setBrightness(uint8_t brightness);
  void startTask();
  void switchMode(LedMode mode) {
    _mode = mode;
    _lastRenderMs = 0;
  };

 private:
  LedMode _mode = LedMode::None;
  EventManager& _eventManager;
  TimeManager& _timeManager;
  CRGB* _buf;
  bool _isRunning;
  TaskHandle_t _taskHandle;
  uint32_t _lastRenderMs;
  uint8_t _brightness = 255;
  mutable std::mutex _mutex;

  std::map<LedMode, uint32_t> refreshIntervals = {{LedMode::Events, 1000},
                                                  {LedMode::None, 60000}};

  void clear();  // clears framebuffer
  void update();
  void addRangePct(float startPct, float endPct, CRGB paintColor);  // 0..100 %
  void fadeRangePct(float startPct, float endPct, CRGB fadeColor,
                    bool inverse = false);
  void fillEvents(const std::vector<Event>& events, time_t currentTimestamp,
                  uint32_t fadeDistance);
  template <typename Func>
  void forEachLedInRange(float startPct, float endPct,
                         Func&& callback);  // 0..100 %

  void showBlank();
  void show();
};