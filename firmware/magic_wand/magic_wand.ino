// magic_wand.ino — AI Magic Wand firmware utama
//
// Alur kerja:
//  1. Idle: baca sensor terus-menerus, tunggu percepatan melewati threshold (trigger gesture).
//  2. Saat trigger, tangkap jendela sampel (GESTURE_WINDOW_MS) untuk diklasifikasi.
//  3. Klasifikasi gesture (model Edge Impulse atau fallback threshold, lihat gesture_classifier.h).
//  4. Nyalakan LED sesuai gesture.
//  5. Jika gesture == WAVE (atau gesture lain yang kamu mau), kirim ke AI API dan tampilkan respons.
//  6. Jika idle terlalu lama, masuk deep sleep; bangun otomatis saat ada motion (interrupt MPU6050).
//
// Sebelum upload, siapkan:
//  - Copy secrets.h.example -> secrets.h dan isi WiFi + alamat server AI lokal
//  - Install library: Adafruit MPU6050, Adafruit Unified Sensor, Adafruit SSD1306,
//    Adafruit GFX Library, ArduinoJson
//  - (Opsional) install library hasil export Edge Impulse, lalu set USE_EDGE_IMPULSE_MODEL=1
//    di gesture_classifier.h

#include "config.h"
#include "mpu6050_sensor.h"
#include "gesture_classifier.h"
#include "led_effects.h"
#include "oled_display.h"
#include "ai_client.h"
#include "audio_player.h"
#include "power_manager.h"

Mpu6050Sensor sensor;
GestureClassifier classifier;
LedEffects leds;
OledDisplay oled;
AiClient ai;
AudioPlayer speaker;
PowerManager power;

enum class State { IDLE, CAPTURING, CLASSIFYING };
State state = State::IDLE;

uint32_t lastSampleMs = 0;
const uint32_t sampleIntervalMs = 1000 / SAMPLE_RATE_HZ;

// Gesture yang memicu pengiriman ke AI (bisa ditambah/dikurangi sesuai kebutuhan)
bool shouldTriggerAI(Gesture g) {
  return g == Gesture::WAVE;
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(200);
#if DEBUG_PRINT
  Serial.println("\n=== AI Magic Wand ===");
#endif

  power.logWakeReason();
  power.begin();

  leds.begin();

  if (!sensor.begin()) {
#if DEBUG_PRINT
    Serial.println("[FATAL] MPU6050 tidak terdeteksi. Cek wiring I2C.");
#endif
    leds.blinkError(10);
    while (true) delay(1000);
  }
  sensor.configureMotionInterrupt();
  classifier.begin();

  if (!oled.begin()) {
#if DEBUG_PRINT
    Serial.println("[Warn] OLED tidak terdeteksi, lanjut tanpa tampilan OLED.");
#endif
  }

  speaker.begin();

#if DEBUG_PRINT
  Serial.println("[Setup] Selesai. Siap deteksi gesture.");
#endif
}

void loop() {
  uint32_t now = millis();
  if (now - lastSampleMs < sampleIntervalMs) {
    // Isi waktu luang dengan efek idle breathing + cek deep sleep, tanpa nge-block sampling
    if (state == State::IDLE) leds.idleBreath();
    return;
  }
  lastSampleMs = now;

  GyroSample sample;
  if (!sensor.read(sample)) {
#if DEBUG_PRINT
    Serial.println("[Warn] Gagal baca sensor");
#endif
    return;
  }

  switch (state) {
    case State::IDLE: {
      float mag = sensor.accelMagnitude(sample);
      if (mag > GESTURE_TRIGGER_THRESHOLD_G) {
        power.notifyActivity();
        classifier.reset();
        classifier.addSample(sample);
        state = State::CAPTURING;
#if DEBUG_PRINT
        Serial.println("[State] Trigger terdeteksi, mulai capture gesture...");
#endif
      } else if (power.shouldSleep()) {
        leds.off();
        oled.showMessage("Tidur...", "Gerakkan utk bangun");
        power.enterDeepSleep(); // tidak return, ESP32 reset setelah bangun
      }
      break;
    }

    case State::CAPTURING: {
      bool full = classifier.addSample(sample);
      if (full) {
        state = State::CLASSIFYING;
      }
      break;
    }

    case State::CLASSIFYING: {
      Gesture g = classifier.classify();
#if DEBUG_PRINT
      Serial.printf("[Gesture] Terdeteksi: %s\n", gestureName(g));
#endif
      leds.playForGesture(g);
      oled.showGesture(gestureName(g));

      if (shouldTriggerAI(g)) {
        oled.showMessage("Menghubungi AI...", "");
        if (ai.connectWifi()) {
          AiReply reply;
          if (ai.sendGesture(gestureName(g), reply)) {
#if DEBUG_PRINT
            Serial.print("[AI] Respons: ");
            Serial.println(reply.text);
#endif
            oled.showAiResponse(reply.text.c_str());

            if (reply.audioUrl.length() > 0) {
#if DEBUG_PRINT
              Serial.print("[Audio] Streaming: ");
              Serial.println(reply.audioUrl);
#endif
              if (speaker.playUrl(reply.audioUrl)) {
                // WiFi WAJIB tetap nyala selama audio streaming -- jangan
                // disconnectWifi() sebelum playback selesai.
                uint32_t playStart = millis();
                while (speaker.isPlaying() && millis() - playStart < 20000) {
                  speaker.loop();
                }
              } else {
#if DEBUG_PRINT
                Serial.println("[Audio] Gagal mulai streaming, lanjut tanpa suara.");
#endif
                delay(3000); // fallback: beri waktu baca teks di OLED saja
              }
            } else {
              delay(3000); // tidak ada audio_url dari server, cukup tampilkan teks
            }
          } else {
#if DEBUG_PRINT
            Serial.println("[AI] Gagal mendapat respons (cek koneksi/server AI lokal).");
#endif
            leds.blinkError(2);
            oled.showMessage("AI tidak merespons", "Cek server lokal");
            delay(1500);
          }
          ai.disconnectWifi(); // matikan WiFi lagi untuk hemat daya (setelah audio selesai)
        } else {
#if DEBUG_PRINT
          Serial.println("[AI] Gagal connect WiFi.");
#endif
          leds.blinkError(2);
          oled.showMessage("WiFi gagal konek", "");
          delay(1500);
        }
      }

      classifier.reset();
      power.notifyActivity();
      state = State::IDLE;
      oled.showIdle();
      break;
    }
  }
}
