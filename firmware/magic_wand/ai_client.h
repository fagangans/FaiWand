// ai_client.h — kirim event gesture ke AI (OpenAI/Grok) dan terima respons teks
#pragma once
#include <Arduino.h>

class AiClient {
 public:
  // Hubungkan WiFi. Return false kalau gagal dalam timeoutMs.
  bool connectWifi(uint32_t timeoutMs = 15000);
  void disconnectWifi();
  bool isWifiConnected();

  // Kirim nama gesture ke API, dapatkan balasan teks (mis. respons "spell" dari AI).
  // Return false kalau request gagal (WiFi mati, HTTP error, dsb) -> caller boleh fallback offline.
  bool sendGesture(const char *gestureName, String &outResponse);
};
