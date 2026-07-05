// config.h — pengaturan umum proyek AI Magic Wand
// Ubah nilai di sini untuk menyesuaikan hardware/perilaku tanpa menyentuh logika utama.
#pragma once

// ---------- Pin mapping (ESP32 DevKit V1) ----------
#define PIN_I2C_SDA        21
#define PIN_I2C_SCL        22
#define PIN_MPU_INT        27   // pin interrupt MPU6050 (wake-on-motion dari deep sleep)
#define PIN_LED_R          25
#define PIN_LED_G          26
#define PIN_LED_B          33
#define PIN_BUTTON         0    // tombol BOOT bawaan, dipakai untuk push-to-talk (tahan+bicara+lepas)

// LED aktif LOW jika pakai modul RGB common-anode. Set false jika common-cathode.
#define LED_ACTIVE_LOW     false

// ---------- OLED (SSD1306 0.96" I2C) ----------
// Pakai bus I2C yang sama dengan MPU6050 (SDA=21, SCL=22) — SSD1306 alamat default 0x3C,
// MPU6050 alamat default 0x68, jadi tidak bentrok di 1 bus.
#define OLED_ENABLED        true
#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define OLED_I2C_ADDR       0x3C

// ---------- Speaker (I2S, MAX98357A amp + speaker mini) ----------
// Suara AI di-generate & di-TTS di server (dinamis, beda tiap gesture),
// lalu ESP32 STREAMING file audio itu lewat WiFi dan mainkan langsung via
// I2S -- ESP32 sendiri tidak generate suara apapun.
#define SPEAKER_ENABLED     true
#define PIN_I2S_BCLK        14
#define PIN_I2S_LRC         32
#define PIN_I2S_DOUT        13
#define SPEAKER_VOLUME      12   // skala 0-21 (library ESP32-audioI2S)

// ---------- Mikrofon (I2S, INMP441) + push-to-talk ----------
// Tahan PIN_BUTTON (tombol BOOT) sambil bicara, lepas untuk kirim. Audio
// direkam ke RAM (WAV 16kHz/16-bit mono), dikirim ke server AI lokal untuk
// speech-to-text, lalu server jawab pakai teks hasil transkrip itu -- BUKAN
// lagi prompt tetap per gesture. ESP32 sendiri tidak melakukan STT apapun.
#define MIC_ENABLED         true
#define PIN_MIC_SCK         4     // I2S clock (BCLK) mikrofon
#define PIN_MIC_WS          5     // I2S word select (LRC) mikrofon
#define PIN_MIC_SD          18    // I2S data OUT dari mic ke ESP32
#define MIC_SAMPLE_RATE_HZ  16000 // 16kHz cukup untuk speech-to-text, hemat RAM/bandwidth
#define MIC_MAX_RECORD_MS   3000  // batas rekam maksimal (ms) -- ESP32 tanpa PSRAM RAM-nya
                                  // terbatas, 3 detik @16kHz mono ~= 96KB, cukup aman
#define MIC_MIN_RECORD_MS   400   // rekaman lebih pendek dari ini dianggap tidak sengaja/noise

// ---------- Sampling gesture ----------
#define SAMPLE_RATE_HZ         100     // frekuensi sampling accel/gyro
#define GESTURE_WINDOW_MS      1000    // panjang jendela sampel per gesture (harus sama dgn training Edge Impulse)
#define GESTURE_TRIGGER_THRESHOLD_G   1.5f // percepatan (dalam g) yang memicu mulai menangkap gesture

// ---------- Deep sleep / power ----------
#define IDLE_TIMEOUT_MS         30000  // masuk deep sleep jika tak ada gerakan selama ini (ms)
#define DEEP_SLEEP_ENABLED      true

// ---------- WiFi & AI lokal ----------
// Isi kredensial asli di secrets.h (jangan commit secrets.h ke git publik)
#include "secrets.h"

// AI lokal (speech-to-speech / speech-to-text) diakses lewat HTTP POST JSON di jaringan lokal.
// Alamat server diisi di secrets.h lewat LOCAL_AI_HOST — boleh hostname mDNS ("namaserver.local")
// ATAU IP statis ("192.168.1.50"), dua-duanya jalan tanpa ubah kode di sini.
#define LOCAL_AI_PORT       5000          // sesuaikan dengan port server AI lokal kamu
#define LOCAL_AI_PATH       "/gesture"    // sesuaikan dengan endpoint API server AI lokal kamu
#define LOCAL_AI_USE_HTTPS  false         // AI lokal biasanya HTTP biasa (tanpa TLS) di LAN

// ---------- Debug ----------
#define SERIAL_BAUD      115200
#define DEBUG_PRINT      true
