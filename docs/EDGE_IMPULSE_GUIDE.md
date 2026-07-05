# Training Model Gesture dengan Edge Impulse

Firmware di repo ini default jalan pakai **fallback threshold sederhana** (`USE_EDGE_IMPULSE_MODEL 0`
di `gesture_classifier.h`) supaya kamu bisa test hardware dulu tanpa nunggu training model. Setelah
model Edge Impulse siap, tinggal flip flag itu ke `1`.

## 1. Persiapan akun & project
1. Daftar gratis di https://studio.edgeimpulse.com
2. Buat project baru, pilih tipe **"Accelerometer data"** (walau kita juga pakai gyro, EI tetap
   menghandle multi-axis time-series dengan baik).

## 2. Kumpulkan data training

Cara termudah: **pakai serial output langsung dari ESP32** sebagai data source custom, atau pakai
**Edge Impulse Data Forwarder** (lebih gampang karena mereka yang urus format & upload).

### Opsi A — Edge Impulse Data Forwarder (rekomendasi)
1. Install CLI: `npm install -g edge-impulse-cli`
2. Buat sketch collector sementara (terpisah dari `magic_wand.ino`) yang mencetak sample ke Serial
   dengan format CSV per baris: `ax,ay,az,gx,gy,gz` pada baud rate `115200`, frekuensi tetap
   (contoh 100Hz, sesuai `SAMPLE_RATE_HZ` di `config.h`). Kamu bisa reuse `Mpu6050Sensor` yang sudah
   ada — tinggal `Serial.printf("%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n", ...)` tiap sample.
3. Jalankan: `edge-impulse-data-forwarder` lalu login, pilih project, dan device akan otomatis
   terhubung. Data akan langsung masuk ke dashboard Edge Impulse "Data acquisition".

### Opsi B — Upload manual file CSV
Kalau forwarder susah setup, kamu bisa capture data ke Serial Monitor, copy-paste ke file `.csv`,
lalu upload manual di tab **Data acquisition > Upload data** di Edge Impulse Studio.

### Berapa banyak data yang dibutuhkan?
- Minimal **~30-50 sample per gesture** (tiap sample = 1 jendela ~1 detik gerakan penuh).
- Pastikan variasi: beberapa dari kamu sendiri melakukan gesture dengan kecepatan/sudut sedikit
  berbeda tiap kali, supaya model tidak overfit ke 1 gaya gerakan saja.
- Tambahkan juga kelas **"idle"/"noise"** — data saat wand didiamkan atau digerakkan random tanpa
  maksud gesture tertentu. Ini penting supaya model tidak salah trigger terus-menerus.
- Split otomatis Edge Impulse: 80% train / 20% test cukup untuk mulai.

Label yang dipakai di firmware ini (harus persis sama, lowercase):
- `lumos`
- `wave`
- `circle`
- (opsional) `idle` / `noise`

## 3. Buat Impulse (pipeline pemrosesan)

1. Tab **Create impulse**:
   - **Window size**: `1000 ms` (harus SAMA dengan `GESTURE_WINDOW_MS` di `config.h`)
   - **Window increase**: `500 ms` (atau sesuai preferensi, untuk augmentasi data)
   - **Frequency**: `100 Hz` (harus SAMA dengan `SAMPLE_RATE_HZ` di `config.h`)
2. Tambah processing block: **Spectral Analysis** (cocok untuk data motion/IMU).
3. Tambah learning block: **Classification (Keras)**.
4. Klik **Save Impulse**.

## 4. Generate features
Buka tab **Spectral features**, klik **Generate features** — Edge Impulse akan proses semua data
training jadi fitur numerik dan tampilkan visualisasi cluster antar kelas (kalau kelas kepisah jelas
di grafik, tandanya gesture cukup berbeda satu sama lain).

## 5. Training
1. Buka tab **Classifier (NN)**.
2. Pakai setting default dulu (epoch ~30-100, learning rate default).
3. Pilih **quantized (int8)** di opsi "select a model" untuk ukuran model paling kecil & cepat di
   ESP32 — penting karena RAM ESP32 terbatas.
4. Klik **Start training**, tunggu selesai, cek **accuracy** dan **confusion matrix**. Target
   accuracy >90% untuk gesture yang jelas berbeda gerakannya.

## 6. Testing
Buka tab **Model testing**, jalankan test terhadap 20% data yang di-hold-out tadi. Kalau akurasi
test jauh lebih rendah dari training accuracy, kemungkinan overfitting — tambah variasi data.

## 7. Deploy ke Arduino library
1. Buka tab **Deployment**.
2. Pilih target **Arduino library**.
3. Centang **EON Compiler** (opsional, biasanya bikin model lebih kecil/cepat).
4. Klik **Build** — akan download file `.zip`.
5. Di Arduino IDE: **Sketch > Include Library > Add .ZIP Library...** lalu pilih file tadi.
6. Buka contoh sketch bawaan (`File > Examples > <nama_project>_inferencing > ...`) untuk lihat
   nama header yang benar, biasanya `<nama_project>_inferencing.h`.

## 8. Integrasi ke firmware repo ini
1. Buka `firmware/magic_wand/gesture_classifier.cpp`, ganti baris:
   ```cpp
   #include <your_project_inferencing.h>
   ```
   dengan nama header asli dari project Edge Impulse kamu.
2. Buka `firmware/magic_wand/gesture_classifier.h`, ubah:
   ```cpp
   #define USE_EDGE_IMPULSE_MODEL 0
   ```
   menjadi:
   ```cpp
   #define USE_EDGE_IMPULSE_MODEL 1
   ```
3. Pastikan urutan label di `classifyEdgeImpulse()` (`lumos`, `wave`, `circle`) sesuai persis dengan
   nama label yang kamu pakai saat training di Edge Impulse Studio.
4. Compile & upload. Cek Serial Monitor — kalau ada error compile terkait memori penuh
   ("region overflowed"), coba kurangi ukuran model (lower epoch/complexity) atau pakai
   `EON Compiler` saat deploy.

## Tips supaya nggak salah
- **Konsistensi window & frequency** antara `config.h` firmware dan setting Edge Impulse itu WAJIB
  sama — kalau beda, klasifikasi akan ngaco walau model "terlihat" berhasil training.
- Jangan lupa test langsung dengan device fisik (bukan cuma di dashboard EI) — noise sensor asli
  kadang beda dengan data yang di-collect lewat forwarder karena timing/jitter software serial.
- Kalau model terlalu besar untuk ESP32 (RAM error saat compile), turunkan kompleksitas Spectral
  Analysis block (kurangi jumlah filter/FFT length) sebelum training ulang.
