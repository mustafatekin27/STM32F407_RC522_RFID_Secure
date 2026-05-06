/*
 * rc522.h
 * RC522 RFID Okuyucu - Header Dosyası
 */

#ifndef RC522_H
#define RC522_H

#include "stm32f4xx_hal.h"

// ============================================
// RC522 Register Adresleri
// ============================================

#define RC522_REG_RESERVED_00           0x00
#define RC522_REG_COMMAND               0x01
#define RC522_REG_FIFO_DATA             0x02
#define RC522_REG_FIFO_LEVEL            0x04
#define RC522_REG_FIFO_WATER_LEVEL      0x05
#define RC522_REG_IRQ_EN                0x06
#define RC522_REG_COMM_IRQ              0x07
#define RC522_REG_ERROR                 0x08
#define RC522_REG_STATUS1               0x09
#define RC522_REG_STATUS2               0x0A
#define RC522_REG_RX_CONTROL            0x0B
#define RC522_REG_RX_WAIT               0x0C
#define RC522_REG_RX_THRESHOLD          0x0D
#define RC522_REG_TX_CONTROL            0x14
#define RC522_REG_TX_ASK                0x15
#define RC522_REG_TX_SEL                0x16
#define RC522_REG_RX_SEL                0x17
#define RC522_REG_RX_GAIN               0x18
#define RC522_REG_RF_CONFIG             0x26
#define RC522_REG_GS_N                  0x27
#define RC522_REG_CW_GS_P               0x28
#define RC522_REG_MOD_GS_P              0x29
#define RC522_REG_T_MODE                0x2A
#define RC522_REG_T_PRESCALER           0x2B
#define RC522_REG_T_RELOAD_H            0x2C
#define RC522_REG_T_RELOAD_L            0x2D
#define RC522_REG_T_COUNTER_VALUE_H     0x2E
#define RC522_REG_T_COUNTER_VALUE_L     0x2F
#define RC522_REG_MODE                  0x3F
#define RC522_REG_VERSION               0x37
#define RC522_REG_ANALOGUE              0x35
#define RC522_REG_BIT_FRAMING           0x0F
#define RC522_REG_FORCE_ALIGNMENT       0x1E
#define RC522_REG_DIV1                  0x24
#define RC522_REG_DIV2                  0x25
#define RC522_REG_DIV3                  0x26

// ============================================
// RC522 Komutları
// ============================================

#define RC522_CMD_IDLE                  0x00
#define RC522_CMD_TRANSMIT              0x01
#define RC522_CMD_RECEIVE               0x02
#define RC522_CMD_TRANSCEIVE            0x0C
#define RC522_CMD_CALC_CRC              0x03
#define RC522_CMD_AUTH_A                0x60
#define RC522_CMD_AUTH_B                0x61
#define RC522_CMD_SOFT_RESET            0x0F

// ============================================
// MiFare Komutları
// ============================================

#define RC522_CMD_REQA                  0x26    // Request A
#define RC522_CMD_REQB                  0x14    // Request B
#define RC522_CMD_SELECT                0x93    // Select
#define RC522_CMD_READ                  0x30    // Read Block
#define RC522_CMD_WRITE                 0xA0    // Write Block
#define RC522_CMD_HALT                  0x50    // Halt
#define RC522_CMD_INCREMENT             0xC1    // Increment
#define RC522_CMD_DECREMENT             0xC0    // Decrement
#define RC522_CMD_RESTORE               0xC2    // Restore
#define RC522_CMD_TRANSFER              0xB0    // Transfer

// ============================================
// Fonksiyon Prototipi
// ============================================

/**
 * RC522 Register'ine Yazma
 */
void RC522_Write(uint8_t addr, uint8_t value);

/**
 * RC522 Register'den Okuma
 */
uint8_t RC522_Read(uint8_t addr);

/**
 * RC522 İnitiyalizasyon
 */
void RC522_Init(void);

/**
 * Anten Aç
 */
void RC522_AntennaOn(void);

/**
 * Anten Kapat
 */
void RC522_AntennaOff(void);

/**
 * Bit Maskesini Ayarla
 */
void RC522_SetBitMask(uint8_t reg, uint8_t mask);

/**
 * Bit Maskesini Temizle
 */
void RC522_ClearBitMask(uint8_t reg, uint8_t mask);

/**
 * RC522 Komutu Gönder
 * @param cmd: Komut
 * @param data: Komut verisi
 * @param data_len: Veri uzunluğu
 * @return: 0 = Başarılı, 1 = Hata
 */
uint8_t RC522_SendCommand(uint8_t cmd, uint8_t *data, uint8_t data_len);

/**
 * FIFO Seviyesini Oku
 */
uint8_t RC522_GetFifoLevel(void);

/**
 * FIFO'dan Veri Oku
 */
uint8_t RC522_ReadFifo(void);

/**
 * UID Oku
 * @param uid: UID buffer (4 bayt)
 * @return: 0 = Başarılı, 1 = Hata
 */
uint8_t RC522_ReadUID(uint8_t *uid);

/**
 * Kartla Doğrulama (MiFare Key A)
 * @param sector: Sektör numarası
 * @param key: 6 baytlık anahtar
 * @return: 0 = Başarılı, 1 = Hata
 */
uint8_t RC522_Authenticate(uint8_t sector, uint8_t *key);

/**
 * Blok Oku
 * @param block: Blok numarası
 * @param data: Okunacak veri (16 bayt)
 * @return: 0 = Başarılı, 1 = Hata
 */
uint8_t RC522_ReadBlock(uint8_t block, uint8_t *data);

/**
 * Blok Yaz
 * @param block: Blok numarası
 * @param data: Yazılacak veri (16 bayt)
 * @return: 0 = Başarılı, 1 = Hata
 */
uint8_t RC522_WriteBlock(uint8_t block, uint8_t *data);

/**
 * Kart Uyut (Halt)
 */
void RC522_Halt(void);

// ============================================
// Genel Değişkenler
// ============================================

extern uint8_t rc522_uid[4];
extern uint8_t rc522_uid_size;
extern SPI_HandleTypeDef hspi1;

#endif /* RC522_H */
