// ai_client.h — kirim event gesture ke AI lokal (speech-to-speech/speech-to-text)
// via HTTP POST JSON di jaringan lokal (LAN), dan terima respons teks untuk ditampilkan di OLED.
#pragma once
#include <Arduino.h>

class AiClient {
 public:
  // Hubungkan WiFi. Return false kalau gagal dalam timeoutMs.
  bool connectWifi(uint32_t timeoutMs = 15000);
  void disconnectWifi();
  bool isWifiConnected();

  // Kirim nama gesture ke AI lokal, dapatkan balasan teks untuk ditampilkan di OLED.
  // AI lokal yang menangani sendiri urusan speech-to-speech/TTS di sisi servernya;
  // ESP32 hanya perlu teks ringkas untuk tampilan.
  // Return false kalau request gagal (WiFi mati, server tidak terjangkau, dsb) -> caller
  // boleh fallback tampilkan pesan offline di OLED.
  bool sendGesture(const char *gestureName, String &outResponse);
};
