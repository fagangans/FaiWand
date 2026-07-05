# FaiWand Gesture AI Server

Server AI lokal untuk AI Magic Wand — menerima gesture dari ESP32, membalas teks singkat
untuk OLED, dan mengucapkan jawabannya lewat speaker lokal server (speech-to-speech). Tidak
butuh API key (pakai AI4Chat, dengan fallback PublicAI).

Diadaptasi dari `Ai4Chat.js` + `whisper-server.js` di repo Faganlenwy, disesuaikan dengan
kontrak API yang sudah diasumsikan firmware ESP32 (`ai_client.cpp` + `config.h`).

## Kontrak API

```
POST /gesture
Body:  {"gesture": "Wave"}
Balas: {"text": "Halo! Selamat datang..."}
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

## Menambah/mengubah gesture

Edit `GESTURE_PROMPTS` di `gesture_server.js` — tiap gesture punya satu prompt tetap yang
dikirim ke AI. Gesture yang tidak terdaftar memakai `DEFAULT_PROMPT`.

## Kenapa server balas JSON dulu baru bicara?

ESP32 (`ai_client.cpp`) hanya menunggu respons HTTP untuk menampilkan teks di OLED (timeout
10 detik) — tidak menerima/memutar audio. Server membalas `{"text": ...}` secepatnya, lalu
memutar audio TTS di speaker lokal secara background (`speak()` di `gesture_server.js`),
supaya OLED tidak ikut menunggu proses text-to-speech selesai.

## Troubleshooting

- **AI4Chat & PublicAI dua-duanya gagal**: endpoint publik ini kadang rate-limit/down —
  server akan balas HTTP 502 ke ESP32. Cek log server untuk detail error.
- **Tidak ada suara keluar**: `play-sound` memerlukan pemutar audio command-line di OS
  (`afplay` di macOS — sudah ada bawaan; `mpg123`/`mplayer`/`ffplay` di Linux — install salah
  satu, mis. `sudo apt install mpg123`; Windows pakai `cmdmp3` bawaan `play-sound`).
- **ESP32 tidak bisa connect**: pastikan port `GESTURE_PORT` (default 5000) sama dengan
  `LOCAL_AI_PORT` di `config.h`, dan firewall OS server tidak memblokir port tersebut di LAN.
