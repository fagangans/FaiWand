// oled_display.cpp
// Library: "Adafruit SSD1306" + "Adafruit GFX Library" (install lewat Library Manager).
#include "oled_display.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

static Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
static bool oledReady = false;

bool OledDisplay::begin() {
#if !OLED_ENABLED
  return false;
#else
  // Bus I2C (Wire.begin) sudah di-init oleh Mpu6050Sensor::begin() sebelumnya,
  // jadi di sini cukup langsung inisialisasi controller SSD1306.
  oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR);
  if (!oledReady) return false;

  display.setTextColor(SSD1306_WHITE);
  showMessage("AI Magic Wand", "Siap...");
  return true;
#endif
}

// Word-wrap sederhana untuk font default (ukuran 1 -> ~21 karakter per baris di lebar 128px)
static void printWrapped(const String &text, uint8_t maxCharsPerLine) {
  int start = 0;
  int16_t y = 0;
  while (start < (int)text.length() && y < OLED_HEIGHT) {
    int end = min((int)text.length(), start + (int)maxCharsPerLine);
    if (end < (int)text.length()) {
      int lastSpace = text.lastIndexOf(' ', end);
      if (lastSpace > start) end = lastSpace;
    }
    display.setCursor(0, y);
    display.println(text.substring(start, end));
    start = end + 1;
    y += 10;
  }
}

void OledDisplay::showIdle() {
  if (!oledReady) return;
  showMessage("AI Magic Wand", "Menunggu gestur...");
}

void OledDisplay::showGesture(const char *gestureName) {
  if (!oledReady) return;
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println(gestureName);
  display.display();
}

void OledDisplay::showAiResponse(const char *text) {
  if (!oledReady) return;
  display.clearDisplay();
  display.setTextSize(1);
  printWrapped(String(text), 21);
  display.display();
}

void OledDisplay::showMessage(const char *line1, const char *line2) {
  if (!oledReady) return;
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(line1);
  if (line2 && strlen(line2) > 0) {
    display.setCursor(0, 12);
    display.println(line2);
  }
  display.display();
}
