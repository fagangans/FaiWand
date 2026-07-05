// gesture_classifier.cpp
#include "gesture_classifier.h"

#if USE_EDGE_IMPULSE_MODEL
// Ganti "your_project_inferencing.h" dengan nama header yang dihasilkan Edge Impulse
// (formatnya: <nama_project>_inferencing.h), lihat docs/EDGE_IMPULSE_GUIDE.md
#include <your_project_inferencing.h>
#endif

const char *gestureName(Gesture g) {
  switch (g) {
    case Gesture::LUMOS: return "Lumos";
    case Gesture::WAVE: return "Wave";
    case Gesture::CIRCLE: return "Circle";
    case Gesture::UNKNOWN: return "Unknown";
    default: return "None";
  }
}

void GestureClassifier::begin() {
  reset();
}

void GestureClassifier::reset() {
  sampleCount_ = 0;
}

bool GestureClassifier::addSample(const GyroSample &s) {
  if (sampleCount_ >= GESTURE_MAX_SAMPLES) {
    return true; // buffer sudah penuh
  }
  int base = sampleCount_ * GESTURE_AXES;
  buffer_[base + 0] = s.ax;
  buffer_[base + 1] = s.ay;
  buffer_[base + 2] = s.az;
  buffer_[base + 3] = s.gx;
  buffer_[base + 4] = s.gy;
  buffer_[base + 5] = s.gz;
  sampleCount_++;
  return sampleCount_ >= GESTURE_MAX_SAMPLES;
}

Gesture GestureClassifier::classify() {
#if USE_EDGE_IMPULSE_MODEL
  return classifyEdgeImpulse();
#else
  return classifyThreshold();
#endif
}

#if USE_EDGE_IMPULSE_MODEL
Gesture GestureClassifier::classifyEdgeImpulse() {
  // Bungkus buffer_ jadi signal_t yang diminta EI SDK
  signal_t signal;
  numpy::signal_from_buffer(buffer_, GESTURE_MAX_SAMPLES * GESTURE_AXES, &signal);

  ei_impulse_result_t result = {0};
  EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false /* debug */);
  if (err != EI_IMPULSE_OK) {
    return Gesture::UNKNOWN;
  }

  // Ambil label dengan confidence tertinggi
  float bestScore = 0;
  const char *bestLabel = "";
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    if (result.classification[ix].value > bestScore) {
      bestScore = result.classification[ix].value;
      bestLabel = result.classification[ix].label;
    }
  }

  if (bestScore < 0.7f) return Gesture::UNKNOWN; // ambang confidence, sesuaikan sesuai kebutuhan

  if (strcmp(bestLabel, "lumos") == 0) return Gesture::LUMOS;
  if (strcmp(bestLabel, "wave") == 0) return Gesture::WAVE;
  if (strcmp(bestLabel, "circle") == 0) return Gesture::CIRCLE;
  return Gesture::UNKNOWN;
}
#endif

// Fallback sederhana tanpa model ML: heuristik berbasis pola sumbu accel/gyro.
// Ini HANYA untuk bring-up/testing awal sebelum model Edge Impulse siap —
// akurasinya jauh di bawah model ML asli, ganti begitu USE_EDGE_IMPULSE_MODEL = 1.
Gesture GestureClassifier::classifyThreshold() {
  if (sampleCount_ < GESTURE_MAX_SAMPLES) return Gesture::NONE;

  float azSum = 0, gzAbsSum = 0, gyAbsSum = 0;
  float azMax = -1000, azMin = 1000;

  for (int i = 0; i < sampleCount_; i++) {
    int base = i * GESTURE_AXES;
    float az = buffer_[base + 2];
    float gy = buffer_[base + 4];
    float gz = buffer_[base + 5];

    azSum += az;
    azMax = max(azMax, az);
    azMin = min(azMin, az);
    gyAbsSum += fabsf(gy);
    gzAbsSum += fabsf(gz);
  }

  float azRange = azMax - azMin;

  // "Lumos": angkat lalu tahan relatif stabil -> az naik tinggi, rotasi rendah
  if (azRange > 1.2f && gzAbsSum < 200.0f) {
    return Gesture::LUMOS;
  }
  // "Wave": rotasi kiri-kanan cepat pada sumbu Z -> gz tinggi
  if (gzAbsSum > 400.0f) {
    return Gesture::WAVE;
  }
  // "Circle": rotasi gabungan Y dan Z -> keduanya tinggi
  if (gyAbsSum > 300.0f && gzAbsSum > 300.0f) {
    return Gesture::CIRCLE;
  }

  return Gesture::UNKNOWN;
}
