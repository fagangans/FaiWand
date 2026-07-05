// mpu6050_sensor.h — wrapper baca accelerometer + gyro dari MPU6050
#pragma once
#include <Arduino.h>

struct GyroSample {
  float ax, ay, az; // g
  float gx, gy, gz; // deg/s
};

class Mpu6050Sensor {
 public:
  bool begin();
  // Baca satu sample. Return false jika gagal komunikasi I2C.
  bool read(GyroSample &out);
  // Magnitudo percepatan total (g), dipakai untuk deteksi trigger gesture.
  float accelMagnitude(const GyroSample &s) const;
  // Konfigurasi interrupt motion-detection di MPU6050 (untuk wake-on-motion saat deep sleep)
  void configureMotionInterrupt(uint8_t threshold = 20, uint8_t duration = 1);
};
