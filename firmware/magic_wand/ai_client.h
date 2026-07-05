// ai_client.h — kirim event gesture ke AI lokal (speech-to-speech/speech-to-text)
// via HTTP POST JSON di jaringan lokal (LAN). Server balas teks (untuk OLED)
// DAN url audio hasil TTS (untuk distreaming & dimainkan lewat speaker ESP32).
#pragma once
#include <Arduino.h>

struct AiReply {
  String text;      // teks singkat untuk ditampilkan di OLED
  String audioUrl;   // url absolut ke file mp3 hasil TTS server, kosong kalau tidak ada
};

class AiClient {
 public:
  // Hubungkan WiFi. Return false kalau gagal dalam timeoutMs.
  bool connectWifi(uint32_t timeoutMs = 15000);
  void disconnectWifi();
  bool isWifiConnected();

  // Kirim nama gesture ke AI lokal, dapatkan balasan (teks + url audio).
  // Return false kalau request gagal (WiFi mati, server tidak terjangkau, dsb) -> caller
  // boleh fallback tampilkan pesan offline di OLED.
  bool sendGesture(const char *gestureName, AiReply &outReply);

  // Kirim rekaman suara (WAV 16kHz/16-bit mono) ke server AI lokal untuk
  // speech-to-text -> AI jawab sesuai isi rekaman (bukan prompt tetap gesture).
  bool sendVoice(const uint8_t *wavData, size_t wavSize, AiReply &outReply);
};
