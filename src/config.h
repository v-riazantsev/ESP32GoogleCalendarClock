#pragma once

#define PIN_WS2812B 32
#define NUM_PIXELS 44

#define TIMEZONE "CET-1CEST,M3.5.0,M10.5.0"

#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "time.nist.gov"
#define NTP_SERVER_3 "time.google.com"

#define API_CALL_TIMEOUT_MS 30000

#define NIGHT_START_HOUR 23.00f
#define NIGHT_END_HOUR 6.0f
#define NIGHT_TRANSITION_DURATION 0.5f
#define NIGHT_BRIGHTNESS 100
#define DAY_BRIGHTNESS 255
#define BRIGHTNESS_CUTOFF 100
#define LED_DEFAULT_BRIGHTNESS 5