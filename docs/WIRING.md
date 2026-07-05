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
        LED  R      ───┤ GPIO25 ──[220Ω]── │──> kaki R LED RGB
        LED  G      ───┤ GPIO26 ──[220Ω]── │──> kaki G LED RGB
        LED  B      ───┤ GPIO33 ──[220Ω]── │──> kaki B LED RGB
        LED  GND/COM ──┤ GND (common cathode) atau 3V3 (common anode, lihat catatan)
                       │                   │
        Tombol BOOT ───┤ GPIO0 (bawaan board, tidak perlu wiring tambahan)
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

## Diagram visual (disarankan)
Untuk diagram visual grafis (bukan teks), gunakan [Wokwi](https://wokwi.com) — sudah ada simulator ESP32 + MPU6050 online, gratis, dan bisa langsung simulasikan sebagian logic (I2C, LED) sebelum wiring fisik. Cari template "ESP32 MPU6050" di Wokwi sebagai starting point lalu sesuaikan pin sesuai tabel di atas.
