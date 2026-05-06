# STM32F407 RC522 RFID Güvenli Okuma/Yazma Sistemi

## 📌 Proje Açıklaması

Bu proje, **STM32F407G-DISC1** geliştirme kartı üzerinde **RC522 RFID modülü** kullanarak MiFare Classic kartlarından:
- **UID okuma**
- **Şifreli veri yazma/okuma** (MiFare Classic şifrelemesi + AES-128)
- **Kart doğrulama**
- **UART üzerinden PC iletişimi**

işlemlerini gerçekleştirir.

---

## 🎯 Özellikler

✅ RC522 RFID okuyucu desteği (13.56 MHz)  
✅ MiFare Classic kartı desteği  
✅ 6 baytlık MiFare Key A ile kimlik doğrulama  
✅ AES-128-CBC şifreleme  
✅ CRC-16 hata kontrolü  
✅ UART debug mesajları (115200 baud)  
✅ Güvenli veri depolama (Ad, Soyad, Bakiye)  
✅ LED göstergeleri (Okuma, Yazma, Hata)  

---

## 🔌 Donanım Bağlantıları

### RC522 → STM32F407G-DISC1

| RC522 Pini | STM32 Pini | Açıklama |
|-----------|-----------|----------|
| VCC | 3.3V | Güç kaynağı |
| GND | GND | Toprak |
| RST | PB0 | Reset |
| NSS/CS | PA4 | Chip Select (SPI) |
| MOSI | PA7 | Master Out Slave In (SPI) |
| MISO | PA6 | Master In Slave Out (SPI) |
| SCK | PA5 | Serial Clock (SPI) |
| IRQ | PB1 | Kesinti (opsiyonel) |

### UART Bağlantısı (PC Debug)

| STM32 | Bağlantı |
|-------|----------|
| PB10 (TX) | USB-TTL TX |
| PB11 (RX) | USB-TTL RX |
| GND | USB-TTL GND |

### Devre Şeması Kapasitörleri

```
RC522 VCC → 100nF kapasitör → GND
STM32 VCC → 100nF kapasitör → GND
```

---

## 📂 Proje Yapısı

```
STM32F407_RC522_RFID_Secure/
├── README.md
├── .gitignore
├── Firmware/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── config.h
│   │   ├── rc522.h
│   │   ├── card_operations.h
│   │   ├── uart_debug.h
│   │   └── aes.h
│   └── Src/
│       ├── main.c
│       ├── rc522.c
│       ├── card_operations.c
│       ├── uart_debug.c
│       └── aes.c
├── Documentation/
│   ├── RC522_Hardware_Setup.md
│   ├── MiFare_Classic_Protocol.md
│   └── Security_Implementation.md
└── Examples/
    └── basic_read_write_example.c
```

---

## 🚀 Kurulum Adımları

### 1. STM32CubeMX Projesi Oluşturun

```
1. STM32CubeMX → New Project
2. MCU Seçin: STM32F407IGHx
3. Periferalleri Yapılandırın:
   - Sistem Saati: 168 MHz (PLL)
   - SPI1: Master, 8 MHz, Full-duplex
   - UART3: 115200 baud, 8N1
   - GPIO Çıkışları: PA4, PB0, PB1
4. Kod üretme (Generate Code)
```

### 2. Proje Dosyalarını Kopyalayın

```bash
# STM32CubeIDE Workspace
Firmware/Inc/ → Proje/Inc/
Firmware/Src/ → Proje/Src/
```

### 3. STM32CubeIDE'de Derleyin

```bash
Project → Build Project
```

### 4. STM32F407'ye Programlayın

```bash
Run → Run As → STM32 C/C++ Application
```

---

## 💾 Veri Formatı (Kartta Depolanacak)

### Blok 16-18 (48 bayt)

```
Blok 16: İsim (16 bayt) - Şifreli
Blok 17: Soyisim (16 bayt) - Şifreli
Blok 18: Bakiye (16 bayt) - Şifreli + CRC

Format: [Veri (14 bayt)] [CRC-16 (2 bayt)]
```

### Şifreleme

```
Algoritma: AES-128-CBC
Anahtar: config.h içinde tanımlandı
IV: UID'nin ilk 16 baytı
```

---

## 🔐 Güvenlik Yapılandırması

### MiFare Classic Anahtarı Değiştirme

**Dosya: `Firmware/Inc/config.h`**

```c
#define MIFARE_CUSTOM_KEY {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5}
```

**⚠️ ÖNEMLİ:** Anahtarı güvenli yerde saklayın!

---

## 📊 Veri Akışı

```
┌─────────────────────┐
│  Kart Algılanması   │
└──────────┬──────────┘
           ↓
┌─────────────────────┐
│  UID Okunması       │ (4 bayt)
└──────────┬──────────┘
           ↓
┌─────────────────────┐
│  MiFare Key Doğrulaması│
└──────────┬──────────┘
           ↓
┌─────────────────────┐
│  Blok 16-18 Okunması│
└──────────┬──────────┘
           ↓
┌─────────────────────┐
│  AES-128 Deşifresi  │
└──────────┬──────────┘
           ↓
┌─────────────────────┐
│  CRC Doğrulaması    │
└──────────┬──────────┘
           ↓
┌─────────────────────┐
│  Veri PC'ye Gönderilmesi│
└─────────────────────┘
```

---

## ✅ Test Kontrol Listesi

- [ ] Donanım bağlantıları kontrol ettim
- [ ] STM32CubeMX projesi oluşturdum
- [ ] Kodları Src/ ve Inc/ klasörlerine kopyaladım
- [ ] Proje başarıyla derlenmiş
- [ ] STM32F407'ye programlandı
- [ ] UART terminal açtım (115200 baud)
- [ ] Kart cihaza yaklaştırıldı
- [ ] UID başarıyla okundu
- [ ] Veriler şifreli olarak yazıldı
- [ ] Şifreli veriler okundu
- [ ] Deşifresi başarılı oldu

---

## 🔧 Sorun Giderme

### ❌ RC522 Algılanmıyor

**Çözüm:**
1. VCC/GND bağlantılarını kontrol et
2. 100nF kapasitörü VCC-GND arasına bağla
3. PA4, PA5, PA6, PA7 SPI bağlantılarını kontrol et
4. PB0 (RST) bağlantısını kontrol et

### ❌ UART Mesajları Görülmüyor

**Çözüm:**
1. Baud rate'i 115200 olarak ayarla
2. USB-TTL modülünün sürücüsünü yükle
3. PB10 (TX) ve PB11 (RX) doğru bağlı mı kontrol et

### ❌ Kart Okunamıyor

**Çözüm:**
1. MiFare Classic kartı mı?
2. Anahtar doğru mu? (config.h)
3. Kartın magnet hasarı olabilir mi?

### ❌ AES Şifre Çözme Başarısız

**Çözüm:**
1. AES anahtarı config.h ile eşleşiyor mu?
2. IV (UID) doğru mu?
3. CRC doğrulaması başarılı mı?

---

## 📚 İlgili Kaynaklar

- [RC522 Datasheet](https://www.nxp.com/docs/en/data-sheet/RC522.pdf)
- [MiFare Classic Protokol](https://www.nxp.com/docs/en/application-note/AN10833.pdf)
- [STM32F407 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00031020-stm32f405-415-stm32f407-417-stm32f427-437-and-stm32f429-439-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)

---

## 📝 Lisans

MIT License - Özgürce kullanabilirsiniz

## 👨‍💻 Yazar

Mustafa Tekin - 2026

---

**Sorularınız mı var? GitHub Issues'de soru sorun!** 🚀
