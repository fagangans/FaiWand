// power_manager.h — kelola deep sleep dan wake-on-motion
#pragma once
#include <Arduino.h>

class PowerManager {
 public:
  void begin();
  // Panggil tiap loop; reset idle timer setiap kali ada gerakan terdeteksi
  void notifyActivity();
  // Cek apakah sudah waktunya tidur (idle timeout tercapai)
  bool shouldSleep();
  // Masuk deep sleep, bangun via interrupt pin MPU_INT (motion) atau tombol BOOT
  void enterDeepSleep();
  // Panggil di awal setup() untuk tahu apakah bangun dari deep sleep dan sebabnya
  void logWakeReason();

 private:
  uint32_t lastActivityMs_ = 0;
};
