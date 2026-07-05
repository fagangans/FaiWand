// gesture_classifier.h — klasifikasi gesture dari buffer sampel sensor
//
// Dua mode:
//  1) USE_EDGE_IMPULSE_MODEL = 1 -> pakai model TFLite Micro hasil export Edge Impulse
//  2) USE_EDGE_IMPULSE_MODEL = 0 -> fallback threshold sederhana (buat bring-up sebelum training)
#pragma once
#include <Arduino.h>
#include "mpu6050_sensor.h"
#include "config.h"

// Set ke 1 setelah kamu meng-install library hasil export Edge Impulse
// (Project > Deployment > Arduino library, lalu Sketch > Include Library > Add .zip)
#define USE_EDGE_IMPULSE_MODEL 0

enum class Gesture {
  NONE,
  LUMOS,   // gesture 1: angkat + tahan (contoh)
  WAVE,    // gesture 2: kibas kiri-kanan
  CIRCLE,  // gesture 3: putar melingkar
  UNKNOWN
};

const char *gestureName(Gesture g);

// Jumlah sample per jendela gesture, harus konsisten dgn window Edge Impulse (ms) x sample rate (Hz)
#define GESTURE_MAX_SAMPLES ((SAMPLE_RATE_HZ * GESTURE_WINDOW_MS) / 1000)
#define GESTURE_AXES 6 // ax, ay, az, gx, gy, gz

class GestureClassifier {
 public:
  void begin();
  // Tambahkan satu sample ke buffer jendela gesture. Return true kalau buffer sudah penuh
  // dan siap diklasifikasi (classify() bisa dipanggil).
  bool addSample(const GyroSample &s);
  Gesture classify();
  void reset();

 private:
  float buffer_[GESTURE_MAX_SAMPLES * GESTURE_AXES];
  int sampleCount_ = 0;

  Gesture classifyThreshold();
#if USE_EDGE_IMPULSE_MODEL
  Gesture classifyEdgeImpulse();
#endif
};
