// audio_player.cpp
#include "audio_player.h"
#include "config.h"
#include <Audio.h> // dari library ESP32-audioI2S

static Audio audio;
static bool playing = false;

bool AudioPlayer::begin() {
#if !SPEAKER_ENABLED
  return false;
#else
  audio.setPinout(PIN_I2S_BCLK, PIN_I2S_LRC, PIN_I2S_DOUT);
  audio.setVolume(SPEAKER_VOLUME);
  return true;
#endif
}

bool AudioPlayer::playUrl(const String &url) {
#if !SPEAKER_ENABLED
  return false;
#else
  playing = audio.connecttohost(url.c_str());
  return playing;
#endif
}

void AudioPlayer::loop() {
#if SPEAKER_ENABLED
  audio.loop();
#endif
}

bool AudioPlayer::isPlaying() {
#if SPEAKER_ENABLED
  return audio.isRunning();
#else
  return false;
#endif
}

// Callback wajib dari library ESP32-audioI2S, dipanggil saat stream selesai.
void audio_eof_mp3(const char *info) {
  playing = false;
}
