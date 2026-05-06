/*
 * card_operations.c
 * RFID Kart İşlemleri - AES-128 Şifreleme ile Güvenli Okuma/Yazma
 * STM32F407G-DISC1 için
 */

#include "card_operations.h"
#include "rc522.h"
#include "uart_debug.h"
#include "config.h"
#include <string.h>
#include <stdio.h>

// AES Şifreleme Anahtarı
const uint8_t aes_key[16] = AES_ENCRYPTION_KEY;

// MiFare Classic Anahtarı (Key A)
const uint8_t mifare_key[6] = {MIFARE_KEY_A};

/*
 * CRC-16 Hesapla
 * data: Veri buffer
 * len: Veri uzunluğu
 * return: CRC-16 değeri
 */
uint16_t Calculate_CRC16(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    uint16_t i, j;
    
    for (i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc = crc >> 1;
            }
        }
    }
    
    return crc;
}

/*
 * AES-128-ECB Şifreleme (Basit versiyon)
 * Gerçek uygulamada mbedTLS veya CryptoLib kullanılmalı
 */
void AES128_Encrypt(uint8_t *plaintext, uint8_t *ciphertext, const uint8_t *key)
{
    // NOT: Bu basit bir placeholder. Gerçek AES için:
    // - mbedTLS: https://github.com/Mbed-TLS/mbedtls
    // - ARM CMSIS-DSP
    // - Veya STM32CubeMX'in Hardware Crypto kullanın
    
    // Şimdilik XOR tabanlı basit şifreleme (DEMO amaçlı)
    uint8_t i;
    for (i = 0; i < 16; i++)
    {
        ciphertext[i] = plaintext[i] ^ key[i % 16];
    }
    
    Debug_Printf("AES Sifreleme Yapildi\r\n");
}

/*
 * AES-128-ECB Deşifreleme
 */
void AES128_Decrypt(uint8_t *ciphertext, uint8_t *plaintext, const uint8_t *key)
{
    // Basit XOR deşifreleme (DEMO amaçlı)
    uint8_t i;
    for (i = 0; i < 16; i++)
    {
        plaintext[i] = ciphertext[i] ^ key[i % 16];
    }
    
    Debug_Printf("AES Desifreleme Yapildi\r\n");
}

/*
 * Kart Verilerini Yapılandır
 * card_data: Kart veri yapısı
 * name: İsim
 * surname: Soyisim
 * balance: Bakiye (uint32_t)
 */
void Card_SetData(CardData_t *card_data, const char *name, const char *surname, uint32_t balance)
{
    memset(card_data, 0, sizeof(CardData_t));
    
    // İsim (14 bayt)
    strncpy((char *)card_data->name, name, 14);
    
    // Soyisim (14 bayt)
    strncpy((char *)card_data->surname, surname, 14);
    
    // Bakiye (4 bayt)
    card_data->balance = balance;
    
    Debug_Printf("Kart Verisi Olusturuldu: %s %s - %lu TL\r\n", 
                 name, surname, balance);
}

/*
 * Kart Verilerini Yazdır
 * block_data: Şifreli blok verisi (16 bayt)
 * aes_key: AES anahtarı
 */
void Card_PrintData(uint8_t *block_data)
{
    CardData_t *card_data;
    uint8_t plaintext[16];
    uint16_t crc_calc, crc_stored;
    
    // Deşifre
    AES128_Decrypt(block_data, plaintext, aes_key);
    
    // Cast
    card_data = (CardData_t *)plaintext;
    
    // CRC Kontrol
    crc_calc = Calculate_CRC16(plaintext, 14);
    crc_stored = (plaintext[14] << 8) | plaintext[15];
    
    Debug_Printf("=== KART VERİSİ ===\r\n");
    Debug_Printf("İsim: %s\r\n", card_data->name);
    Debug_Printf("Soyisim: %s\r\n", card_data->surname);
    Debug_Printf("Bakiye: %lu TL\r\n", card_data->balance);
    Debug_Printf("CRC Doğru: %s\r\n", (crc_calc == crc_stored) ? "EVET" : "HAYIR");
    Debug_Printf("===================\r\n");
}

/*
 * Şifreli Blok Hazırla
 * name: İsim
 * surname: Soyisim
 * balance: Bakiye
 * block_data: Çıkış - Şifreli 16 baytlık blok
 */
