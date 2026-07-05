# Bill of Materials — AI Magic Wand

| # | Komponen | Spesifikasi | Perkiraan Harga (IDR) | Catatan |
|---|----------|-------------|------------------------|---------|
| 1 | ESP32 DevKit V1 | 30-pin, chip CP2102/CH340 | 45.000 – 65.000 | Beli yang sudah ada breakout header, mudah cari di marketplace lokal |
| 2 | MPU6050 module | Breakout GY-521 | 8.000 – 15.000 | Sudah include regulator 3.3V & pull-up I2C |
| 3 | LED RGB common-anode/cathode | 5mm atau modul WS2812 (opsional upgrade) | 2.000 – 5.000 | Kalau pakai WS2812, hanya butuh 1 pin data, tapi kode perlu diubah (lihat catatan di bawah) |
| 4 | Resistor 220Ω x3 | Untuk tiap kaki LED (kalau bukan modul) | 1.000 | Skip jika pakai modul RGB dengan resistor bawaan |
| 5 | Baterai LiPo 3.7V | 500–1000mAh, konektor JST-PH 2 pin | 25.000 – 45.000 | Sesuaikan ukuran dengan casing tongkat |
| 6 | Modul charger TP4056 | Dengan proteksi (versi ada chip DW01+FS8205) | 3.000 – 6.000 | WAJIB pilih versi dengan proteksi over-discharge/short-circuit |
| 7 | Switch ON/OFF slide/tactile | Mini slide switch | 1.000 – 3.000 | Pasang antara baterai dan ESP32/TP4056 output |
| 8 | Push button (opsional) | Tactile 6x6mm | 500 | Untuk trigger manual/mode training, bisa juga pakai tombol BOOT bawaan ESP32 |
| 9 | Kabel jumper female-female & male-female | Secukupnya | 5.000 | Untuk wiring prototipe di breadboard |
| 10 | Breadboard mini (opsional, untuk prototyping) | 400/800 titik | 8.000 – 15.000 | Bisa dilepas setelah pindah ke solder permanen |
| 11 | Casing/tongkat | Lihat docs/ENCLOSURE.md | Variatif | PVC pipe, 3D print, atau tongkat kayu bekas |
| 12 | OLED SSD1306 0.96" I2C | 128x64, alamat default 0x3C | 15.000 – 25.000 | Dipakai untuk tampilkan nama gesture & respons teks dari AI lokal |

**Total estimasi (dengan OLED, tanpa casing 3D print):** sekitar **Rp 115.000 – 185.000**

## Catatan Pemilihan Komponen

- **ESP32 DevKit V1** dipilih karena: dual-core, WiFi built-in, cukup RAM/flash untuk model TFLite Micro kecil hasil Edge Impulse, dan harga jauh lebih murah dari Arduino Nano 33 BLE Sense (board asli di Google Codelab).
- **MPU6050** adalah alternatif termurah dari sensor IMU yang dipakai Google (yang aslinya pakai sensor built-in Arduino Nano 33 BLE Sense). Akurasi cukup untuk gesture classification.
- Kalau nanti mau upgrade ke **WS2812 (NeoPixel)** hanya butuh 1 GPIO data + resistor 300-500Ω di jalur data + kapasitor 100-1000uF di power — tapi kode `led_effects.cpp` perlu diganti untuk pakai library `Adafruit_NeoPixel` alih-alih `analogWrite`.
- **TP4056 dengan proteksi** itu wajib, bukan opsional — versi tanpa proteksi bisa merusak baterai LiPo kalau over-discharge.
