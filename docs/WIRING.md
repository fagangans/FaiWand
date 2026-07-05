# Wiring Diagram — AI Magic Wand (ESP32 DevKit V1)

## Diagram teks (skematik sederhana)

```
                          ESP32 DevKit V1
                       ┌───────────────────┐
        MPU6050 SCL ───┤ GPIO22 (SCL)      │
        MPU6050 SDA ───┤ GPIO21 (SDA)      │
        MPU6050 INT ───┤ GPIO27            │  (wake-on-motion, deep sleep)
        MPU6050 VCC ───┤ 3V3               │
        MPU6050 GND ───┤ GND               │
                       │                   │
        OLED   SCL  ───┤ GPIO22 (SCL, bus sama dgn MPU6050) │
        OLED   SDA  ───┤ GPIO21 (SDA, bus sama dgn MPU6050) │
        OLED   VCC  ───┤ 3V3               │
        OLED   GND  ───┤ GND               │
                       │                   │
        LED  R      ───┤ GPIO25 ──[220Ω]── │──> kaki R LED RGB
        LED  G      ───┤ GPIO26 ──[220Ω]── │──> kaki G LED RGB
        LED  B      ───┤ GPIO33 ──[220Ω]── │──> kaki B LED RGB
        LED  GND/COM ──┤ GND (common cathode) atau 3V3 (common anode, lihat catatan)
                       │                   │
        Tombol BOOT ───┤ GPIO0 (bawaan board, tidak perlu wiring tambahan)
                       │                   │
        MAX98357A BCLK ┤ GPIO14            │
        MAX98357A LRC  ┤ GPIO32            │
        MAX98357A DIN  ┤ GPIO13            │
        MAX98357A VIN  ┤ VIN (bukan 3V3 -- amp butuh daya lebih untuk speaker) │
        MAX98357A GND  ┤ GND               │
        MAX98357A GAIN ┤ (lihat catatan di bawah)                              │
                       │                   │
        Baterai LiPo ──┤ VIN / 5V (via TP4056 OUT+) │
                       │ GND (via TP4056 OUT-)       │
                       └───────────────────┘

        Baterai LiPo (JST) ──> TP4056 B+/B-
        TP4056 OUT+/OUT- ──> Switch ON/OFF ──> ESP32 VIN & GND
```

## Detail koneksi

### MPU6050 (I2C)
| MPU6050 Pin | ESP32 Pin | Keterangan |
|---|---|---|
| VCC | 3V3 | Jangan pakai 5V walau modul GY-521 punya regulator, demi konsistensi logic level I2C |
| GND | GND | |
| SCL | GPIO22 | Default I2C clock ESP32 |
| SDA | GPIO21 | Default I2C data ESP32 |
| INT | GPIO27 | Untuk wake-on-motion dari deep sleep |
| AD0 | GND (atau biarkan floating) | Set alamat I2C ke 0x68 (default) |

### OLED SSD1306 0.96" (I2C)
| OLED Pin | ESP32 Pin | Keterangan |
|---|---|---|
| VCC | 3V3 | |
| GND | GND | |
| SCL | GPIO22 | Bus I2C sama dengan MPU6050 — aman karena alamat beda (OLED 0x3C, MPU6050 0x68) |
| SDA | GPIO21 | Bus I2C sama dengan MPU6050 |

Tidak perlu resistor pull-up tambahan — modul GY-521 dan modul OLED umumnya sudah punya
pull-up on-board masing-masing; kalau layar tidak stabil/garis-garis, coba lepas salah satu
pull-up modul (cabut satu resistor 4.7kΩ di salah satu breakout) supaya tidak dobel.

### LED RGB
- Kalau pakai **LED diskrit 4-kaki (common cathode)**: kaki common ke GND, tiap kaki R/G/B ke GPIO lewat resistor 220Ω.
- Kalau **common anode**: kaki common ke 3V3, dan set `LED_ACTIVE_LOW true` di `config.h` supaya logika nyala/mati kebalik dengan benar.
- Kalau pakai **modul RGB siap pakai** (biasanya sudah ada resistor internal), tinggal sambung 3 pin data + GND/VCC sesuai datasheet modul.

