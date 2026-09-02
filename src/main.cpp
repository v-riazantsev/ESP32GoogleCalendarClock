#include <Arduino.h>
#include <HTTPClient.h>

#include "calendar_api.h"
#include "event_manager.h"
#include "led_controller.h"
#include "secrets.h"
#include "time_manager.h"
#include "wifi_manager.h"

#define PIN_WS2812B 32
#define NUM_PIXELS 44

CalendarApi calendarApi;
TimeManager timeManager;
EventManager eventManager(calendarApi, 5000);  // Refresh every 5 seconds
LedController strip(NUM_PIXELS, PIN_WS2812B, eventManager, timeManager);
WiFiManager wifi(WIFI_SSID, WIFI_PASSWORD);

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Smart Clock...");
  strip.setBrightness(10);
  strip.switchMode(LedMode::Events);
  strip.startTask();
  wifi.begin();
  timeManager.begin();
  eventManager.startTask();  // Start the event fetch loop
}

void loop() {
  wifi.update();

  String events = "Events: " + String(eventManager.getEvents().size());
  Serial.println(events);

  delay(5000);
}
