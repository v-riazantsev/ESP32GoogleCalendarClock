#include <Arduino.h>
#include <HTTPClient.h>

#include "brightness_controller.h"
#include "calendar_api.h"
#include "event_manager.h"
#include "led_controller.h"
#include "secrets.h"
#include "time_manager.h"
#include "wifi_manager.h"

CalendarApi calendarApi;
TimeManager timeManager;
EventManager eventManager(calendarApi);
LedController strip(eventManager, timeManager);
BrightnessController brightnessController(strip, timeManager);
WiFiManager wifi(WIFI_SSID, WIFI_PASSWORD);

void setup() {
  Serial.begin(115200);
  wifi.begin();
  strip.switchMode(LedMode::Events);
  strip.startTask();
  timeManager.begin();
  brightnessController.startTask();
  eventManager.startTask();  // Start the event fetch loop
}

void loop() {
  wifi.update();  // Keep Wi-Fi connection alive

  delay(5000);
}
