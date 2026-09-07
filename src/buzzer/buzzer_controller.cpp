#include "buzzer_controller.h"

#include "config.h"

BuzzerController::BuzzerController()
    : _queue(nullptr), _taskHandle(nullptr), _isRunning(false) {}

void BuzzerController::begin() {
  pinMode(PIN_BUZZER, OUTPUT);
  noTone(PIN_BUZZER);

  _queue = xQueueCreate(5, sizeof(BuzzerPreset));

  if (_queue == nullptr) {
    Serial.println("Failed to create buzzer queue.");
    return;
  }

  _isRunning = true;

  xTaskCreatePinnedToCore(
      [](void* param) {
        auto* controller = static_cast<BuzzerController*>(param);

        controller->update();
      },
      "BuzzerTask", 4096, this, 1, &_taskHandle, 0);
}

void BuzzerController::play(BuzzerPreset preset) {
  if (_queue == nullptr) return;

  xQueueSend(_queue, &preset, 0);
}

void BuzzerController::update() {
  while (_isRunning) {
    BuzzerPreset preset;

    // Sleep until a preset is requested.
    if (xQueueReceive(_queue, &preset, portMAX_DELAY) == pdTRUE) {
      const auto& tones = BuzzerPresets.at(preset);

      for (const auto& buzzerTone : tones) {
        if (!_isRunning) {
          break;
        }

        playTone(buzzerTone);
      }

      noTone(PIN_BUZZER);
    }
  }

  noTone(PIN_BUZZER);

  _taskHandle = nullptr;
  vTaskDelete(nullptr);
}

void BuzzerController::playTone(const BuzzerTone& buzzerTone) {
  if (buzzerTone.frequency == 0) {
    noTone(PIN_BUZZER);
  } else {
    ::tone(PIN_BUZZER, buzzerTone.frequency);
  }

  vTaskDelay(pdMS_TO_TICKS(buzzerTone.durationMs));
}

void BuzzerController::stop() { noTone(PIN_BUZZER); }