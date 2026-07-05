# AI Magic Wand (ESP32 + MPU6050 + OLED)

Proyek "AI Magic Wand" terinspirasi dari [TensorFlow Lite Magic Wand Codelab](https://codelabs.developers.google.com/codelabs/ai-magicwand)
Google, tapi dibangun dengan hardware yang jauh lebih murah dan gampang didapat:
**ESP32 DevKit V1 + MPU6050 + OLED SSD1306 0.96"**, terintegrasi dengan **AI lokal
(speech-to-speech/speech-to-text)** milik kamu sendiri di jaringan LAN — bukan cloud API.

## Struktur repo

```
firmware/magic_wand/     -> Sketch Arduino (buka magic_wand.ino di Arduino IDE)
docs/BOM.md               -> Daftar komponen + estimasi harga
docs/WIRING.md             -> Panduan wiring lengkap + setup jaringan AI lokal
docs/EDGE_IMPULSE_GUIDE.md -> Cara training model gesture custom
docs/ENCLOSURE.md          -> Saran casing/enclosure tongkat
```

## Arsitektur singkat

```
MPU6050 (accel+gyro) --> ESP32 --> klasifikasi gesture (on-device)
                                        |
                            ┌───────────┴───────────┐
                            v                        v
                     LED RGB (feedback)       OLED (tampilkan nama gesture)
                                        |
                          (gesture tertentu, mis. "Wave")
                                        v
                        WiFi (LAN) --> Server AI lokal kamu
                              (speech-to-speech / speech-to-text)
                                        |
                                        v
                       Respons teks ditampilkan di OLED
                 (AI lokal menangani sendiri output suaranya)
```

**Kenapa klasifikasi tetap on-device (bukan kirim raw sensor ke AI)?** Time-series classification
real-time tidak cocok dikirim sebagai data mentah — lambat dan boros baterai/bandwidth. Jadi:
gesture diklasifikasi lokal di ESP32 (model kecil via Edge Impulse, atau fallback threshold
sederhana), lalu **hasil klasifikasi** (nama gesture, misal `"Wave"`) yang dikirim ke server AI
lokal kamu untuk diproses lebih lanjut (mis. dijadikan trigger untuk speech-to-speech).

## Quick start

1. **Hardware**: ikuti `docs/BOM.md` dan `docs/WIRING.md` untuk rakit board (termasuk wiring OLED).
2. **Software**:
   - Install Arduino IDE + board support "esp32" (via Boards Manager, cari "esp32" by Espressif).
   - Install library lewat Library Manager: `Adafruit MPU6050`, `Adafruit Unified Sensor`,
     `Adafruit SSD1306`, `Adafruit GFX Library`, `ArduinoJson`.
   - Buka folder `firmware/magic_wand/` di Arduino IDE (buka file `magic_wand.ino`).
   - Copy `secrets.h.example` jadi `secrets.h`, isi WiFi SSID/password dan `LOCAL_AI_HOST`
     (hostname mDNS atau IP server AI lokal kamu).
   - Pilih board **"ESP32 Dev Module"** di Tools > Board, pilih port yang sesuai, lalu Upload.
3. **Siapkan endpoint di server AI lokal**: firmware mengirim `POST http://<host>:5000/gesture`
   dengan body `{"gesture": "Wave"}`, dan mengharapkan balasan JSON `{"text": "..."}`. Kalau
   endpoint/format server AI lokal kamu beda, sesuaikan di `config.h` (`LOCAL_AI_PORT`,
   `LOCAL_AI_PATH`) dan `ai_client.cpp` (key JSON request/response).
4. **Test awal (tanpa training model dulu)**: firmware default pakai fallback threshold
   sederhana (`USE_EDGE_IMPULSE_MODEL 0` di `gesture_classifier.h`) — cukup untuk memverifikasi
   wiring, LED, OLED, dan koneksi ke server AI lokal sebelum kamu invest waktu training model asli.
5. **Training model gesture asli**: ikuti `docs/EDGE_IMPULSE_GUIDE.md` step-by-step.
6. **Casing**: lihat opsi-opsi di `docs/ENCLOSURE.md`.

## Gesture default yang didukung

| Gesture | Aksi |
|---|---|
| `Lumos` | LED menyala kuning terang, tahan ~1.2 detik; nama gesture tampil di OLED |
| `Wave` | LED kedip biru 4x, DAN mengirim event ke server AI lokal, menampilkan respons teks di OLED |
| `Circle` | LED efek sweep ungu-pink; nama gesture tampil di OLED |

Gesture, warna, dan aksi mana yang trigger AI lokal semuanya bisa diubah di
`firmware/magic_wand/led_effects.cpp` dan `firmware/magic_wand/magic_wand.ino`
(lihat fungsi `shouldTriggerAI()`).

## Power management

- Board masuk **deep sleep** otomatis setelah idle (default 30 detik, ubah `IDLE_TIMEOUT_MS`
  di `config.h`).
- Bangun otomatis lewat **interrupt motion-detection MPU6050** (pin `PIN_MPU_INT`) — begitu
  tongkat digerakkan, ESP32 langsung reset & siap deteksi gesture lagi.
- WiFi hanya dinyalakan saat mengirim gesture ke AI lokal, lalu dimatikan lagi — sehingga hemat
  baterai saat idle/deep sleep.

## Catatan penting

- **Jangan commit `secrets.h`** ke git (sudah otomatis di-ignore lewat `.gitignore`) — file itu
  berisi WiFi password dan alamat server AI lokal.
- Perhatikan catatan power supply di `docs/WIRING.md` soal regulator ESP32 vs tegangan LiPo
  3.7V — banyak board versi murah butuh boost converter tambahan biar stabil.
- Server AI lokal dan ESP32 **harus di WiFi/LAN yang sama**. Kalau pakai hostname mDNS
  (`xxx.local`), pastikan OS server mendukung mDNS (lihat `docs/WIRING.md`).