void Card_PrepareEncryptedBlock(const char *name, const char *surname, 
                                 uint32_t balance, uint8_t *block_data)
{
    CardData_t card_data;
    uint8_t plaintext[16];
    uint16_t crc;
    uint8_t i;
    
    // Veri Yapısını Doldur
    memset(&card_data, 0, sizeof(CardData_t));
    strncpy((char *)card_data.name, name, 14);
    strncpy((char *)card_data.surname, surname, 14);
    card_data.balance = balance;
    
    // Plaintext oluştur
    memcpy(plaintext, &card_data, 14);
    
    // CRC Hesapla
    crc = Calculate_CRC16(plaintext, 14);
    plaintext[14] = (crc >> 8) & 0xFF;
    plaintext[15] = crc & 0xFF;
    
    // Şifre
    AES128_Encrypt(plaintext, block_data, aes_key);
    
    Debug_Printf("Block Hazirland: ");
    for (i = 0; i < 16; i++)
    {
        Debug_Printf("%02X ", block_data[i]);
    }
    Debug_Printf("\r\n");
}

/*
 * Kartı İlk Kurulumla Yaz
 * name: İsim
 * surname: Soyisim
 * balance: Bakiye
 * return: 0 = Başarılı, 1 = Hata
 */
uint8_t Card_WriteInitial(const char *name, const char *surname, uint32_t balance)
{
    uint8_t status;
    uint8_t uid[4];
    uint8_t block_data[16];
    uint8_t sector = 4;  // Sektör 4 (Bloklar 16-19)
    
    Debug_Printf("\r\n=== KART YAZMA ISLEMI BASLIYOR ===\r\n");
    
    // 1. UID Oku
    Debug_Printf("1. UID Okunuyor...\r\n");
    status = RC522_ReadUID(uid);
    if (status != 0)
    {
        Debug_Printf("HATA: UID Okunamadi\r\n");
        return 1;
    }
    
    memcpy(rc522_uid, uid, 4);
    
    // 2. Doğrulama
    Debug_Printf("2. Dogrulama Yapiliyor...\r\n");
    status = RC522_Authenticate(sector, (uint8_t *)mifare_key);
    if (status != 0)
    {
        Debug_Printf("HATA: Dogrulama Basarisiz\r\n");
        return 1;
    }
    
    // 3. İsim Yaz (Blok 16)
    Debug_Printf("3. Isim Yaziliyor (Blok 16)...\r\n");
    Card_PrepareEncryptedBlock(name, "", 0, block_data);
    status = RC522_WriteBlock(16, block_data);
    if (status != 0)
    {
        Debug_Printf("HATA: Blok 16 Yazma Basarisiz\r\n");
        return 1;
    }
    
    // 4. Soyisim Yaz (Blok 17)
    Debug_Printf("4. Soyisim Yaziliyor (Blok 17)...\r\n");
    Card_PrepareEncryptedBlock("", surname, 0, block_data);
    status = RC522_WriteBlock(17, block_data);
    if (status != 0)
    {
        Debug_Printf("HATA: Blok 17 Yazma Basarisiz\r\n");
        return 1;
    }
    
    // 5. Bakiye Yaz (Blok 18)
    Debug_Printf("5. Bakiye Yaziliyor (Blok 18)...\r\n");
    Card_PrepareEncryptedBlock("", "", balance, block_data);
    status = RC522_WriteBlock(18, block_data);
    if (status != 0)
    {
        Debug_Printf("HATA: Blok 18 Yazma Basarisiz\r\n");
        return 1;
    }
    
    // 6. Halt
    RC522_Halt();
    
    Debug_Printf("=== KART YAZMA ISLEMI TAMAMLANDI ===\r\n\r\n");
    
    return 0;
}

/*
 * Karttan Veri Oku
 * uid: UID çıkışı (4 bayt)
 * return: 0 = Başarılı, 1 = Hata
 */
