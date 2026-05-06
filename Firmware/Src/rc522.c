/*
 * rc522.c
 * RC522 RFID Okuyucu SPI Driver
 * STM32F407G-DISC1 için
 */

#include "rc522.h"
#include "uart_debug.h"
#include <string.h>

// SPI Handle (main.c'den alınacak)
extern SPI_HandleTypeDef hspi1;

// Genel değişkenler
uint8_t rc522_uid[4];
uint8_t rc522_uid_size;

/*
 * RC522 Yazma Komutunu Gönder
 * addr: Yazılacak register adresi
 * value: Yazılacak değer
 */
void RC522_Write(uint8_t addr, uint8_t value)
{
    uint8_t tx_data[2];
    
    // Yazma komutu: ((addr << 1) & 0x7E) | 0x00
    tx_data[0] = ((addr << 1) & 0x7E);
    tx_data[1] = value;
    
    // CS (PA4) Düşük
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_Delay(1);
    
    // Veri Gönder
    HAL_SPI_Transmit(&hspi1, tx_data, 2, HAL_MAX_DELAY);
    
    // CS Yüksek
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(1);
}

/*
 * RC522'den Okuma Komutu
 * addr: Okunacak register adresi
 * return: Okunan değer
 */
uint8_t RC522_Read(uint8_t addr)
{
    uint8_t tx_data[2];
    uint8_t rx_data[2];
    
    // Okuma komutu: ((addr << 1) & 0x7E) | 0x80
    tx_data[0] = ((addr << 1) & 0x7E) | 0x80;
    tx_data[1] = 0x00;
    
    // CS Düşük
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_Delay(1);
    
    // Veri Gönder/Al
    HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, 2, HAL_MAX_DELAY);
    
    // CS Yüksek
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(1);
    
    return rx_data[1];
}

/*
 * RC522 Başlatma
 */
void RC522_Init(void)
{
    uint8_t i;
    
    Debug_Printf("RC522 Baslatiliyor...\r\n");
    
    // PB0 (RST) Reset Pini Yüksek
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    HAL_Delay(50);
    
    // Soft Reset
    RC522_Write(RC522_REG_COMMAND, RC522_CMD_SOFT_RESET);
    HAL_Delay(50);
    
    // Timer Register
    RC522_Write(RC522_REG_T_MODE, 0x8D);
    RC522_Write(RC522_REG_T_PRESCALER, 0x3E);
    RC522_Write(RC522_REG_T_RELOAD_H, 0x00);
    RC522_Write(RC522_REG_T_RELOAD_L, 0x30);
    
    // TX Control Register
    RC522_Write(RC522_REG_TX_CONTROL, 0x83);
    
    // RX Control Register
    RC522_Write(RC522_REG_RX_CONTROL, 0x3D);
    
    // RF Configuration Register
    RC522_Write(RC522_REG_RF_CONFIG, 0x78);
    
    // Bit Framing Register
    RC522_Write(RC522_REG_BIT_FRAMING, 0x00);
    
    // Mode Register
    RC522_Write(RC522_REG_MODE, 0x3D);
    
    Debug_Printf("RC522 Baslatildi.\r\n");
}

/*
 * RC522 Anten Aç
 */
void RC522_AntennaOn(void)
{
    uint8_t tx_control = RC522_Read(RC522_REG_TX_CONTROL);
    
    if ((tx_control & 0x03) == 0)
    {
        RC522_Write(RC522_REG_TX_CONTROL, tx_control | 0x03);
    }
    
    Debug_Printf("RC522 Anteni Acildi.\r\n");
}

/*
 * RC522 Anten Kapat
 */
void RC522_AntennaOff(void)
{
    uint8_t tx_control = RC522_Read(RC522_REG_TX_CONTROL);
    RC522_Write(RC522_REG_TX_CONTROL, tx_control & ~0x03);
}

/*
 * RC522 Komut Gönder
 * cmd: Gönderilecek komut
 * data: Komut verisi
 * data_len: Veri uzunluğu
 * return: Durum (0: Başarılı, 1: Hata)
 */
