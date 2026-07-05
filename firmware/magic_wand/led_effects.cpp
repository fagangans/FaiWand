// led_effects.cpp
#include "led_effects.h"
#include "config.h"

static inline uint8_t applyPolarity(uint8_t v) {
  return LED_ACTIVE_LOW ? (255 - v) : v;
}

void LedEffects::begin() {
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  off();
}

void LedEffects::off() {
  setColor(0, 0, 0);
}

void LedEffects::setColor(uint8_t r, uint8_t g, uint8_t b) {
  analogWrite(PIN_LED_R, applyPolarity(r));
  analogWrite(PIN_LED_G, applyPolarity(g));
  analogWrite(PIN_LED_B, applyPolarity(b));
}

void LedEffects::playForGesture(Gesture g) {
  switch (g) {
    case Gesture::LUMOS:
      // Nyala kuning terang, tahan
      setColor(255, 200, 40);
      delay(1200);
      off();
      break;

    case Gesture::WAVE:
      // Kedip biru cepat 4x (trigger AI command)
      for (int i = 0; i < 4; i++) {
        setColor(40, 120, 255);
        delay(120);
        off();
        delay(120);
      }
      break;

    case Gesture::CIRCLE:
      // Efek "sweep" ungu -> pink
      for (int i = 0; i <= 255; i += 15) {
        setColor(150, 0, i);
        delay(15);
      }
      delay(300);
      off();
      break;

    default:
      blinkError(1);
      break;
  }
}

void LedEffects::idleBreath() {
  // Efek napas hijau redup, dipanggil berulang dari loop non-blocking
  static float phase = 0;
  phase += 0.05f;
  uint8_t v = (uint8_t)((sinf(phase) * 0.5f + 0.5f) * 40);
  setColor(0, v, 0);
}

void LedEffects::blinkError(uint8_t times) {
  for (uint8_t i = 0; i < times; i++) {
    setColor(255, 0, 0);
    delay(150);
    off();
    delay(150);
  }
}
