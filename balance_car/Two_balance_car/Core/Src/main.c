/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdint.h>
#include "drv_adc.h"
#include "drv_encoder.h"
#include "drv_soft_i2c.h"
#include "dev_mpu6050.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

static uint32_t g_uart_heartbeat_last_ms = 0U;
static drv_adc_t g_battery_adc;
static drv_encoder_t g_encoder_tim2;
static drv_encoder_t g_encoder_tim4;
static drv_soft_i2c_bus_t g_imu_soft_i2c_bus;
static dev_mpu6050_t g_mpu6050;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

static void App_UartSendText(const char *text);
static void App_DelayUsInit(void);
static void App_DelayUs(uint32_t delay_us);
static void App_DriversInit(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void App_DelayUsInit(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0U;
}

static void App_DelayUs(uint32_t delay_us)
{
  uint32_t start_cycle;
  uint32_t wait_cycles;

  if (delay_us == 0U)
  {
    return;
  }

  start_cycle = DWT->CYCCNT;
  wait_cycles = (SystemCoreClock / 1000000U) * delay_us;

  while ((DWT->CYCCNT - start_cycle) < wait_cycles)
  {
  }
}

static void App_DriversInit(void)
{
  App_DelayUsInit();

  g_imu_soft_i2c_bus.scl_port = IMU_SCL_GPIO_Port;
  g_imu_soft_i2c_bus.scl_pin = IMU_SCL_Pin;
  g_imu_soft_i2c_bus.sda_port = IMU_SDA_GPIO_Port;
  g_imu_soft_i2c_bus.sda_pin = IMU_SDA_Pin;
  g_imu_soft_i2c_bus.bit_delay_us = 5U;
  g_imu_soft_i2c_bus.delay_us = App_DelayUs;

  if (drv_encoder_init(&g_encoder_tim2, &htim2, 1) != HAL_OK)
  {
    Error_Handler();
  }

  if (drv_encoder_init(&g_encoder_tim4, &htim4, 1) != HAL_OK)
  {
    Error_Handler();
  }

  if (drv_encoder_start(&g_encoder_tim2) != HAL_OK)
  {
    Error_Handler();
  }

  if (drv_encoder_start(&g_encoder_tim4) != HAL_OK)
  {
    Error_Handler();
  }

  if (drv_soft_i2c_init(&g_imu_soft_i2c_bus) != HAL_OK)
  {
    Error_Handler();
  }

  if (drv_adc_init(&g_battery_adc, &hadc1, ADC_CHANNEL_6) != HAL_OK)
  {
    Error_Handler();
  }

  if (dev_mpu6050_bind(&g_mpu6050, &g_imu_soft_i2c_bus, DEV_MPU6050_DEFAULT_ADDRESS_7BIT) != HAL_OK)
  {
    Error_Handler();
  }

  if (dev_mpu6050_init(&g_mpu6050, &g_dev_mpu6050_default_init_config) != HAL_OK)
  {
    Error_Handler();
  }
}

static void App_UartSendText(const char *text)
{
  uint16_t len = 0U;

  if (text == NULL)
  {
    return;
  }

  while (text[len] != '\0')
  {
    len++;
  }

  if (len > 0U)
  {
    (void)HAL_UART_Transmit(&huart1, (uint8_t *)text, len, 100U);
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  App_DriversInit();
  App_UartSendText("BOOT,USART1=OK\r\n");
  g_uart_heartbeat_last_ms = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if ((HAL_GetTick() - g_uart_heartbeat_last_ms) >= 1000U)
    {
      g_uart_heartbeat_last_ms = HAL_GetTick();
      App_UartSendText("HB,USART1=OK\r\n");
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
