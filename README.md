# AI Magic Wand (ESP32 + MPU6050)

Proyek "AI Magic Wand" terinspirasi dari [TensorFlow Lite Magic Wand Codelab](https://codelabs.developers.google.com/codelabs/ai-magicwand)
Google, tapi dibangun dengan hardware yang jauh lebih murah dan gampang didapat:
**ESP32 DevKit V1 + MPU6050**, ditambah integrasi ke AI API (OpenAI/Grok) untuk respons
teks berdasarkan gesture yang terdeteksi.

## Struktur repo

```
firmware/magic_wand/     -> Sketch Arduino (buka magic_wand.ino di Arduino IDE)
docs/BOM.md               -> Daftar komponen + estimasi harga
docs/WIRING.md             -> Panduan wiring lengkap
docs/EDGE_IMPULSE_GUIDE.md -> Cara training model gesture custom
docs/ENCLOSURE.md          -> Saran casing/enclosure tongkat
```

## Arsitektur singkat

```
MPU6050 (accel+gyro) --> ESP32 --> klasifikasi gesture (on-device)
                                        |
                                        v
                                 LED RGB (feedback visual)
                                        |
                          (gesture tertentu, mis. "Wave")
                                        v
                                WiFi --> OpenAI/Grok API
                                        |
                                        v
                              Respons teks (Serial/app)
```

**Kenapa klasifikasi tetap on-device (bukan kirim raw sensor ke LLM)?** Karena LLM text-based
tidak cocok untuk time-series classification real-time, dan mengirim data mentah tiap gerakan
akan lambat & boros kuota/baterai. Jadi: gesture diklasifikasi lokal pakai model kecil (TFLite
Micro via Edge Impulse, atau fallback threshold sederhana), lalu **hasil klasifikasi** (nama
gesture) yang dikirim ke AI API untuk mendapat respons kontekstual.

## Quick start

1. **Hardware**: ikuti `docs/BOM.md` dan `docs/WIRING.md` untuk rakit board.
2. **Software**:
   - Install Arduino IDE + board support "esp32" (via Boards Manager, cari "esp32" by Espressif).
   - Install library lewat Library Manager: `Adafruit MPU6050`, `Adafruit Unified Sensor`,
     `ArduinoJson`.
   - Buka folder `firmware/magic_wand/` di Arduino IDE (buka file `magic_wand.ino`).
   - Copy `secrets.h.example` jadi `secrets.h`, isi WiFi SSID/password dan API key
     OpenAI/Grok kamu.
   - Pilih board **"ESP32 Dev Module"** di Tools > Board, pilih port yang sesuai, lalu Upload.
3. **Test awal (tanpa training model dulu)**: firmware default pakai fallback threshold
   sederhana (`USE_EDGE_IMPULSE_MODEL 0` di `gesture_classifier.h`) — cukup untuk memverifikasi
   wiring, LED, dan koneksi AI API bekerja sebelum kamu invest waktu training model asli.
4. **Training model gesture asli**: ikuti `docs/EDGE_IMPULSE_GUIDE.md` step-by-step.
5. **Casing**: lihat opsi-opsi di `docs/ENCLOSURE.md`.

## Gesture default yang didukung

| Gesture | Aksi |
|---|---|
| `Lumos` | LED menyala kuning terang, tahan ~1.2 detik |
| `Wave` | LED kedip biru 4x, DAN mengirim event ke AI API, menampilkan respons teks di Serial Monitor |
| `Circle` | LED efek sweep ungu-pink |

Gesture, warna, dan aksi mana yang trigger AI API semuanya bisa diubah di
`firmware/magic_wand/led_effects.cpp` dan `firmware/magic_wand/magic_wand.ino`
(lihat fungsi `shouldTriggerAI()`).

## Power management

- Board masuk **deep sleep** otomatis setelah idle (default 30 detik, ubah `IDLE_TIMEOUT_MS`
  di `config.h`).
- Bangun otomatis lewat **interrupt motion-detection MPU6050** (pin `PIN_MPU_INT`) — begitu
  tongkat digerakkan, ESP32 langsung reset & siap deteksi gesture lagi.

## Catatan penting

- **Jangan commit `secrets.h`** ke git (sudah otomatis di-ignore lewat `.gitignore`) — file itu
  berisi WiFi password dan API key asli.
- Perhatikan catatan power supply di `docs/WIRING.md` soal regulator ESP32 vs tegangan LiPo
  3.7V — banyak board versi murah butuh boost converter tambahan biar stabil.