uint8_t RC522_SendCommand(uint8_t cmd, uint8_t *data, uint8_t data_len)
{
    uint8_t i;
    uint8_t status = 0;
    uint8_t irq_en = 0x00;
    uint8_t wait_irq = 0x00;
    
    // Komuta göre IRQ ayarla
    switch (cmd)
    {
        case RC522_CMD_AUTH_A:
        case RC522_CMD_AUTH_B:
            irq_en = 0x12;
            wait_irq = 0x10;
            break;
            
        case RC522_CMD_TRANSMIT:
            irq_en = 0x77;
            wait_irq = 0x30;
            break;
            
        case RC522_CMD_RECEIVE:
            irq_en = 0x77;
            wait_irq = 0x30;
            break;
            
        case RC522_CMD_TRANSCEIVE:
            irq_en = 0x77;
            wait_irq = 0x30;
            break;
            
        default:
            break;
    }
    
    // IRQ Enable Register
    RC522_Write(RC522_REG_IRQ_EN, irq_en | 0x80);
    
    // IRQ Clear Register
    RC522_ClearBitMask(RC522_REG_COMM_IRQ, 0x7F);
    
    // Set Bit Mask Register - Start komutu
    RC522_SetBitMask(RC522_REG_BIT_FRAMING, 0x80);
    
    // FIFO Data Register - Veriyi yaz
    RC522_Write(RC522_REG_FIFO_LEVEL, 0x00);
    for (i = 0; i < data_len; i++)
    {
        RC522_Write(RC522_REG_FIFO_DATA, data[i]);
    }
    
    // Command Register - Komutu çalıştır
    RC522_Write(RC522_REG_COMMAND, cmd);
    
    // Yanıt Bekle
    i = 0xFF;
    while (1)
    {
        uint8_t n = RC522_Read(RC522_REG_COMM_IRQ);
        i--;
        
        if (n & wait_irq)
        {
            break;
        }
        
        if (!(n & 0x80) || !i)
        {
            status = 1;
            break;
        }
    }
    
    // Error Register Kontrol
    uint8_t error = RC522_Read(RC522_REG_ERROR);
    if ((error & 0x13) == 0x13)
    {
        status = 1;
    }
    
    return status;
}

/*
 * Bit Maskesini Ayarla
 */
void RC522_SetBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp = RC522_Read(reg);
    RC522_Write(reg, tmp | mask);
}

/*
 * Bit Maskesini Temizle
 */
void RC522_ClearBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp = RC522_Read(reg);
    RC522_Write(reg, tmp & (~mask));
}

/*
 * FIFO Seviyesi Oku
 */
uint8_t RC522_GetFifoLevel(void)
{
    return RC522_Read(RC522_REG_FIFO_LEVEL);
}

/*
 * FIFO Verisi Oku
 */
uint8_t RC522_ReadFifo(void)
{
    return RC522_Read(RC522_REG_FIFO_DATA);
}

/*
 * UID Ara ve Oku
 * uid: UID verisi (4 bayt)
 * return: 0 = Başarılı, 1 = Hata
 */
uint8_t RC522_ReadUID(uint8_t *uid)
{
    uint8_t status;
    uint8_t i;
    uint8_t command_data[2];
    uint8_t response[16];
    uint8_t fifo_len;
    
    // REQA Komutu (Request A) - Kartı Ara
    Debug_Printf("Kart Aranıyor...\r\n");
    RC522_ClearBitMask(RC522_REG_STATUS2, 0x08);
    RC522_Write(RC522_REG_BIT_FRAMING, 0x07);
    
    command_data[0] = RC522_CMD_REQA;
    status = RC522_SendCommand(RC522_CMD_TRANSCEIVE, command_data, 1);
    
    if (status != 0)
    {
        Debug_Printf("REQA Hatasi\r\n");
        return 1;
    }
    
    // ATQ Kontrol
    fifo_len = RC522_GetFifoLevel();
    if (fifo_len != 2)
    {
        Debug_Printf("ATQ Hatasi\r\n");
        return 1;
    }
    
    uint8_t atq[2];
    for (i = 0; i < 2; i++)
    {
        atq[i] = RC522_ReadFifo();
    }
    Debug_Printf("ATQ: %02X %02X\r\n", atq[0], atq[1]);
    
    // SELECT Komutu - UID Oku
    Debug_Printf("UID Okunuyor...\r\n");
    RC522_ClearBitMask(RC522_REG_STATUS2, 0x08);
    
    command_data[0] = RC522_CMD_SELECT;
    command_data[1] = 0x20;  // NVB (Number of Valid Bits)
    
    status = RC522_SendCommand(RC522_CMD_TRANSCEIVE, command_data, 2);
    
    if (status != 0)
    {
        Debug_Printf("SELECT Hatasi\r\n");
        return 1;
    }
    
    // UID Oku
    fifo_len = RC522_GetFifoLevel();
    if (fifo_len != 7)
    {
        Debug_Printf("UID Hatasi: %d bayt\r\n", fifo_len);
        return 1;
    }
    
    for (i = 0; i < 4; i++)
    {
        uid[i] = RC522_ReadFifo();
    }
    
    uint8_t bcc = RC522_ReadFifo();  // BCC (Checksum)
    
    // BCC Kontrol
    uint8_t calc_bcc = uid[0] ^ uid[1] ^ uid[2] ^ uid[3];
    if (calc_bcc != bcc)
    {
        Debug_Printf("BCC Hatasi: %02X != %02X\r\n", calc_bcc, bcc);
        return 1;
    }
    
    Debug_Printf("UID: %02X %02X %02X %02X\r\n", uid[0], uid[1], uid[2], uid[3]);
    
    return 0;
}

