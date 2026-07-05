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
#define PIN_BUTTON         0    // tombol BOOT bawaan, dipakai untuk mode training/manual wake

// LED aktif LOW jika pakai modul RGB common-anode. Set false jika common-cathode.
#define LED_ACTIVE_LOW     false

// ---------- Sampling gesture ----------
#define SAMPLE_RATE_HZ         100     // frekuensi sampling accel/gyro
#define GESTURE_WINDOW_MS      1000    // panjang jendela sampel per gesture (harus sama dgn training Edge Impulse)
#define GESTURE_TRIGGER_THRESHOLD_G   1.5f // percepatan (dalam g) yang memicu mulai menangkap gesture

// ---------- Deep sleep / power ----------
#define IDLE_TIMEOUT_MS         30000  // masuk deep sleep jika tak ada gerakan selama ini (ms)
#define DEEP_SLEEP_ENABLED      true

// ---------- WiFi & AI API ----------
// Isi kredensial asli di secrets.h (jangan commit secrets.h ke git publik)
#include "secrets.h"

// Pilih provider AI: "openai" (default) atau "grok"
#define AI_PROVIDER            "openai"

// Endpoint tiap provider (format request kompatibel gaya Chat Completions)
#define OPENAI_API_URL   "https://api.openai.com/v1/chat/completions"
#define OPENAI_MODEL     "gpt-4o-mini"

#define GROK_API_URL     "https://api.x.ai/v1/chat/completions"
#define GROK_MODEL       "grok-2-latest"

// ---------- Debug ----------
#define SERIAL_BAUD      115200
#define DEBUG_PRINT      true