### Power (Baterai LiPo + TP4056)
1. Baterai LiPo → konektor B+/B- di TP4056.
2. TP4056 OUT+ dan OUT- → switch ON/OFF → ESP32 pin **VIN** (bukan 3V3!) dan **GND**.
3. Port micro-USB/USB-C di TP4056 dipakai untuk charging — jangan sambung USB ini ke ESP32 secara bersamaan saat charging kalau switch dalam posisi ON (bisa, tapi pastikan arus cukup).
4. ESP32 DevKit V1 punya regulator 5V→3.3V on-board, jadi VIN bisa menerima ~5V dari baterai LiPo yang sudah di-boost, **ATAU** kalau baterai LiPo 3.7V langsung disambung ke VIN, pastikan board kamu punya regulator yang menerima input serendah itu (banyak board DevKit V1 punya AMS1117 yang butuh input >4.5V agar output stabil 3.3V). **Rekomendasi paling aman:** sambungkan baterai LiPo ke pin **3V3** langsung (bypass regulator) HANYA jika board mendukung, atau lebih aman lagi gunakan modul boost converter 3.7V→5V di antara TP4056 dan VIN.

> ⚠️ **Penting soal power**: Banyak DevKit V1 versi murah regulatornya (AMS1117-3.3) butuh minimum ~4.5-5V di VIN untuk output stabil 3.3V. LiPo 3.7V (single cell) yang di-drop ke ~3.0V saat hampir habis bisa membuat ESP32 brownout. Solusi paling aman & murah: tambahkan **modul boost converter kecil (MT3608, ~3.000 IDR)** antara output TP4056 dan VIN ESP32, di-set ke 5V.

### Push button (opsional, mode training/manual trigger)
Bisa pakai tombol **BOOT** bawaan board (GPIO0) — tidak perlu wiring tambahan, cukup dibaca lewat `digitalRead(PIN_BUTTON)` (active LOW).

### Speaker (I2S, MAX98357A + speaker mini)
| MAX98357A Pin | ESP32 Pin | Keterangan |
|---|---|---|
| VIN | VIN (bukan 3V3) | Amplifier butuh daya lebih besar untuk menggerakkan speaker; 3V3 regulator ESP32 tidak cukup kuat |
| GND | GND | |
| BCLK | GPIO14 | I2S bit clock |
| LRC | GPIO32 | I2S word select (L/R clock) |
| DIN | GPIO13 | I2S data in (audio digital dari ESP32 ke amp) |
| GAIN | Floating (default) atau ke GND (gain lebih tinggi) — sesuaikan dengan volume speaker yang diinginkan | |
| SD | Floating (aktif) atau ke GND lewat resistor kalau mau kontrol shutdown manual | |
| Speaker + / - | Ke speaker mini 4-8Ω | Polaritas tidak terlalu kritis untuk speaker mono kecil |

> ⚠️ **Penting**: MAX98357A disambung ke **VIN** (bukan 3V3) karena perlu daya lebih untuk
> menggerakkan speaker dengan volume cukup. Pastikan baterai/boost converter (MT3608) yang
> sudah ada mampu suplai arus tambahan ini (~500mA-1A saat volume tinggi) — kalau baterai
> kamu kecil (300-400mAh), daya tahan akan berkurang signifikan saat audio sering diputar.

### Jendela speaker di enclosure
Karena speaker perlu suara bisa keluar, enclosure gagang perlu **lubang-lubang kecil (grille)**
di dekat posisi speaker dipasang. Ini belum ada di file `.scad` bawaan — perlu ditambahkan
manual sesuai posisi speaker kamu pasang nanti.

## Diagram visual (disarankan)
Untuk diagram visual grafis (bukan teks), gunakan [Wokwi](https://wokwi.com) — sudah ada simulator ESP32 + MPU6050 online, gratis, dan bisa langsung simulasikan sebagian logic (I2C, LED) sebelum wiring fisik. Cari template "ESP32 MPU6050" di Wokwi sebagai starting point lalu sesuaikan pin sesuai tabel di atas.

## Jaringan: ESP32 <-> Server AI Lokal
ESP32 dan server AI lokal (yang menjalankan speech-to-speech/speech-to-text) harus berada di
**WiFi/LAN yang sama**. Tidak ada wiring fisik antara keduanya — komunikasi murni lewat WiFi
(HTTP). Yang perlu disiapkan:
1. Server AI lokal harus punya endpoint HTTP (default di firmware: `POST /gesture` port `5000`,
   bisa diubah di `config.h`) yang menerima JSON `{"gesture": "Wave"}` dan membalas
   `{"text": "..."}`.
2. Isi `LOCAL_AI_HOST` di `secrets.h` dengan hostname mDNS server (`namaserver.local`) atau IP
   statis-nya.
3. Kalau pakai hostname mDNS, pastikan OS server AI lokal mendukung mDNS/Bonjour
   (bawaan di macOS, perlu paket `avahi-daemon` di Linux, atau software "Bonjour" di Windows).
