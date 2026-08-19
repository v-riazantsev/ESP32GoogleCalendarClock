#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WiFiManager {
 public:
  // Create the Wi-Fi manager with network credentials.
  WiFiManager(const char* ssid, const char* password);

  // Start the Wi-Fi connection.
  void begin();

  // Keep the Wi-Fi connection alive and reconnect if needed.
  void update();

  // Return true when the ESP32 is connected to Wi-Fi.
  bool isConnected() const;

 private:
  // Wi-Fi network name.
  const char* _ssid;

  // Wi-Fi network password.
  const char* _password;
};