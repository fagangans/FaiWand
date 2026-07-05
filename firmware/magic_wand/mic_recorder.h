// mic_recorder.h — rekam suara dari mikrofon I2S (INMP441) ke buffer WAV di RAM
#pragma once
#include <Arduino.h>

class MicRecorder {
 public:
  bool begin();

  // Rekam sampai stopCondition() return true (biasanya "tombol dilepas") atau
  // sampai MIC_MAX_RECORD_MS tercapai. Return false kalau rekaman terlalu
  // pendek (< MIC_MIN_RECORD_MS) -- dianggap tidak sengaja, tidak usah dikirim.
  bool record(bool (*stopCondition)());

  // Buffer WAV siap kirim (sudah termasuk header WAV 44-byte) setelah record().
  const uint8_t *wavData() const;
  size_t wavSize() const;

 private:
  uint8_t *buffer_ = nullptr;
  size_t recordedBytes_ = 0;
  size_t capacityBytes_ = 0;
};
