# Saran Casing / Enclosure

## Opsi 1 — Pipa PVC/akrilik (paling murah, tanpa 3D printer)
- Pipa PVC diameter ~20-25mm, panjang 25-30cm sebagai "badan tongkat".
- ESP32 + MPU6050 + baterai ditempatkan di bagian pegangan (lebih tebal/lebar untuk muat komponen).
- LED RGB dipasang di ujung, dengan **diffuser** dari bola pingpong dipotong setengah atau
  potongan tabung akrilik susu (frosted) supaya cahaya menyebar rata, bukan titik terang tajam.
- Tutup ujung pegangan pakai dop PVC yang dibor untuk switch ON/OFF dan port USB (akses charging
  tanpa bongkar casing).

## Opsi 2 — 3D print (kalau ada akses printer)
- Desain 2 bagian: **grip** (menampung ESP32, baterai, TP4056, switch) dan **shaft** (menyalurkan
  kabel LED ke ujung).
- Material: PETG lebih baik dari PLA untuk ketahanan (PLA getas kalau dipakai gesture cepat/wave).
- Sisakan slot akses untuk port USB-C/micro-USB charging tanpa bongkar total.
- Cari template dasar "wand enclosure" atau "pen case" di Thingiverse/Printables sebagai starting
  point, lalu sesuaikan dimensi internal ke ukuran board ESP32 DevKit V1 (~52x28mm) + baterai LiPo.

## Opsi 3 — Tongkat kayu/akrilik bekas (paling cepat, prototyping)
- Pakai tongkat kayu bulat (dowel) yang dilubangi tengahnya, atau tongkat harry-potter-style yang
  dijual online sebagai basis, lalu tempel komponen di bagian pegangan pakai hot glue/velcro.
- Cocok untuk iterasi cepat sebelum commit ke desain casing permanen.

## Pertimbangan umum semua opsi
- **Sirkulasi kabel**: MPU6050 idealnya ditempatkan sedekat mungkin ke ujung gerak (tip) supaya
  sensitivitas gesture lebih baik, tapi ini menambah panjang kabel dari grip. Kompromi umum: taruh
  MPU6050 di grip juga (dekat tangan) karena gesture berbasis rotasi/pergerakan tangan tetap
  terbaca dengan baik dari situ, dan ini menyederhanakan wiring.
- **Ventilasi/panas**: ESP32 + WiFi aktif bisa hangat saat request AI API; casing tertutup rapat
  tanpa 3D print (misal PVC solid) sebaiknya punya sedikit celah ventilasi.
- **Akses charging**: selalu sisakan akses ke port USB TP4056 tanpa perlu bongkar casing.
- **Berat & keseimbangan**: taruh baterai (komponen terberat) sedekat mungkin ke tangan (grip) demi
  keseimbangan tongkat saat dipakai gesture cepat.
