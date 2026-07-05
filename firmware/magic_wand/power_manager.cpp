// power_manager.cpp
#include "power_manager.h"
#include "config.h"
#include <esp_sleep.h>

void PowerManager::begin() {
  lastActivityMs_ = millis();
}

void PowerManager::notifyActivity() {
  lastActivityMs_ = millis();
}

bool PowerManager::shouldSleep() {
  if (!DEEP_SLEEP_ENABLED) return false;
  return (millis() - lastActivityMs_) > IDLE_TIMEOUT_MS;
}

void PowerManager::enterDeepSleep() {
#if DEBUG_PRINT
  Serial.println("[PowerManager] Masuk deep sleep, tunggu gerakan atau tombol BOOT...");
  Serial.flush();
#endif

  // Bangun dari deep sleep saat pin interrupt MPU6050 (motion) jadi HIGH,
  // atau tombol BOOT (PIN_BUTTON) ditekan (active LOW -> pakai ext0 dgn level LOW jika perlu).
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_MPU_INT, 1); // wake saat HIGH (motion interrupt)

  // ext1 bisa dipakai untuk multi-pin wake jika mau tambah tombol; disederhanakan ke ext0 dulu.
  esp_deep_sleep_start();
  // Eksekusi tidak akan lanjut ke sini — ESP32 reset & setup() jalan lagi setelah bangun.
}

void PowerManager::logWakeReason() {
#if DEBUG_PRINT
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  switch (cause) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("[PowerManager] Bangun dari deep sleep: motion interrupt (MPU6050)");
      break;
    case ESP_SLEEP_WAKEUP_UNDEFINED:
    default:
      Serial.println("[PowerManager] Cold boot / reset normal");
      break;
  }
#endif
}
