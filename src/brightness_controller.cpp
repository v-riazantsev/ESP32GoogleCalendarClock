#include "brightness_controller.h"

#include "config.h"

BrightnessController::BrightnessController(LedController& ledController,
                                           TimeManager& timeManager)
    : _ledController(ledController), _timeManager(timeManager) {}

void BrightnessController::startTask() {
  if (_taskHandle != nullptr) return;

  _isRunning = true;

  xTaskCreatePinnedToCore(
      [](void* param) {
        auto* controller = static_cast<BrightnessController*>(param);

        controller->update();
      },
      "BrightnessUpdateTask", 4096, this, 1, &_taskHandle, 1);
}

void BrightnessController::stopTask() {
  if (_taskHandle == nullptr) return;

  _isRunning = false;

  vTaskDelete(_taskHandle);
  _taskHandle = nullptr;
}

void BrightnessController::update() {
  uint8_t brightness = DAY_BRIGHTNESS;
  while (_isRunning) {
    // If the time is not synchronized, set brightness to default and wait for
    // synchronization
    if (!_timeManager.synchronized()) {
      _ledController.setBrightness(DAY_BRIGHTNESS);  // Default brightness
      vTaskDelay(pdMS_TO_TICKS(1000));  // Wait for time synchronization
      continue;
    }

    float currentHour = _timeManager.nowHour();
    uint8_t newBrightness = DAY_BRIGHTNESS;

    // Determine the new brightness based on the current hour
    if (currentHour >= NIGHT_START_HOUR - NIGH_TRANSITION_DURATION &&
        currentHour < NIGHT_START_HOUR) {
      // Transition to night
      float t = (currentHour - (NIGHT_START_HOUR - NIGH_TRANSITION_DURATION)) /
                NIGH_TRANSITION_DURATION;
      newBrightness = static_cast<uint8_t>(
          MathUtils::lerp(DAY_BRIGHTNESS, NIGHT_BRIGHTNESS, t));
    } else if (currentHour > NIGHT_END_HOUR &&
               currentHour <= NIGHT_END_HOUR + NIGH_TRANSITION_DURATION) {
      // Transition to day
      float t = (currentHour - NIGHT_END_HOUR) / NIGH_TRANSITION_DURATION;
      newBrightness = static_cast<uint8_t>(
          MathUtils::lerp(NIGHT_BRIGHTNESS, DAY_BRIGHTNESS, t));
    } else if (currentHour >= NIGHT_START_HOUR ||
               currentHour < NIGHT_END_HOUR) {
      // Night time
      newBrightness = NIGHT_BRIGHTNESS;
    }

    Serial.print("Current Hour: ");
    Serial.print(currentHour);
    Serial.print(", New Brightness: ");
    Serial.println(newBrightness);

    // Update brightness if it has changed
    if (newBrightness != brightness)
      _ledController.setBrightness(newBrightness);

    brightness = newBrightness;
    vTaskDelay(pdMS_TO_TICKS(60000));  // Check every minute
  }

  _taskHandle = nullptr;
  vTaskDelete(nullptr);
}
