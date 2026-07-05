// =====================================================================
//  FaiWand Gesture AI Server
//
//  Jalankan:  node server/gesture_server.js
//  Port:      GESTURE_PORT di .env (default 5000, harus sama dengan
//             LOCAL_AI_PORT di firmware/magic_wand/config.h)
//
//  Kontrak API dengan firmware ESP32 (ai_client.cpp):
//    POST /gesture   body: {"gesture": "Wave"}
//                     balas: {"text": "...", "audio_url": "/audio/xxx.mp3"}
//    GET  /health     cek status server
//    GET  /audio/:file  file mp3 hasil TTS, di-stream & dimainkan ESP32
//                       lewat speaker (I2S) di tongkat -- BUKAN diputar
//                       di server lagi (speech-to-speech kini terjadi di
//                       tongkat, bukan di komputer server).
// =====================================================================

import "dotenv/config";
import express from "express";
import { MsEdgeTTS, OUTPUT_FORMAT } from "msedge-tts";
import { writeFile, unlink, mkdir } from "fs/promises";
import path from "path";
import { fileURLToPath } from "url";
import { askFastest } from "./ai4chat.js";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const PORT = process.env.GESTURE_PORT || 5000;
const AUDIO_DIR = path.join(__dirname, "public", "audio");
await mkdir(AUDIO_DIR, { recursive: true });

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
// Hasilnya disimpan sebagai file statis yang di-stream ESP32, BUKAN diputar
// di speaker server -- speech-to-speech kini terjadi fisik di tongkat.
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

  const filename = `gesture-${Date.now()}.mp3`;
  const filePath = path.join(AUDIO_DIR, filename);
  await writeFile(filePath, Buffer.concat(chunks));

  // Bersihkan file setelah 60 detik -- cukup waktu untuk ESP32 selesai streaming,
  // supaya folder public/audio tidak menumpuk file lama.
  setTimeout(() => unlink(filePath).catch(() => {}), 60_000);

  return filename;
}

const app = express();
app.use(express.json());
app.use("/audio", express.static(AUDIO_DIR));

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

  let audioUrl = "";
  try {
    const filename = await textToSpeechFile(answer);
    audioUrl = `/audio/${filename}`;
    console.log(`[FaiWand] Audio siap di-stream: ${audioUrl}`);
  } catch (err) {
    console.error("[FaiWand] TTS gagal, lanjut tanpa audio:", err.message);
  }

  res.json({ text, audio_url: audioUrl });
});

app.listen(PORT, () => {
  console.log(`\n🪄  FaiWand Gesture AI Server jalan di http://localhost:${PORT}`);
  console.log(`💬  POST http://localhost:${PORT}/gesture   body: {"gesture":"Wave"}`);
  console.log(`🔊  Audio TTS di-stream ke tongkat via /audio/*.mp3 (dimainkan di speaker ESP32)\n`);
});
