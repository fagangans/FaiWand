// mpu6050_sensor.cpp
// Menggunakan library "Adafruit MPU6050" + "Adafruit Unified Sensor" (install lewat Library Manager).
#include "mpu6050_sensor.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

static Adafruit_MPU6050 mpu;

bool Mpu6050Sensor::begin() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  if (!mpu.begin()) {
    return false;
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);
  return true;
}

bool Mpu6050Sensor::read(GyroSample &out) {
  sensors_event_t a, g, temp;
  if (!mpu.getEvent(&a, &g, &temp)) {
    return false;
  }
  // Adafruit lib mengembalikan accel dalam m/s^2 dan gyro dalam rad/s; konversi ke g dan deg/s
  const float G = 9.80665f;
  out.ax = a.acceleration.x / G;
  out.ay = a.acceleration.y / G;
  out.az = a.acceleration.z / G;
  out.gx = g.gyro.x * 180.0f / PI;
  out.gy = g.gyro.y * 180.0f / PI;
  out.gz = g.gyro.z * 180.0f / PI;
  return true;
}

float Mpu6050Sensor::accelMagnitude(const GyroSample &s) const {
  return sqrtf(s.ax * s.ax + s.ay * s.ay + s.az * s.az);
}

void Mpu6050Sensor::configureMotionInterrupt(uint8_t threshold, uint8_t duration) {
  // Adafruit_MPU6050 tidak expose register motion-detection langsung, jadi tulis manual via I2C.
  // Register MPU6050: 0x1F=MOT_THR, 0x20=MOT_DUR, 0x37=INT_PIN_CFG, 0x38=INT_ENABLE
  const uint8_t MPU_ADDR = 0x68;

  auto writeReg = [&](uint8_t reg, uint8_t val) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
  };

  writeReg(0x1F, threshold);      // MOT_THR
  writeReg(0x20, duration);       // MOT_DUR
  writeReg(0x37, 0x20);           // INT_PIN_CFG: latch, active high, push-pull
  writeReg(0x38, 0x40);           // INT_ENABLE: motion detection interrupt
}