uint8_t Card_ReadAll(uint8_t *uid)
{
    uint8_t status;
    uint8_t block_data[16];
    uint8_t sector = 4;
    
    Debug_Printf("\r\n=== KART OKUMA ISLEMI BASLIYOR ===\r\n");
    
    // 1. UID Oku
    Debug_Printf("1. UID Okunuyor...\r\n");
    status = RC522_ReadUID(uid);
    if (status != 0)
    {
        Debug_Printf("HATA: UID Okunamadi\r\n");
        return 1;
    }
    
    memcpy(rc522_uid, uid, 4);
    
    // 2. Doğrulama
    Debug_Printf("2. Dogrulama Yapiliyor...\r\n");
    status = RC522_Authenticate(sector, (uint8_t *)mifare_key);
    if (status != 0)
    {
        Debug_Printf("HATA: Dogrulama Basarisiz\r\n");
        return 1;
    }
    
    // 3. Blok 16 - İsim Oku
    Debug_Printf("3. Isim Okunuyor (Blok 16)...\r\n");
    status = RC522_ReadBlock(16, block_data);
    if (status != 0)
    {
        Debug_Printf("HATA: Blok 16 Okuma Basarisiz\r\n");
        return 1;
    }
    Card_PrintData(block_data);
    
    // 4. Blok 17 - Soyisim Oku
    Debug_Printf("4. Soyisim Okunuyor (Blok 17)...\r\n");
    status = RC522_ReadBlock(17, block_data);
    if (status != 0)
    {
        Debug_Printf("HATA: Blok 17 Okuma Basarisiz\r\n");
        return 1;
    }
    Card_PrintData(block_data);
    
    // 5. Blok 18 - Bakiye Oku
    Debug_Printf("5. Bakiye Okunuyor (Blok 18)...\r\n");
    status = RC522_ReadBlock(18, block_data);
    if (status != 0)
    {
        Debug_Printf("HATA: Blok 18 Okuma Basarisiz\r\n");
        return 1;
    }
    Card_PrintData(block_data);
    
    // 6. Halt
    RC522_Halt();
    
    Debug_Printf("=== KART OKUMA ISLEMI TAMAMLANDI ===\r\n\r\n");
    
    return 0;
}

/*
 * Bakiye Güncelle
 * new_balance: Yeni bakiye
 * return: 0 = Başarılı, 1 = Hata
 */
uint8_t Card_UpdateBalance(uint32_t new_balance)
{
    uint8_t status;
    uint8_t uid[4];
    uint8_t block_data[16];
    uint8_t sector = 4;
    
    Debug_Printf("\r\n=== BAKIYE GUNCELLEME ISLEMI BASLIYOR ===\r\n");
    
    // 1. UID Oku
    Debug_Printf("1. UID Okunuyor...\r\n");
    status = RC522_ReadUID(uid);
    if (status != 0)
    {
        Debug_Printf("HATA: UID Okunamadi\r\n");
        return 1;
    }
    
    memcpy(rc522_uid, uid, 4);
    
    // 2. Doğrulama
    Debug_Printf("2. Dogrulama Yapiliyor...\r\n");
    status = RC522_Authenticate(sector, (uint8_t *)mifare_key);
    if (status != 0)
    {
        Debug_Printf("HATA: Dogrulama Basarisiz\r\n");
        return 1;
    }
    
    // 3. Yeni Bakiye Yaz (Blok 18)
    Debug_Printf("3. Yeni Bakiye Yaziliyor (Blok 18): %lu TL\r\n", new_balance);
    Card_PrepareEncryptedBlock("", "", new_balance, block_data);
    status = RC522_WriteBlock(18, block_data);
    if (status != 0)
    {
        Debug_Printf("HATA: Blok 18 Yazma Basarisiz\r\n");
        return 1;
    }
    
    // 4. Halt
    RC522_Halt();
    
    Debug_Printf("=== BAKIYE GUNCELLEME TAMAMLANDI ===\r\n\r\n");
    
    return 0;
}

/*
 * Bakiye Azalt
 * amount: Azaltılacak miktar
 * return: 0 = Başarılı, 1 = Hata
 */
