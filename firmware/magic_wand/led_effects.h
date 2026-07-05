// led_effects.h — kontrol LED RGB untuk feedback visual tiap gesture
#pragma once
#include <Arduino.h>
#include "gesture_classifier.h"

class LedEffects {
 public:
  void begin();
  void off();
  void setColor(uint8_t r, uint8_t g, uint8_t b);
  // Jalankan animasi blocking singkat sesuai gesture yang terdeteksi
  void playForGesture(Gesture g);
  // Indikator status: idle breathing, error blink, dst (dipanggil non-blocking dari loop)
  void idleBreath();
  void blinkError(uint8_t times = 3);
};