/*
 * Kartla Doğrulama (MiFare Key A)
 * sector: Sektör numarası
 * key: 6 baytlık anahtar
 * return: 0 = Başarılı, 1 = Hata
 */
uint8_t RC522_Authenticate(uint8_t sector, uint8_t *key)
{
    uint8_t status;
    uint8_t command_data[12];
    
    Debug_Printf("Sektor %d ile Dogrulama Yapiliyor...\r\n", sector);
    
    // AUTH_A Komutu
    command_data[0] = RC522_CMD_AUTH_A;  // Key A
    command_data[1] = sector * 4 + 3;     // Block numarası (Trailer block)
    
    // 6 baytlık anahtar
    memcpy(&command_data[2], key, 6);
    
    // UID (4 bayt)
    memcpy(&command_data[8], rc522_uid, 4);
    
    status = RC522_SendCommand(RC522_CMD_AUTH_A, command_data, 12);
    
    if (status != 0)
    {
        Debug_Printf("Dogrulama Hatasi\r\n");
        return 1;
    }
    
    Debug_Printf("Dogrulama Basarili\r\n");
    return 0;
}

/*
 * Blok Oku
 * block: Blok numarası
 * data: Okunacak veri (16 bayt)
 * return: 0 = Başarılı, 1 = Hata
 */
uint8_t RC522_ReadBlock(uint8_t block, uint8_t *data)
{
    uint8_t status;
    uint8_t command_data[2];
    uint8_t i;
    uint8_t fifo_len;
    
    Debug_Printf("Blok %d Okunuyor...\r\n", block);
    
    command_data[0] = RC522_CMD_READ;
    command_data[1] = block;
    
    status = RC522_SendCommand(RC522_CMD_TRANSCEIVE, command_data, 2);
    
    if (status != 0)
    {
        Debug_Printf("Blok Okuma Hatasi\r\n");
        return 1;
    }
    
    fifo_len = RC522_GetFifoLevel();
    if (fifo_len != 16)
    {
        Debug_Printf("Veri Uzunlugu Hatasi: %d\r\n", fifo_len);
        return 1;
    }
    
    for (i = 0; i < 16; i++)
    {
        data[i] = RC522_ReadFifo();
    }
    
    Debug_Printf("Blok Okundu: ");
    for (i = 0; i < 16; i++)
    {
        Debug_Printf("%02X ", data[i]);
    }
    Debug_Printf("\r\n");
    
    return 0;
}

/*
 * Blok Yaz
 * block: Blok numarası
 * data: Yazılacak veri (16 bayt)
 * return: 0 = Başarılı, 1 = Hata
 */
uint8_t RC522_WriteBlock(uint8_t block, uint8_t *data)
{
    uint8_t status;
    uint8_t command_data[18];
    uint8_t i;
    
    Debug_Printf("Blok %d Yaziliyor...\r\n", block);
    
    command_data[0] = RC522_CMD_WRITE;
    command_data[1] = block;
    
    memcpy(&command_data[2], data, 16);
    
    status = RC522_SendCommand(RC522_CMD_TRANSCEIVE, command_data, 18);
    
    if (status != 0)
    {
        Debug_Printf("Blok Yazma Hatasi\r\n");
        return 1;
    }
    
    Debug_Printf("Blok Yazildi\r\n");
    return 0;
}

/*
 * Halt Komutu - Kartı Uyutma
 */
void RC522_Halt(void)
{
    uint8_t command_data[4];
    
    command_data[0] = RC522_CMD_HALT;
    command_data[1] = 0x00;
    
    RC522_SendCommand(RC522_CMD_TRANSCEIVE, command_data, 2);
    
    Debug_Printf("Kart Uyutuldu\r\n");
}
