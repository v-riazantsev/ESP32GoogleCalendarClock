#include "led_controller.h"

#include <cmath>

#include "config.h"
#include "time_manager.h"

LedController::LedController(EventManager& eventManager,
                             TimeManager& timeManager,
                             AlarmManager& alarmManager)
    : _eventManager(eventManager),
      _timeManager(timeManager),
      _alarmManager(alarmManager),
      MAX_LEDS(NUM_PIXELS),
      _taskHandle(nullptr),
      _lastRenderMs(0),
      _isRunning(false) {
  _buf = new CRGB[MAX_LEDS];

  FastLED.addLeds<WS2812B, PIN_WS2812B, GRB>(_buf, MAX_LEDS);

  // Non-linear gamma correction
  FastLED.setCorrection(TypicalLEDStrip);

  // Enables temporal dithering
  FastLED.setDither(BINARY_DITHER);

  FastLED.setBrightness(LED_DEFAULT_BRIGHTNESS);

  clear();
}

void LedController::startTask() {
  if (_taskHandle != nullptr) return;

  _isRunning = true;

  xTaskCreatePinnedToCore(
      [](void* param) {
        auto* controller = static_cast<LedController*>(param);
        controller->update();
      },
      "LedUpdateTask", 8192, this, 1, &_taskHandle, 1);
}

void LedController::update() {
  uint8_t animationFrame = 0;

  while (_isRunning) {
    clear();

    switch (_mode) {
      case LedMode::Events: {
        std::string blinkingEventId =
            _alarmManager.getUnacknowledgedActiveEventId();

        float animationProgress = (float)animationFrame / ANIMATION_LENGTH;
        fillEvents(_eventManager.getEvents(), blinkingEventId,
                   animationProgress, _timeManager.now(), FADE_DISTANCE);
        break;
      }
      case LedMode::None:
        break;
    }

    {
      std::lock_guard<std::mutex> lock(_mutex);
      uint8_t scaledBrightness = dim8_raw(_brightness);
      nscale8(_buf, MAX_LEDS, scaledBrightness);
    }

    animationFrame++;
    if (animationFrame >= ANIMATION_LENGTH) animationFrame = 0;

    // FastLED.delay() calls .show() internally and is necessary for dithering
    // to work properly.
    FastLED.delay(32);
  }

  showBlank();

  _taskHandle = nullptr;
  vTaskDelete(nullptr);
}

void LedController::setBrightness(uint8_t brightness) {
  std::lock_guard<std::mutex> lock(_mutex);
  _brightness = brightness;
}

void LedController::showBlank() {
  clear();
  FastLED.show();
}

void LedController::clear() { fill_solid(_buf, MAX_LEDS, CRGB::Black); }

template <typename Func>
void LedController::forEachLedInRange(float startPct, float endPct, Func&& fn) {
  if (endPct == startPct) return;

  float startPos = startPct * MAX_LEDS;
  float endPos = endPct * MAX_LEDS;

  float rangeLength = (startPos < endPos) ? (endPos - startPos)
                                          : (MAX_LEDS - startPos + endPos);

  uint16_t startLed = static_cast<uint16_t>(startPos);
  uint16_t endLed = static_cast<uint16_t>(std::ceil(endPos));

  for (uint16_t i = startLed, j = 0; i != endLed;
       i = (i >= MAX_LEDS) ? 0 : i + 1, j++) {
    float coveragePct = 1.0f;
    if (i == startLed) coveragePct -= (startPos - startLed);
    if (i == endLed - 1) coveragePct -= ((endLed - 1) + 1.0f - endPos);

    coveragePct = std::max(0.0f, coveragePct);
    float progress =
        (rangeLength > 0.0f) ? static_cast<float>(j) / rangeLength : 0.0f;

    fn(i, coveragePct, progress);
  }
}

void LedController::addRangePct(float startPct, float endPct, CRGB paintColor) {
  forEachLedInRange(startPct, endPct,
                    [this, paintColor](uint16_t i, float coveragePct, float) {
                      CRGB scaledPaint = paintColor;
                      scaledPaint.nscale8(coveragePct * 255.0f);

                      _buf[i].r = qadd8(_buf[i].r, scaledPaint.r);
                      _buf[i].g = qadd8(_buf[i].g, scaledPaint.g);
                      _buf[i].b = qadd8(_buf[i].b, scaledPaint.b);
                    });
}

void LedController::fadeRangePct(float startPct, float endPct, CRGB fadeColor,
                                 bool inverse) {
  forEachLedInRange(startPct, endPct,
                    [this, fadeColor, inverse](uint16_t i, float coveragePct,
                                               float progress) {
                      float fadeValue = 1.0f - (progress * std::sqrt(progress));

                      if (inverse) fadeValue = 1.0f - fadeValue;

                      uint8_t blendAmount = static_cast<uint8_t>(constrain(
                          fadeValue * coveragePct * 255.0f, 0.0f, 255.0f));

                      nblend(_buf[i], fadeColor, blendAmount);
                    });
}

void LedController::addTimestampRange(time_t startTimestamp,
                                      time_t endTimestamp,
                                      time_t currentTimestamp, CRGB paintColor,
                                      uint32_t fadeDistance,
                                      int32_t displayRangeOffsetStart,
                                      int32_t displayRangeOffsetEnd) {
  int32_t startOffset = startTimestamp - currentTimestamp;
  int32_t endOffset = endTimestamp - currentTimestamp;

  if (endOffset < -(int32_t)fadeDistance &&
      startOffset < -(int32_t)fadeDistance)
    return;

  if (startOffset > 12 * 3600 - (int32_t)fadeDistance) return;

  // Clamp offset values to the display range.
  startOffset =
      constrain(startOffset, displayRangeOffsetStart, displayRangeOffsetEnd);
  endOffset =
      constrain(endOffset, displayRangeOffsetStart, displayRangeOffsetEnd);

  float startPct = _timeManager.clock12hPct(currentTimestamp + startOffset);
  float endPct = _timeManager.clock12hPct(currentTimestamp + endOffset);

  addRangePct(startPct, endPct, paintColor);
}

void LedController::fillEvents(std::shared_ptr<const std::vector<Event>> events,
                               const std::string& blinkingEventId,
                               float animationProgress, time_t currentTimestamp,
                               uint32_t fadeDistance) {
  int32_t displayRangeOffsetStart =
      -(int32_t)fadeDistance + (12 * 3600 / MAX_LEDS);
  int32_t displayRangeOffsetEnd = 12 * 3600 - (int32_t)fadeDistance;

  for (const Event& event : *events) {
    CRGB eventColor = CRGB(event.color);

    if (blinkingEventId == event.id) {
      float brightness = std::cos(animationProgress * PI * 2.0f) * 0.5f + 0.5f;

      eventColor.nscale8_video(static_cast<uint8_t>(brightness * 255.0f));
    }

    addTimestampRange(event.startTimestamp, event.endTimestamp,
                      currentTimestamp, eventColor, fadeDistance,
                      displayRangeOffsetStart, displayRangeOffsetEnd);
  }

  if (fadeDistance != 0 && !events->empty()) {
    float fadeStartPct =
        _timeManager.clock12hPct(currentTimestamp - fadeDistance);
    float fadeEndPct = _timeManager.clock12hPct(currentTimestamp);

    fadeRangePct(fadeStartPct, fadeEndPct, CRGB::Black);
  }
}