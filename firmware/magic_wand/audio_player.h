// audio_player.h — streaming & playback audio dari server AI lokal via I2S
// Library: "ESP32-audioI2S" by schreibfaul1 (install lewat Library Manager,
// cari "ESP32-audioI2S"). Mendukung streaming MP3 langsung dari URL HTTP.
#pragma once
#include <Arduino.h>

class AudioPlayer {
 public:
  bool begin();
  // Mulai streaming & mainkan file audio dari URL (non-blocking: audio jalan
  // di background, panggil loop() terus dari main loop sampai selesai).
  bool playUrl(const String &url);
  // WAJIB dipanggil tiap iterasi loop() utama selama audio mungkin sedang main,
  // supaya library sempat memproses buffer streaming.
  void loop();
  bool isPlaying();
};
