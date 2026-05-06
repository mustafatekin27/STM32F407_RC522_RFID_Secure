/*
 * main.c
 * STM32F407G-DISC1 RC522 RFID Güvenli Okuma/Yazma
 * Ana Program
 */

#include "main.h"
#include "rc522.h"
#include "card_operations.h"
#include "uart_debug.h"
#include "config.h"

// Periperalal Handle'ları (STM32CubeMX tarafından üretilecek)
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart3;
TIM_HandleTypeDef htim2;

// Fonksiyon Prototipi
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM2_Init(void);

// LED GPIO Tanımlamaları
#define LED_ORANGE_PIN GPIO_PIN_13   // PD13
#define LED_RED_PIN GPIO_PIN_14      // PD14
#define LED_BLUE_PIN GPIO_PIN_15     // PD15
#define LED_GREEN_PIN GPIO_PIN_12    // PD12

/*
 * LED Kontrolü
 */
void LED_Orange_On(void)
{
    HAL_GPIO_WritePin(GPIOD, LED_ORANGE_PIN, GPIO_PIN_SET);
}

void LED_Orange_Off(void)
{
    HAL_GPIO_WritePin(GPIOD, LED_ORANGE_PIN, GPIO_PIN_RESET);
}

void LED_Red_On(void)
{
    HAL_GPIO_WritePin(GPIOD, LED_RED_PIN, GPIO_PIN_SET);
}

void LED_Red_Off(void)
{
    HAL_GPIO_WritePin(GPIOD, LED_RED_PIN, GPIO_PIN_RESET);
}

void LED_Blue_On(void)
{
    HAL_GPIO_WritePin(GPIOD, LED_BLUE_PIN, GPIO_PIN_SET);
}

void LED_Blue_Off(void)
{
    HAL_GPIO_WritePin(GPIOD, LED_BLUE_PIN, GPIO_PIN_RESET);
}

void LED_Green_On(void)
{
    HAL_GPIO_WritePin(GPIOD, LED_GREEN_PIN, GPIO_PIN_SET);
}

void LED_Green_Off(void)
{
    HAL_GPIO_WritePin(GPIOD, LED_GREEN_PIN, GPIO_PIN_RESET);
}

/*
 * Tüm LED'leri Kapat
 */
void LED_AllOff(void)
{
    LED_Orange_Off();
    LED_Red_Off();
    LED_Blue_Off();
    LED_Green_Off();
}

/*
 * Hata Göstergesi
 */
void LED_ErrorBlink(void)
{
    uint8_t i;
    for (i = 0; i < 5; i++)
    {
        LED_Red_On();
        HAL_Delay(200);
        LED_Red_Off();
        HAL_Delay(200);
    }
}

/*
 * Başarı Göstergesi
 */
void LED_SuccessBlink(void)
{
    uint8_t i;
    for (i = 0; i < 3; i++)
    {
        LED_Green_On();
        HAL_Delay(300);
        LED_Green_Off();
        HAL_Delay(300);
    }
}

/*
 * Ana Program
 */
