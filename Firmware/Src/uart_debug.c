/*
 * uart_debug.c
 * UART Debug Mesajları - STM32F407G-DISC1
 * UART3 kullanılıyor (PB10=TX, PB11=RX)
 */

#include "uart_debug.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// UART Handle (main.c'den alınacak)
extern UART_HandleTypeDef huart3;

// Buffer
static char debug_buffer[256];

/*
 * Debug Mesajı Yazdır (Printf benzeri)
 * format: Format string
 * ...: Argümanlar
 */
void Debug_Printf(const char *format, ...)
{
    va_list args;
    uint16_t len;
    
    // Argümanları işle
    va_start(args, format);
    len = vsnprintf(debug_buffer, sizeof(debug_buffer), format, args);
    va_end(args);
    
    // UART'a Gönder
    if (len > 0)
    {
        HAL_UART_Transmit(&huart3, (uint8_t *)debug_buffer, len, HAL_MAX_DELAY);
    }
}

/*
 * Raw Veri Gönder
 * data: Veri buffer
 * len: Veri uzunluğu
 */
void Debug_SendRaw(uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart3, data, len, HAL_MAX_DELAY);
}

/*
 * Hex Dump Yazdır
 * data: Veri buffer
 * len: Veri uzunluğu
 * label: Etiket (opsiyonel)
 */
void Debug_HexDump(uint8_t *data, uint16_t len, const char *label)
{
    uint16_t i;
    
    if (label)
    {
        Debug_Printf("%s (%d bayt):\r\n", label, len);
    }
    
    for (i = 0; i < len; i++)
    {
        Debug_Printf("%02X ", data[i]);
        
        if ((i + 1) % 16 == 0)
        {
            Debug_Printf("\r\n");
        }
    }
    
    if (len % 16 != 0)
    {
        Debug_Printf("\r\n");
    }
}

/*
 * Sistem Bilgileri Yazdır
 */
void Debug_SystemInfo(void)
{
    Debug_Printf("\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("STM32F407G-DISC1 RC522 RFID Sistemi\r\n");
    Debug_Printf("Tarih: 2026-05-06\r\n");
    Debug_Printf("Sistem Saati: 168 MHz\r\n");
    Debug_Printf("UART: 115200 baud\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("\r\n");
}

/*
 * Başlatma Mesajı
 */
void Debug_Init(void)
{
    HAL_Delay(100);
    Debug_Printf("\r\n");
    Debug_Printf("╔═══════���════════════════════════════════╗\r\n");
    Debug_Printf("║   RC522 RFID Guvenli Okuma/Yazma      ║\r\n");
    Debug_Printf("║        STM32F407G-DISC1               ║\r\n");
    Debug_Printf("║   MiFare Classic + AES-128 Sifreleme  ║\r\n");
    Debug_Printf("╚════════════════════════════════════════╝\r\n");
    Debug_Printf("\r\n");
    Debug_Printf("Sistem Baslatiliyor...\r\n");
    Debug_Printf("\r\n");
}

/*
 * Hata Mesajı
 */
void Debug_Error(const char *error_msg)
{
    Debug_Printf("❌ HATA: %s\r\n", error_msg);
}

/*
 * Başarı Mesajı
 */
void Debug_Success(const char *success_msg)
{
    Debug_Printf("✓ BASARILI: %s\r\n", success_msg);
}

/*
 * Uyarı Mesajı
 */
void Debug_Warning(const char *warning_msg)
{
    Debug_Printf("⚠ UYARI: %s\r\n", warning_msg);
}

/*
 * Bilgi Mesajı
 */
void Debug_Info(const char *info_msg)
{
    Debug_Printf("ℹ BILGI: %s\r\n", info_msg);
}

/*
 * Ayırıcı Satır
 */
void Debug_Separator(void)
{
    Debug_Printf("========================================\r\n");
}

/*
 * Boş Satır
 */
void Debug_Newline(void)
{
    Debug_Printf("\r\n");
}

/*
 * UART Callback - Veri Alındı (opsiyonel)
 * İhtiyac halinde main.c'de kullanılabilir
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        // PC'den veri geldi
        // Gerekli işlemleri yap
    }
}
