#include <Arduino.h>
#include <HTTPClient.h>

#include "alarm_manager.h"
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
AlarmManager alarmManager(eventManager, timeManager);
LedController strip(eventManager, timeManager, alarmManager);
BrightnessController brightnessController(strip, timeManager);
WiFiManager wifi(WIFI_SSID, WIFI_PASSWORD);

void setup() {
  Serial.begin(115200);
  wifi.begin();
  timeManager.begin();
  eventManager.startTask();
  alarmManager.begin();
  strip.switchMode(LedMode::Events);
  strip.startTask();
  brightnessController.startTask();
}

void loop() { delay(60000); }
