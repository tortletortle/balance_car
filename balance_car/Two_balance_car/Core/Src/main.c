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
#include <string.h>
#include "app.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_DELAY_US_USE_DWT        0U
#define APP_DELAY_US_NOP_INNER_LOOP 6U
#define APP_IMU_I2C_BIT_DELAY_US    20U
#define APP_MOTOR_OUTPUT_ENABLE     0U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static app_t g_app;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void App_DelayUsInit(void);
static void App_DelayUs(uint32_t delay_us);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void App_DelayUsInit(void)
{
#if APP_DELAY_US_USE_DWT == 1U
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0U;
#endif
}

static void App_DelayUs(uint32_t delay_us)
{
#if APP_DELAY_US_USE_DWT == 1U
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
#else
  volatile uint32_t outer_count;
  volatile uint32_t inner_count;

  for (outer_count = 0U; outer_count < delay_us; outer_count++)
  {
    for (inner_count = 0U; inner_count < APP_DELAY_US_NOP_INNER_LOOP; inner_count++)
    {
      __NOP();
    }
  }
#endif
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
  app_config_t app_config;

  App_DelayUsInit();
  memset(&app_config, 0, sizeof(app_config));
  app_config.hw.hadc_battery = &hadc1;
  app_config.hw.adc_battery_channel = ADC_CHANNEL_6;
  app_config.hw.htim_encoder_a = &htim2;
  app_config.hw.encoder_a_direction = 1;
  app_config.hw.htim_encoder_b = &htim4;
  app_config.hw.encoder_b_direction = 1;
  app_config.hw.htim_pwm = &htim3;
  app_config.hw.pwm_channel_motor_a = TIM_CHANNEL_4;
  app_config.hw.pwm_channel_motor_b = TIM_CHANNEL_3;
  app_config.hw.huart_debug = &huart1;
  app_config.hw.imu_scl_port = IMU_SCL_GPIO_Port;
  app_config.hw.imu_scl_pin = IMU_SCL_Pin;
  app_config.hw.imu_sda_port = IMU_SDA_GPIO_Port;
  app_config.hw.imu_sda_pin = IMU_SDA_Pin;
  app_config.hw.estop_port = ESTOP_GPIO_Port;
  app_config.hw.estop_pin = ESTOP_Pin;
  app_config.hw.estop_active_low = 1U;
  app_config.hw.motor_output_enable = APP_MOTOR_OUTPUT_ENABLE;
  app_config.hw.imu_i2c_bit_delay_us = APP_IMU_I2C_BIT_DELAY_US;
  app_config.hw.delay_us = App_DelayUs;
  app_config.logic = g_app_default_logic_config;

  if (app_init(&g_app, &app_config, HAL_GetTick()) != HAL_OK)
  {
    Error_Handler();
  }
/* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (app_task(&g_app, HAL_GetTick()) != HAL_OK)
    {
      Error_Handler();
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
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (app_on_uart_rx_cplt(&g_app, huart) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (app_on_uart_error(&g_app, huart) != HAL_OK)
  {
    Error_Handler();
  }
}

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
