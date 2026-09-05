#include "wifi_manager.h"

WiFiManager::WiFiManager(const char* ssid, const char* password)
    : _ssid(ssid), _password(password) {}

void WiFiManager::begin() {
  // Start connecting to the configured Wi-Fi network.
  WiFi.begin(_ssid, _password);

  if (!WiFi.waitForConnectResult()) {
    Serial.println("Wi-Fi connection failed. Retrying...");
    WiFi.disconnect();
    WiFi.begin(_ssid, _password);
  }

  if (_isRunning) return;  // Prevent creating duplicate tasks

  _isRunning = true;
  xTaskCreatePinnedToCore(
      [](void* param) {
        auto* manager = static_cast<WiFiManager*>(param);
        manager->update();
      },
      "WiFiReconnectTask", 4096, this, 1, nullptr, 0);
}

void WiFiManager::stop() {
  _isRunning = false;
  WiFi.disconnect(true);
}

void WiFiManager::update() {
  while (_isRunning) {
    // Reconnect if the Wi-Fi connection was lost.
    if (!isConnected()) WiFi.reconnect();

    vTaskDelay(pdMS_TO_TICKS(5000));  // Check every 5 seconds
  }

  _isRunning = false;
  vTaskDelete(nullptr);  // Delete current task context
}

bool WiFiManager::isConnected() const {
  // Return the current Wi-Fi connection state.
  return WiFi.status() == WL_CONNECTED;
}