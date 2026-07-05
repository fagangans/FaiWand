# AI Magic Wand (ESP32 + MPU6050 + OLED + Mic + Speaker)

Proyek "AI Magic Wand" terinspirasi dari [TensorFlow Lite Magic Wand Codelab](https://codelabs.developers.google.com/codelabs/ai-magicwand)
Google, tapi dibangun dengan hardware yang jauh lebih murah dan gampang didapat:
**ESP32 DevKit V1 + MPU6050 + OLED SSD1306 0.96" + mikrofon I2S (INMP441) + speaker
I2S (MAX98357A)**, terintegrasi dengan **AI lokal (speech-to-speech dua arah)** milik
kamu sendiri di jaringan LAN — bukan cloud API.

Kamu bisa **bicara ke tongkat** (push-to-talk, tahan tombol BOOT) dan AI menjawab lewat
suara + teks di tongkat, ATAU **menggerakkan tongkat** (gesture) untuk trigger respons
tetap yang lebih ringkas.

## Struktur repo

```
firmware/magic_wand/     -> Sketch Arduino (buka magic_wand.ino di Arduino IDE)
server/                   -> Server AI lokal Node.js (gesture/suara -> teks + audio TTS)
docs/BOM.md               -> Daftar komponen + estimasi harga
docs/WIRING.md             -> Panduan wiring lengkap + setup jaringan AI lokal
docs/EDGE_IMPULSE_GUIDE.md -> Cara training model gesture custom
docs/ENCLOSURE.md          -> Saran casing/enclosure tongkat
enclosure/                 -> Model 3D gagang (.scad + .stl siap print)
```

## Arsitektur singkat

```
                     ┌── GESTURE (Lumos/Wave/Circle) ──┐   ┌── PUSH-TO-TALK (tombol BOOT) ──┐
                     |  MPU6050 -> ESP32 klasifikasi    |   |  Mikrofon I2S rekam ke RAM      |
                     v                                  v   v
                              WiFi (LAN) --> Server AI lokal kamu
                                        |
                    (gesture: prompt tetap per gesture)
                    (suara: transkrip Whisper lokal -> jadi prompt)
                                        |
                         ┌──────────────┴──────────────┐
                         v                              v
                  Teks respons AI                Audio TTS (mp3)
                  (JSON, untuk OLED)         (di-stream via HTTP ke ESP32)
                         |                              |
                         v                              v
                  Tampil di OLED          Dimainkan lewat speaker I2S
                                          di tongkat (MAX98357A + speaker)
```

**Kenapa klasifikasi gesture tetap on-device (bukan kirim raw sensor ke AI)?** Time-series
classification real-time tidak cocok dikirim sebagai data mentah — lambat dan boros
baterai/bandwidth. Jadi gesture diklasifikasi lokal di ESP32 (model kecil via Edge Impulse,
atau fallback threshold sederhana), baru **hasil klasifikasinya** yang dikirim ke server.

**Kenapa suara (TTS & STT) diproses di server tapi didengar/direkam di tongkat?** ESP32
tidak mampu generate suara atau transkrip suara sendiri — terlalu berat untuk board sekecil
ini. Server (komputer kamu) yang punya daya proses untuk itu:
- **Bicara ke tongkat**: ESP32 rekam suara ke RAM → kirim WAV ke server → server transkrip
  (Whisper lokal, offline) → AI jawab sesuai isi omongan → jawaban (teks+audio) dikirim balik.
- **Tongkat menjawab**: server generate teks + audio TTS → ESP32 **streaming** audio itu dan
  memutarnya langsung lewat speaker I2S di tongkat.

Hasilnya speech-to-speech terasa benar-benar terjadi di tongkat, walau otak pemrosesannya
ada di server lokal kamu.

## Quick start

1. **Hardware**: ikuti `docs/BOM.md` dan `docs/WIRING.md` untuk rakit board (OLED, speaker
   I2S, DAN mikrofon I2S).
2. **Software firmware**:
   - Install Arduino IDE + board support "esp32" (via Boards Manager, cari "esp32" by Espressif).
   - Install library lewat Library Manager: `Adafruit MPU6050`, `Adafruit Unified Sensor`,
     `Adafruit SSD1306`, `Adafruit GFX Library`, `ArduinoJson`, `ESP32-audioI2S` (by schreibfaul1).
     (Mikrofon pakai driver I2S bawaan Arduino-ESP32 core, tidak perlu library tambahan.)
   - Buka folder `firmware/magic_wand/` di Arduino IDE (buka file `magic_wand.ino`).
   - Copy `secrets.h.example` jadi `secrets.h`, isi WiFi SSID/password dan `LOCAL_AI_HOST`
     (hostname mDNS atau IP server AI lokal kamu).
   - Pilih board **"ESP32 Dev Module"** di Tools > Board, pilih port yang sesuai, lalu Upload.
3. **Server AI lokal**: implementasi siap pakai ada di `server/` (Node.js, tanpa API key untuk
   chat AI — pakai AI4Chat + TTS Edge Neural Voice + STT Whisper lokal). Jalankan:
   ```
   cd server && npm install && npm start
   ```
   Lihat `server/README.md` untuk detail kontrak API (`/gesture`, `/voice`, `/audio/:file`).
4. **Test awal (tanpa training model dulu)**: firmware default pakai fallback threshold
   sederhana untuk gesture (`USE_EDGE_IMPULSE_MODEL 0` di `gesture_classifier.h`) — cukup
   untuk memverifikasi wiring, LED, OLED, speaker, mikrofon, dan koneksi ke server AI lokal
   sebelum kamu invest waktu training model gesture asli.
5. **Training model gesture asli**: ikuti `docs/EDGE_IMPULSE_GUIDE.md` step-by-step.
6. **Casing**: file 3D print siap pakai ada di `enclosure/` (STL + sumber `.scad`, sudah
   termasuk lubang mikrofon), atau lihat opsi lain di `docs/ENCLOSURE.md`.

## Cara pakai

| Metode | Aksi |
|---|---|
| **Push-to-talk** | Tahan tombol BOOT, bicara, lepas untuk kirim. AI jawab SESUAI ISI OMONGAN kamu. |
| Gesture `Lumos` | LED menyala kuning terang, tahan ~1.2 detik; nama gesture tampil di OLED |
| Gesture `Wave` | LED kedip biru 4x, kirim ke server AI (prompt TETAP "sapa pengguna..."), tampil + suarakan respons |
| Gesture `Circle` | LED efek sweep ungu-pink; nama gesture tampil di OLED |

Gesture, warna, prompt tetap tiap gesture, dan mana yang trigger AI semuanya bisa diubah di
`firmware/magic_wand/led_effects.cpp`, `firmware/magic_wand/magic_wand.ino`, dan
`server/gesture_server.js` (`GESTURE_PROMPTS`).

## Power management

- Board masuk **deep sleep** otomatis setelah idle (default 30 detik, ubah `IDLE_TIMEOUT_MS`
  di `config.h`).
- Bangun otomatis lewat **interrupt motion-detection MPU6050** (pin `PIN_MPU_INT`) — begitu
  tongkat digerakkan, ESP32 langsung reset & siap deteksi gesture lagi.
- WiFi hanya dinyalakan saat mengirim gesture/suara ke AI lokal dan selama audio streaming
  berlangsung, lalu dimatikan lagi — sehingga hemat baterai saat idle/deep sleep.
- **Catatan baterai**: speaker + amplifier + mikrofon menambah konsumsi arus saat aktif
  (~500mA-1A tergantung volume) — kalau baterai kamu kecil (300-400mAh), daya tahan akan
  berkurang lebih banyak dibanding versi tanpa audio, terutama kalau sering dipakai bicara.

## Catatan penting

- **Jangan commit `secrets.h`** ke git (sudah otomatis di-ignore lewat `.gitignore`) — file itu
  berisi WiFi password dan alamat server AI lokal.
- Perhatikan catatan power supply di `docs/WIRING.md` soal regulator ESP32 vs tegangan LiPo
  3.7V — banyak board versi murah butuh boost converter tambahan biar stabil.
- Server AI lokal dan ESP32 **harus di WiFi/LAN yang sama**. Kalau pakai hostname mDNS
  (`xxx.local`), pastikan OS server mendukung mDNS (lihat `docs/WIRING.md`).
- MAX98357A disambung ke **VIN**, bukan 3V3 (butuh daya lebih untuk menggerakkan speaker) —
  lihat `docs/WIRING.md` untuk detail wiring.
- Model Whisper (speech-to-text) di-download otomatis ~150MB saat pertama kali server
  menerima request `/voice` — butuh internet saat itu saja, setelahnya transkripsi jalan
  offline sepenuhnya.
