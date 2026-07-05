// =====================================================================
//  FaiWand Gesture AI Server
//
//  Jalankan:  node server/gesture_server.js
//  Port:      GESTURE_PORT di .env (default 5000, harus sama dengan
//             LOCAL_AI_PORT di firmware/magic_wand/config.h)
//
//  Kontrak API dengan firmware ESP32 (ai_client.cpp):
//    POST /gesture   body: {"gesture": "Wave"}
//                     balas: {"text": "..."}  (JSON, ditampilkan di OLED)
//    GET  /health     cek status server
//
//  Selain balas JSON ke ESP32, server ini JUGA mengucapkan jawabannya lewat
//  speaker lokal (speech-to-speech) — ESP32 sendiri tidak menerima audio,
//  hanya teks singkat untuk OLED.
// =====================================================================

import "dotenv/config";
import express from "express";
import { MsEdgeTTS, OUTPUT_FORMAT } from "msedge-tts";
import player from "play-sound";
import { writeFile, unlink } from "fs/promises";
import { tmpdir } from "os";
import path from "path";
import { askFastest } from "./ai4chat.js";

const PORT = process.env.GESTURE_PORT || 5000;
const audioPlayer = player({});

// ---- Mapping gesture -> prompt AI (prompt tetap, mudah ditambah/diubah) ----
const GESTURE_PROMPTS = {
  Wave: "Sapa pengguna secara singkat, ramah, dan ceria dalam satu kalimat pendek.",
  Lumos: "Berikan satu kalimat motivasi singkat yang menerangi semangat, seperti cahaya.",
  Circle: "Berikan satu fakta unik atau menarik secara singkat dalam satu kalimat.",
};
const DEFAULT_PROMPT = "Berikan satu kalimat singkat dan ramah untuk pengguna tongkat sihir.";

// Batas panjang teks yang dikirim ke OLED (128x64, font kecil ~21 karakter/baris, muat beberapa baris)
const MAX_OLED_CHARS = 120;

function trimForOled(text) {
  const clean = text.replace(/\s+/g, " ").trim();
  return clean.length > MAX_OLED_CHARS ? clean.slice(0, MAX_OLED_CHARS - 1).trim() + "…" : clean;
}

// ---- TTS: Edge Neural Voice (gratis, tanpa API key) ----
async function textToSpeechFile(text) {
  const tts = new MsEdgeTTS();
  await tts.setMetadata("id-ID-ArdiNeural", OUTPUT_FORMAT.AUDIO_24KHZ_96KBITRATE_MONO_MP3);
  const { audioStream } = tts.toStream(text, { rate: "+10%" });

  const chunks = [];
  await new Promise((resolve, reject) => {
    audioStream.on("data", (chunk) => chunks.push(chunk));
    audioStream.on("close", resolve);
    audioStream.on("error", reject);
  });
  tts.close();

  const filePath = path.join(tmpdir(), `faiwand-gesture-${Date.now()}.mp3`);
  await writeFile(filePath, Buffer.concat(chunks));
  return filePath;
}

// Ucapkan teks lewat speaker lokal server (fire-and-forget, tidak menahan response ESP32)
async function speak(text) {
  let filePath;
  try {
    filePath = await textToSpeechFile(text);
    await new Promise((resolve) => {
      audioPlayer.play(filePath, (err) => {
        if (err) console.error("[FaiWand] Gagal memutar audio:", err.message);
        resolve();
      });
    });
  } catch (err) {
    console.error("[FaiWand] TTS/speaker gagal:", err.message);
  } finally {
    if (filePath) unlink(filePath).catch(() => {});
  }
}

const app = express();
app.use(express.json());

app.get("/health", (req, res) => {
  res.json({ status: "ok", ai: "AI4Chat + PublicAI (tanpa API key)", tts: "Microsoft Edge Neural TTS" });
});

// ---- POST /gesture — kontrak wajib dengan ai_client.cpp ----
app.post("/gesture", async (req, res) => {
  const gesture = req.body?.gesture;
  if (!gesture || typeof gesture !== "string") {
    return res.status(400).json({ error: "Field 'gesture' wajib diisi (string)." });
  }

  const prompt = GESTURE_PROMPTS[gesture] || DEFAULT_PROMPT;
  console.log(`[FaiWand] Gesture diterima: "${gesture}"`);

  let answer;
  try {
    answer = await askFastest(prompt);
  } catch (err) {
    console.error("[FaiWand] AI gagal:", err.message);
    return res.status(502).json({ error: "AI tidak merespons." });
  }

  const text = trimForOled(answer);
  console.log(`[FaiWand] Jawaban untuk OLED: "${text}"`);

  // Balas ke ESP32 dulu (OLED harus cepat update), speaker jalan di background
  res.json({ text });
  speak(answer);
});

app.listen(PORT, () => {
  console.log(`\n🪄  FaiWand Gesture AI Server jalan di http://localhost:${PORT}`);
  console.log(`💬  POST http://localhost:${PORT}/gesture   body: {"gesture":"Wave"}`);
  console.log(`🔊  Jawaban juga diucapkan lewat speaker lokal (speech-to-speech)\n`);
});
