// mic_recorder.cpp
// Pakai driver I2S bawaan Arduino-ESP32 core (driver/i2s.h) -- tidak perlu
// library tambahan untuk baca mikrofon (beda dengan speaker yang pakai
// ESP32-audioI2S untuk streaming MP3).
#include "mic_recorder.h"
#include "config.h"
#include <driver/i2s.h>

#define WAV_HEADER_SIZE 44

static void writeWavHeader(uint8_t *buf, uint32_t pcmBytes) {
  uint32_t byteRate = MIC_SAMPLE_RATE_HZ * 2; // 16-bit mono
  uint32_t chunkSize = 36 + pcmBytes;

  memcpy(buf, "RIFF", 4);
  memcpy(buf + 4, &chunkSize, 4);
  memcpy(buf + 8, "WAVE", 4);
  memcpy(buf + 12, "fmt ", 4);
  uint32_t subChunk1Size = 16;
  memcpy(buf + 16, &subChunk1Size, 4);
  uint16_t audioFormat = 1; // PCM
  memcpy(buf + 20, &audioFormat, 2);
  uint16_t numChannels = 1;
  memcpy(buf + 22, &numChannels, 2);
  uint32_t sampleRate = MIC_SAMPLE_RATE_HZ;
  memcpy(buf + 24, &sampleRate, 4);
  memcpy(buf + 28, &byteRate, 4);
  uint16_t blockAlign = 2;
  memcpy(buf + 32, &blockAlign, 2);
  uint16_t bitsPerSample = 16;
  memcpy(buf + 34, &bitsPerSample, 2);
  memcpy(buf + 36, "data", 4);
  memcpy(buf + 40, &pcmBytes, 4);
}

bool MicRecorder::begin() {
#if !MIC_ENABLED
  return false;
#else
  i2s_config_t i2sConfig = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = MIC_SAMPLE_RATE_HZ,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // INMP441 kirim 32-bit slot (data 24-bit)
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = 0,
      .dma_buf_count = 4,
      .dma_buf_len = 256,
      .use_apll = false,
  };
  i2s_pin_config_t pinConfig = {
      .bck_io_num = PIN_MIC_SCK,
      .ws_io_num = PIN_MIC_WS,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = PIN_MIC_SD,
  };

  if (i2s_driver_install(I2S_NUM_1, &i2sConfig, 0, NULL) != ESP_OK) return false;
  if (i2s_set_pin(I2S_NUM_1, &pinConfig) != ESP_OK) return false;

  capacityBytes_ = WAV_HEADER_SIZE + (size_t)((MIC_SAMPLE_RATE_HZ * 2) * (MIC_MAX_RECORD_MS / 1000.0));
  buffer_ = (uint8_t *)malloc(capacityBytes_);
  return buffer_ != nullptr;
#endif
}

bool MicRecorder::record(bool (*stopCondition)()) {
#if !MIC_ENABLED
  return false;
#else
  if (!buffer_) return false;

  recordedBytes_ = 0;
  uint8_t *pcmStart = buffer_ + WAV_HEADER_SIZE;
  size_t maxPcmBytes = capacityBytes_ - WAV_HEADER_SIZE;

  int32_t rawSamples[256];
  uint32_t startMs = millis();

  while (recordedBytes_ < maxPcmBytes && (millis() - startMs) < MIC_MAX_RECORD_MS) {
    if (stopCondition()) break;

    size_t bytesRead = 0;
    i2s_read(I2S_NUM_1, rawSamples, sizeof(rawSamples), &bytesRead, 50 / portTICK_PERIOD_MS);
    int samplesRead = bytesRead / sizeof(int32_t);

    for (int i = 0; i < samplesRead && recordedBytes_ < maxPcmBytes; i++) {
      // INMP441 kirim data 24-bit di 32-bit slot, geser ke 16-bit PCM
      int16_t sample16 = (int16_t)(rawSamples[i] >> 14);
      memcpy(pcmStart + recordedBytes_, &sample16, 2);
      recordedBytes_ += 2;
    }
  }

  uint32_t durationMs = (recordedBytes_ / 2) * 1000 / MIC_SAMPLE_RATE_HZ;
  if (durationMs < MIC_MIN_RECORD_MS) {
    return false; // terlalu pendek, dianggap tidak sengaja
  }

  writeWavHeader(buffer_, recordedBytes_);
  return true;
#endif
}

const uint8_t *MicRecorder::wavData() const {
  return buffer_;
}

size_t MicRecorder::wavSize() const {
  return WAV_HEADER_SIZE + recordedBytes_;
}
