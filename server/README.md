# FaiWand Gesture AI Server

Server AI lokal untuk AI Magic Wand — menerima **gesture ATAU rekaman suara (push-to-talk)**
dari ESP32, transkrip suara pakai Whisper lokal (kalau lewat suara), membalas teks singkat
untuk OLED, dan generate audio TTS yang di-**stream** ke ESP32 lewat HTTP untuk dimainkan
langsung di speaker tongkat (speech-to-speech dua arah, semua terjadi di tongkat). Tidak
butuh API key untuk chat AI-nya (pakai AI4Chat, dengan fallback PublicAI).

Diadaptasi dari `Ai4Chat.js` + `whisper-server.js` di repo Faganlenwy, disesuaikan dengan
kontrak API yang sudah diasumsikan firmware ESP32 (`ai_client.cpp` + `config.h`).

## Kontrak API

```
POST /gesture
Body:  {"gesture": "Wave"}
Balas: {"text": "Halo! Selamat datang...", "audio_url": "/audio/gesture-171234.mp3"}
```

```
POST /voice                       (push-to-talk: kamu bicara, AI jawab sesuai isi omongan)
Body:  raw bytes WAV 16kHz/16-bit mono (Content-Type: audio/wav)
Balas: {"text": "...", "audio_url": "/audio/gesture-171234.mp3"}
```

```
GET /audio/:filename
Balas: file mp3 (di-stream ESP32 lewat library ESP32-audioI2S)
```

```
GET /health
Balas: {"status": "ok", ...}
```

## Jalankan

```
cd server
npm install
cp .env.example .env   # opsional, default port 5000
npm start
```

Server berjalan di `http://localhost:5000` (atau IP LAN mesin ini). Set `LOCAL_AI_HOST` di
`firmware/magic_wand/secrets.h` ke hostname/IP mesin ini, dan pastikan ESP32 + server ada di
WiFi/LAN yang sama.

**Catatan `npm install` pertama kali**: dependency `@xenova/transformers` (untuk speech-to-text
Whisper lokal) cukup besar, dan pertama kali `/voice` dipanggil, model Whisper-tiny akan
di-download otomatis (~150MB, sekali saja, di-cache lokal) — butuh koneksi internet saat itu,
tapi **setelahnya semua transkripsi jalan offline**, tidak ada audio yang dikirim ke cloud.

## Menambah/mengubah gesture

Edit `GESTURE_PROMPTS` di `gesture_server.js` — tiap gesture punya satu prompt tetap yang
dikirim ke AI. Gesture yang tidak terdaftar memakai `DEFAULT_PROMPT`. Ini hanya berlaku untuk
jalur `/gesture` — jalur `/voice` selalu pakai isi transkrip suara kamu sebagai prompt.

## Alur audio: kenapa file mp3, bukan diputar langsung di server?

1. Server dapat teks (dari AI4Chat/PublicAI, berdasarkan gesture ATAU transkrip suara), lalu
   generate audio TTS-nya (Edge Neural Voice), simpan sebagai file mp3 sementara di
   `server/public/audio/`.
2. Server balas ke ESP32: `{"text": ..., "audio_url": "/audio/xxx.mp3"}`.
3. ESP32 menampilkan teks di OLED **segera**, lalu **streaming** file audio itu langsung dari
   URL tersebut dan memutarnya lewat speaker I2S yang terpasang di tongkat.
4. File mp3 di server otomatis dihapus ~60 detik kemudian (cukup waktu untuk ESP32 selesai
   streaming), supaya folder `public/audio/` tidak menumpuk file lama.

Kalau kamu mau ubah durasi cleanup, ubah angka `60_000` (ms) di `textToSpeechFile()` pada
`gesture_server.js`.

## Troubleshooting

- **`npm install` gagal di paket `sharp`**: `sharp` adalah dependency dari `@xenova/transformers`
  (dipakai untuk model vision, tidak kita pakai untuk audio, tapi tetap ter-install). Kalau
  gagal karena masalah jaringan/firewall saat download binary, coba ulangi `npm install` di
  jaringan yang tidak dibatasi proxy/firewall ketat. Ini masalah environment, bukan bug di kode.
- **AI4Chat & PublicAI dua-duanya gagal**: endpoint publik ini kadang rate-limit/down —
  server akan balas HTTP 502 ke ESP32. Cek log server untuk detail error.
- **ESP32 tidak bisa connect**: pastikan port `GESTURE_PORT` (default 5000) sama dengan
  `LOCAL_AI_PORT` di `config.h`, dan firewall OS server tidak memblokir port tersebut di LAN.
- **Suara tidak keluar di tongkat**: cek Serial Monitor ESP32 — kalau log `[Audio] Gagal mulai
  streaming`, kemungkinan `audio_url` tidak terjangkau dari ESP32 (beda jaringan, firewall,
  atau port server tertutup). Test manual dulu buka URL `audio_url` itu di browser dari device
  lain yang satu jaringan dengan ESP32.
- **Audio putus-putus/berisik**: biasanya karena WiFi lemah (sinyal jauh dari router) atau
  server terlalu sibuk generate TTS bersamaan dengan request lain. Pastikan ESP32 dekat
  dengan router/AP saat testing.
- **Transkrip suara ngaco/kosong**: model `whisper-tiny` itu model paling kecil/cepat, akurasi
  lumayan tapi tidak sempurna terutama untuk Bahasa Indonesia dengan aksen tertentu atau
  lingkungan berisik. Kalau butuh lebih akurat (tapi lebih lambat & lebih berat di CPU), ganti
  `Xenova/whisper-tiny` di `speech_to_text.js` jadi `Xenova/whisper-base` atau `Xenova/whisper-small`.
