// speech_to_text.js — transkrip suara (WAV 16kHz/16-bit mono dari ESP32) jadi teks,
// pakai model Whisper kecil yang jalan LOKAL (offline setelah model ter-download
// sekali di run pertama) lewat @xenova/transformers -- tidak ada audio yang
// dikirim ke cloud/API pihak ketiga.
import { pipeline } from "@xenova/transformers";

let transcriberPromise = null;

function getTranscriber() {
  if (!transcriberPromise) {
    console.log("[STT] Memuat model Whisper (tiny)... (pertama kali bisa lama, download model)");
    transcriberPromise = pipeline("automatic-speech-recognition", "Xenova/whisper-tiny");
  }
  return transcriberPromise;
}

// Parse WAV 16-bit PCM mono sederhana (sesuai yang dikirim mic_recorder.cpp:
// header 44-byte standar, data PCM16 setelahnya) -> Float32Array (-1.0..1.0)
function wavToFloat32(buffer) {
  const dataOffset = 44; // header WAV standar 44 byte
  const pcm = buffer.subarray(dataOffset);
  const sampleCount = pcm.length / 2;
  const float32 = new Float32Array(sampleCount);
  for (let i = 0; i < sampleCount; i++) {
    const int16 = pcm.readInt16LE(i * 2);
    float32[i] = int16 / 32768;
  }
  return float32;
}

export async function transcribeWav(wavBuffer, sampleRate = 16000) {
  const transcriber = await getTranscriber();
  const audioData = wavToFloat32(wavBuffer);
  const result = await transcriber(audioData, { sampling_rate: sampleRate, language: "indonesian" });
  return (result?.text || "").trim();
}
