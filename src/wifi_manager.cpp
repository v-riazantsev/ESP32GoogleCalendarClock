#include "wifi_manager.h"

WiFiManager::WiFiManager(const char* ssid, const char* password)
    : _ssid(ssid), _password(password) {}

void WiFiManager::begin() {
  // Start connecting to the configured Wi-Fi network.
  WiFi.begin(_ssid, _password);
}

void WiFiManager::update() {
  // Reconnect if the Wi-Fi connection was lost.
  if (!isConnected()) {
    WiFi.reconnect();
  }
}

bool WiFiManager::isConnected() const {
  // Return the current Wi-Fi connection state.
  return WiFi.status() == WL_CONNECTED;
}