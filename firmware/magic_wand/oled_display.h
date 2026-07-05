// oled_display.h — tampilan teks di OLED SSD1306 0.96" I2C
// Dipakai untuk menampilkan nama gesture dan respons teks dari AI lokal (speech-to-text).
#pragma once
#include <Arduino.h>

class OledDisplay {
 public:
  // Return false kalau OLED tidak terdeteksi di bus I2C (tidak fatal — wand tetap jalan tanpa OLED).
  bool begin();
  void showIdle();
  void showGesture(const char *gestureName);
  void showAiResponse(const char *text);
  void showMessage(const char *line1, const char *line2 = "");
};