int main(void)
{
    // Sistem Başlatma
    HAL_Init();
    SystemClock_Config();
    
    // Periperalal İnitiyalizasyon
    MX_GPIO_Init();
    MX_SPI1_Init();
    MX_USART3_UART_Init();
    MX_TIM2_Init();
    
    // Debug Mesajları
    Debug_Init();
    Debug_SystemInfo();
    
    // RC522 Başlat
    RC522_Init();
    RC522_AntennaOn();
    
    LED_AllOff();
    LED_Orange_On();
    
    Debug_Printf("Sistem Hazir. Karinti Bekliyor...\r\n");
    Debug_Printf("\r\n");
    
    // ============================================
    // TEST MOD - KART İŞLEMLERİ
    // ============================================
    
    uint8_t uid[4];
    uint8_t status;
    
    while (1)
    {
        Debug_Printf("Lütfen Kart Yaklaştırın...\r\n");
        LED_Orange_On();
        
        // Kart Algılanmasını Bekle
        HAL_Delay(500);
        
        // UID Oku
        status = RC522_ReadUID(uid);
        
        if (status == 0)
        {
            LED_Orange_Off();
            LED_Blue_On();
            
            Debug_Printf("\r\n");
            Debug_Success("Kart Algılandi!");
            Debug_Printf("UID: %02X %02X %02X %02X\r\n", uid[0], uid[1], uid[2], uid[3]);
            
            // ============================================
            // SEÇENEK 1: KARTı İLK KURULUMLA YAZ
            // ============================================
            
            // Açıklama: İlk kez bir karta veri yazmak için kullanın
            // Status = Card_WriteInitial("Ahmet", "Yilmaz", 1000);
            
            // if (status == 0)
            // {
            //     LED_SuccessBlink();
            // }
            // else
            // {
            //     LED_ErrorBlink();
            // }
            
            // ============================================
            // SEÇENEK 2: KARTAN VERİ OKU
            // ============================================
            
            status = Card_ReadAll(uid);
            
            if (status == 0)
            {
                LED_SuccessBlink();
            }
            else
            {
                LED_ErrorBlink();
            }
            
            // ============================================
            // SEÇENEK 3: BAKIYE GÜNCELLE
            // ============================================
            
            // status = Card_UpdateBalance(500);
            
            // if (status == 0)
            // {
            //     LED_SuccessBlink();
            // }
            // else
            // {
            //     LED_ErrorBlink();
            // }
            
            // ============================================
            // SEÇENEK 4: BAKIYE ÇIKART
            // ============================================
            
            // status = Card_DeductBalance(100);
            
            // if (status == 0)
            // {
            //     LED_SuccessBlink();
            // }
            // else
            // {
            //     LED_ErrorBlink();
            // }
            
            // ============================================
            // SEÇENEK 5: BAKIYE EKLE
            // ============================================
            
            // status = Card_AddBalance(50);
            
            // if (status == 0)
            // {
            //     LED_SuccessBlink();
            // }
            // else
            // {
            //     LED_ErrorBlink();
            // }
            
            LED_Blue_Off();
            LED_AllOff();
            
            Debug_Printf("\r\nKarti Uzaklastirin...\r\n");
            HAL_Delay(2000);
        }
        else
        {
            LED_ErrorBlink();
        }
        
        Debug_Printf("\r\n");
        HAL_Delay(500);
    }
    
    return 0;
}

/*
 * Sistem Saati Yapılandırması
 * (STM32CubeMX tarafından otomatik üretilecek)
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    // HSE Osilatör Yapılandırması
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    
    // Saat Kaynağı Yapılandırması
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
}

/*
 * GPIO İnitiyalizasyon
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Port D Saati Aç
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    // LED Çıkışları (PD12-15)
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    // RC522 CS (PA4) Çıkışı
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    
    // RC522 RST (PB0) Çıkışı
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    
    // Tüm LED'leri Kapat
    LED_AllOff();
}

/*
 * SPI1 İnitiyalizasyon
 */
static void MX_SPI1_Init(void)
{
    __HAL_RCC_SPI1_CLK_ENABLE();
    
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;  // 8 MHz
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler();
    }
    
    // GPIO Pins: PA5 (CLK), PA6 (MISO), PA7 (MOSI)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/*
 * UART3 İnitiyalizasyon (115200, 8N1)
 */
static void MX_USART3_UART_Init(void)
{
    __HAL_RCC_USART3_CLK_ENABLE();
    
    huart3.Instance = USART3;
    huart3.Init.BaudRate = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart3) != HAL_OK)
    {
        Error_Handler();
    }
    
    // GPIO Pins: PB10 (TX), PB11 (RX)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/*
 * Timer2 İnitiyalizasyon (opsiyonel)
 */
static void MX_TIM2_Init(void)
{
    // İhtiyaca göre yapılandırılabilir
}

/*
 * Hata Handler
 */
void Error_Handler(void)
{
    Debug_Error("Sistem Hatası Oluştu!");
    LED_Red_On();
    while (1)
    {
        HAL_Delay(100);
    }
}

/*
 * Assertion Handler (Debug Mode)
 */
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    Debug_Printf("Assertion hatasi: %s:%lu\r\n", file, line);
}
#endif

/*
 * SPI MSP İnitiyalizasyon (HAL Callback)
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
    if (hspi->Instance == SPI1)
    {
        __HAL_RCC_SPI1_CLK_ENABLE();
    }
}

/*
 * UART MSP İnitiyalizasyon (HAL Callback)
 */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    if (huart->Instance == USART3)
    {
        __HAL_RCC_USART3_CLK_ENABLE();
    }
}
