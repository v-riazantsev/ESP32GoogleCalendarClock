#include <Arduino.h>
#include <HTTPClient.h>

#include "calendar_api.h"
#include "event_manager.h"
#include "led_controller.h"
#include "secrets.h"
#include "wifi_manager.h"

#define PIN_WS2812B 32
#define NUM_PIXELS 44

CalendarApi calendarApi;
EventManager eventManager(calendarApi, 5000);  // Refresh every 5 seconds
LedController strip(NUM_PIXELS, PIN_WS2812B);
WiFiManager wifi(WIFI_SSID, WIFI_PASSWORD);

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Smart Clock...");
  strip.begin(3);  // brightness
  wifi.begin();
  eventManager.startTask();  // Start the event fetch loop
}

void loop() {
  wifi.update();

  strip.clear();

  if (wifi.isConnected()) {
    strip.addRangePct(0.0f, 0.5f, strip.Color(0, 255, 0));  // green

  } else {
    strip.addRangePct(0.5f, 1.0f, strip.Color(255, 0, 0));  // red
  }

  strip.show(1000, 0.0f);

  String events = "Events: " + String(eventManager.getEvents().size());
  Serial.println(events);

  delay(5000);
}