uint8_t Card_DeductBalance(uint32_t amount)
{
    uint8_t status;
    uint8_t uid[4];
    uint8_t block_data[16];
    uint8_t plaintext[16];
    CardData_t *card_data;
    uint32_t current_balance;
    uint8_t sector = 4;
    
    Debug_Printf("\r\n=== BAKIYE CIKMA ISLEMI BASLIYOR ===\r\n");
    
    // 1. UID Oku
    Debug_Printf("1. UID Okunuyor...\r\n");
    status = RC522_ReadUID(uid);
    if (status != 0)
    {
        Debug_Printf("HATA: UID Okunamadi\r\n");
        return 1;
    }
    
    memcpy(rc522_uid, uid, 4);
    
    // 2. Doğrulama
    Debug_Printf("2. Dogrulama Yapiliyor...\r\n");
    status = RC522_Authenticate(sector, (uint8_t *)mifare_key);
    if (status != 0)
    {
        Debug_Printf("HATA: Dogrulama Basarisiz\r\n");
        return 1;
    }
    
    // 3. Mevcut Bakiye Oku (Blok 18)
    Debug_Printf("3. Mevcut Bakiye Okunuyor...\r\n");
    status = RC522_ReadBlock(18, block_data);
    if (status != 0)
    {
        Debug_Printf("HATA: Blok 18 Okuma Basarisiz\r\n");
        return 1;
    }
    
    // Deşifre
    AES128_Decrypt(block_data, plaintext, aes_key);
    card_data = (CardData_t *)plaintext;
    current_balance = card_data->balance;
    
    Debug_Printf("Mevcut Bakiye: %lu TL\r\n", current_balance);
    
    // Bakiye Kontrol
    if (current_balance < amount)
    {
        Debug_Printf("HATA: Yeterli Bakiye Yok! (%lu < %lu)\r\n", current_balance, amount);
        RC522_Halt();
        return 1;
    }
    
    // Yeni Bakiye
    uint32_t new_balance = current_balance - amount;
    
    // 4. Yeni Bakiye Yaz
    Debug_Printf("4. Yeni Bakiye Yaziliyor: %lu TL\r\n", new_balance);
    Card_PrepareEncryptedBlock("", "", new_balance, block_data);
    status = RC522_WriteBlock(18, block_data);
    if (status != 0)
    {
        Debug_Printf("HATA: Blok 18 Yazma Basarisiz\r\n");
        return 1;
    }
    
    // 5. Halt
    RC522_Halt();
    
    Debug_Printf("=== BAKIYE CIKMA ISLEMI TAMAMLANDI ===\r\n");
    Debug_Printf("Cikilan: %lu TL\r\n", amount);
    Debug_Printf("Yeni Bakiye: %lu TL\r\n\r\n", new_balance);
    
    return 0;
}

/*
 * Bakiye Ekle
 * amount: Eklenecek miktar
 * return: 0 = Başarılı, 1 = Hata
 */
uint8_t Card_AddBalance(uint32_t amount)
{
    uint8_t status;
    uint8_t uid[4];
    uint8_t block_data[16];
    uint8_t plaintext[16];
    CardData_t *card_data;
    uint32_t current_balance;
    uint32_t new_balance;
    uint8_t sector = 4;
    
    Debug_Printf("\r\n=== BAKIYE YATIRMA ISLEMI BASLIYOR ===\r\n");
    
    // 1. UID Oku
    Debug_Printf("1. UID Okunuyor...\r\n");
    status = RC522_ReadUID(uid);
    if (status != 0)
    {
        Debug_Printf("HATA: UID Okunamadi\r\n");
        return 1;
    }
    
    memcpy(rc522_uid, uid, 4);
    
    // 2. Doğrulama
    Debug_Printf("2. Dogrulama Yapiliyor...\r\n");
    status = RC522_Authenticate(sector, (uint8_t *)mifare_key);
    if (status != 0)
    {
        Debug_Printf("HATA: Dogrulama Basarisiz\r\n");
        return 1;
    }
    
    // 3. Mevcut Bakiye Oku
    Debug_Printf("3. Mevcut Bakiye Okunuyor...\r\n");
    status = RC522_ReadBlock(18, block_data);
    if (status != 0)
    {
        Debug_Printf("HATA: Blok 18 Okuma Basarisiz\r\n");
        return 1;
    }
    
    // Deşifre
    AES128_Decrypt(block_data, plaintext, aes_key);
    card_data = (CardData_t *)plaintext;
    current_balance = card_data->balance;
    
    Debug_Printf("Mevcut Bakiye: %lu TL\r\n", current_balance);
    
    // Yeni Bakiye
    new_balance = current_balance + amount;
    
    // 4. Yeni Bakiye Yaz
    Debug_Printf("4. Yeni Bakiye Yaziliyor: %lu TL\r\n", new_balance);
    Card_PrepareEncryptedBlock("", "", new_balance, block_data);
    status = RC522_WriteBlock(18, block_data);
    if (status != 0)
    {
        Debug_Printf("HATA: Blok 18 Yazma Basarisiz\r\n");
        return 1;
    }
    
    // 5. Halt
    RC522_Halt();
    
    Debug_Printf("=== BAKIYE YATIRMA ISLEMI TAMAMLANDI ===\r\n");
    Debug_Printf("Yatirilan: %lu TL\r\n", amount);
    Debug_Printf("Yeni Bakiye: %lu TL\r\n\r\n", new_balance);
    
    return 0;
}
