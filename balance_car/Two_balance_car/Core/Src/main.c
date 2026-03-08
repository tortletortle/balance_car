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

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include "drv_adc.h"
#include "drv_encoder.h"
#include "drv_soft_i2c.h"
#include "dev_mpu6050.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum
{
  APP_IMU_STATE_RETRY_WAIT = 0U,
  APP_IMU_STATE_SETTLE,
  APP_IMU_STATE_CALIBRATING,
  APP_IMU_STATE_RUNNING
} app_imu_state_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define APP_MPU6050_ZERO_SAMPLE_COUNT        300U
#define APP_MPU6050_ZERO_SAMPLE_INTERVAL_MS  5U
#define APP_MPU6050_REPORT_INTERVAL_MS       200U
#define APP_MPU6050_CAL_PROGRESS_STEP       50U
#define APP_MPU6050_RETRY_INTERVAL_MS       200U
#define APP_MPU6050_STARTUP_SETTLE_MS       500U
#define APP_IMU_BRINGUP_ONLY                1U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

static uint32_t g_uart_heartbeat_last_ms = 0U;
static uint32_t g_mpu_report_last_ms = 0U;
static uint8_t g_mpu_calibration_ok = 0U;
static int32_t g_mpu_gyro_bias_x = 0;
static int32_t g_mpu_gyro_bias_y = 0;
static int32_t g_mpu_gyro_bias_z = 0;
static int32_t g_mpu_accel_zero_x = 0;
static int32_t g_mpu_accel_zero_y = 0;
static int32_t g_mpu_accel_zero_z = 0;
static int32_t g_mpu_pitch_zero_mdeg = 0;
#if APP_IMU_BRINGUP_ONLY == 0U
static drv_adc_t g_battery_adc;
static drv_encoder_t g_encoder_tim2;
static drv_encoder_t g_encoder_tim4;
#endif
static drv_soft_i2c_bus_t g_imu_soft_i2c_bus;
static dev_mpu6050_t g_mpu6050;
static app_imu_state_t g_mpu_state = APP_IMU_STATE_RETRY_WAIT;
static uint32_t g_mpu_retry_last_ms = 0U;
static uint32_t g_mpu_next_action_ms = 0U;
static uint32_t g_mpu_cal_sample_index = 0U;
static int64_t g_mpu_cal_sum_accel_x = 0;
static int64_t g_mpu_cal_sum_accel_y = 0;
static int64_t g_mpu_cal_sum_accel_z = 0;
static int64_t g_mpu_cal_sum_gyro_x = 0;
static int64_t g_mpu_cal_sum_gyro_y = 0;
static int64_t g_mpu_cal_sum_gyro_z = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

static void App_UartSendText(const char *text);
static void App_UartSendFormat(const char *format, ...);
static void App_ReportResetFlags(void);
static void App_DisableUnusedIrq(void);
static void App_Mpu6050SendCalIndex(uint32_t sample_index);
static int32_t App_Mpu6050EstimatePitchMdeg(int32_t accel_x, int32_t accel_z);
static void App_Mpu6050ResetCalibrationContext(void);
static void App_Mpu6050FinalizeCalibration(void);
static void App_Mpu6050Task(void);
static void App_Mpu6050ReportRaw(void);
static const char *App_SoftI2cDiagStageText(uint8_t stage);
static void App_ReportSoftI2cDiag(const char *prefix);
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
  g_imu_soft_i2c_bus.bit_delay_us = 20U;
  g_imu_soft_i2c_bus.delay_us = App_DelayUs;

#if APP_IMU_BRINGUP_ONLY == 0U
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

  if (drv_adc_init(&g_battery_adc, &hadc1, ADC_CHANNEL_6) != HAL_OK)
  {
    Error_Handler();
  }
#endif

  if (drv_soft_i2c_init(&g_imu_soft_i2c_bus) != HAL_OK)
  {
    Error_Handler();
  }

  if (dev_mpu6050_bind(&g_mpu6050, &g_imu_soft_i2c_bus, DEV_MPU6050_DEFAULT_ADDRESS_7BIT) != HAL_OK)
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

static void App_UartSendFormat(const char *format, ...)
{
  char buffer[160];
  int len;
  va_list args;

  if (format == NULL)
  {
    return;
  }

  va_start(args, format);
  len = vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  if (len <= 0)
  {
    return;
  }

  if (len >= (int)sizeof(buffer))
  {
    len = (int)sizeof(buffer) - 1;
  }

  (void)HAL_UART_Transmit(&huart1, (uint8_t *)buffer, (uint16_t)len, 100U);
}

static const char *App_SoftI2cDiagStageText(uint8_t stage)
{
  switch ((drv_soft_i2c_diag_stage_t)stage)
  {
    case DRV_SOFT_I2C_DIAG_STAGE_READ_START_WRITE:
      return "RD_ST1";
    case DRV_SOFT_I2C_DIAG_STAGE_READ_ADDR_WRITE:
      return "RD_AW";
    case DRV_SOFT_I2C_DIAG_STAGE_READ_REG:
      return "RD_REG";
    case DRV_SOFT_I2C_DIAG_STAGE_READ_REG_ACK:
      return "RD_RACK";
    case DRV_SOFT_I2C_DIAG_STAGE_READ_START_READ:
      return "RD_ST2";
    case DRV_SOFT_I2C_DIAG_STAGE_READ_ADDR_READ:
      return "RD_AR";
    case DRV_SOFT_I2C_DIAG_STAGE_READ_DATA:
      return "RD_DATA";
    case DRV_SOFT_I2C_DIAG_STAGE_READ_DATA_ACK:
      return "RD_WACK";
    case DRV_SOFT_I2C_DIAG_STAGE_READ_STOP:
      return "RD_STOP";
    case DRV_SOFT_I2C_DIAG_STAGE_READ_DONE:
      return "RD_DONE";
    case DRV_SOFT_I2C_DIAG_STAGE_IDLE:
    default:
      return "IDLE";
  }
}

static void App_ReportSoftI2cDiag(const char *prefix)
{
  drv_soft_i2c_diag_t diag;

  diag = drv_soft_i2c_diag_get();
  App_UartSendFormat("%s,DIAG=%s(%u),ACK=%u,IDX=%u,LEN=%u,REG=0x%02X,ADDR=0x%02X\r\n",
                    (prefix != NULL) ? prefix : "I2C",
                    App_SoftI2cDiagStageText(diag.stage),
                    diag.stage,
                    diag.ack_bit,
                    diag.index,
                    diag.length,
                    diag.reg_addr,
                    diag.device_addr7);
}

static void App_ReportResetFlags(void)
{
  uint32_t csr_value;
  uint8_t pin_reset;
  uint8_t por_reset;
  uint8_t software_reset;
  uint8_t iwdg_reset;
  uint8_t wwdg_reset;
  uint8_t lpwr_reset;

  csr_value = RCC->CSR;
  pin_reset = (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET) ? 1U : 0U;
  por_reset = (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET) ? 1U : 0U;
  software_reset = (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) != RESET) ? 1U : 0U;
  iwdg_reset = (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET) ? 1U : 0U;
  wwdg_reset = (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) != RESET) ? 1U : 0U;
  lpwr_reset = (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST) != RESET) ? 1U : 0U;

  App_UartSendFormat("RST,CSR=0x%08lX,PIN=%u,POR=%u,SFT=%u,IWDG=%u,WWDG=%u,LPWR=%u\r\n",
                    (unsigned long)csr_value,
                    pin_reset,
                    por_reset,
                    software_reset,
                    iwdg_reset,
                    wwdg_reset,
                    lpwr_reset);

  __HAL_RCC_CLEAR_RESET_FLAGS();
}
static void App_DisableUnusedIrq(void)
{
#if APP_IMU_BRINGUP_ONLY != 0U
  HAL_NVIC_DisableIRQ(USART1_IRQn);
  HAL_NVIC_DisableIRQ(DMA1_Channel4_IRQn);
  HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);
  HAL_NVIC_DisableIRQ(TIM1_UP_IRQn);
#endif
}

static void App_Mpu6050SendCalIndex(uint32_t sample_index)
{
  char text[] = "MPU,CAL,IDX=000\r\n";
  uint32_t value;

  value = sample_index + 1U;
  if (value > 999U)
  {
    value = 999U;
  }

  text[12] = (char)('0' + ((value / 100U) % 10U));
  text[13] = (char)('0' + ((value / 10U) % 10U));
  text[14] = (char)('0' + (value % 10U));
  App_UartSendText(text);
}

static int32_t App_Mpu6050EstimatePitchMdeg(int32_t accel_x, int32_t accel_z)
{
  float pitch_deg;

  pitch_deg = -atan2f((float)accel_x, (float)accel_z) * 57.2957795f;
  return (int32_t)(pitch_deg * 1000.0f);
}

static void App_Mpu6050ResetCalibrationContext(void)
{
  g_mpu_calibration_ok = 0U;
  g_mpu_cal_sample_index = 0U;
  g_mpu_cal_sum_accel_x = 0;
  g_mpu_cal_sum_accel_y = 0;
  g_mpu_cal_sum_accel_z = 0;
  g_mpu_cal_sum_gyro_x = 0;
  g_mpu_cal_sum_gyro_y = 0;
  g_mpu_cal_sum_gyro_z = 0;
}

static void App_Mpu6050FinalizeCalibration(void)
{
  App_UartSendText("MPU,CAL,SUM_OK\r\n");

  g_mpu_accel_zero_x = (int32_t)(g_mpu_cal_sum_accel_x / (int64_t)APP_MPU6050_ZERO_SAMPLE_COUNT);
  g_mpu_accel_zero_y = (int32_t)(g_mpu_cal_sum_accel_y / (int64_t)APP_MPU6050_ZERO_SAMPLE_COUNT);
  g_mpu_accel_zero_z = (int32_t)(g_mpu_cal_sum_accel_z / (int64_t)APP_MPU6050_ZERO_SAMPLE_COUNT);
  g_mpu_gyro_bias_x = (int32_t)(g_mpu_cal_sum_gyro_x / (int64_t)APP_MPU6050_ZERO_SAMPLE_COUNT);
  g_mpu_gyro_bias_y = (int32_t)(g_mpu_cal_sum_gyro_y / (int64_t)APP_MPU6050_ZERO_SAMPLE_COUNT);
  g_mpu_gyro_bias_z = (int32_t)(g_mpu_cal_sum_gyro_z / (int64_t)APP_MPU6050_ZERO_SAMPLE_COUNT);

  App_UartSendText("MPU,CAL,AVG_OK\r\n");

  g_mpu_pitch_zero_mdeg = App_Mpu6050EstimatePitchMdeg(g_mpu_accel_zero_x, g_mpu_accel_zero_z);
  App_UartSendText("MPU,CAL,PITCH_OK\r\n");

  App_UartSendFormat("MPU_ZERO,GX=%ld,GY=%ld,GZ=%ld\r\n",
                    (long)g_mpu_gyro_bias_x,
                    (long)g_mpu_gyro_bias_y,
                    (long)g_mpu_gyro_bias_z);
  App_UartSendFormat("MPU_ACC_ZERO,AX=%ld,AY=%ld,AZ=%ld\r\n",
                    (long)g_mpu_accel_zero_x,
                    (long)g_mpu_accel_zero_y,
                    (long)g_mpu_accel_zero_z);
  App_UartSendFormat("MPU_PITCH_ZERO_MDEG=%ld\r\n", (long)g_mpu_pitch_zero_mdeg);
  App_UartSendText("MPU,CAL=OK\r\n");
  g_mpu_calibration_ok = 1U;
  g_mpu_state = APP_IMU_STATE_RUNNING;
  g_mpu_report_last_ms = HAL_GetTick();
}

static void App_Mpu6050Task(void)
{
  uint32_t now_ms;
  uint8_t who_am_i = 0U;
  dev_mpu6050_raw_data_t raw_data = {0};

  now_ms = HAL_GetTick();

  switch (g_mpu_state)
  {
    case APP_IMU_STATE_RETRY_WAIT:
      if ((now_ms - g_mpu_retry_last_ms) < APP_MPU6050_RETRY_INTERVAL_MS)
      {
        return;
      }

      g_mpu_retry_last_ms = now_ms;
      if (dev_mpu6050_init(&g_mpu6050, &g_dev_mpu6050_default_init_config) != HAL_OK)
      {
        App_UartSendText("MPU,INIT=RETRY\r\n");
        return;
      }

      if (dev_mpu6050_read_who_am_i(&g_mpu6050, &who_am_i) != HAL_OK)
      {
        App_UartSendText("MPU,WHOAMI=READ_ERR\r\n");
        App_ReportSoftI2cDiag("MPU,WHOAMI");
        return;
      }

      App_Mpu6050ResetCalibrationContext();
      App_UartSendFormat("MPU,WHOAMI=0x%02X\r\n", who_am_i);
      App_UartSendFormat("MPU,CALIBRATING=%lu\r\n", (unsigned long)APP_MPU6050_ZERO_SAMPLE_COUNT);
      g_mpu_next_action_ms = now_ms + APP_MPU6050_STARTUP_SETTLE_MS;
      g_mpu_state = APP_IMU_STATE_SETTLE;
      return;

    case APP_IMU_STATE_SETTLE:
      if ((int32_t)(now_ms - g_mpu_next_action_ms) < 0)
      {
        return;
      }

      App_UartSendText("MPU,CAL,START\r\n");
      if (dev_mpu6050_read_raw(&g_mpu6050, &raw_data) != HAL_OK)
      {
        App_UartSendText("MPU,CAL,RAW_TEST=ERR\r\n");
        App_ReportSoftI2cDiag("MPU,CAL");
        g_mpu_state = APP_IMU_STATE_RETRY_WAIT;
        g_mpu_retry_last_ms = now_ms;
        return;
      }

      App_UartSendText("MPU,CAL,RAW_TEST=OK\r\n");
      g_mpu_next_action_ms = now_ms;
      g_mpu_state = APP_IMU_STATE_CALIBRATING;
      return;

    case APP_IMU_STATE_CALIBRATING:
      if ((int32_t)(now_ms - g_mpu_next_action_ms) < 0)
      {
        return;
      }

      if (dev_mpu6050_read_raw(&g_mpu6050, &raw_data) != HAL_OK)
      {
        App_UartSendFormat("MPU,CAL,READ_ERR,IDX=%lu\r\n", (unsigned long)g_mpu_cal_sample_index);
        App_ReportSoftI2cDiag("MPU,CAL");
        App_Mpu6050ResetCalibrationContext();
        g_mpu_state = APP_IMU_STATE_RETRY_WAIT;
        g_mpu_retry_last_ms = now_ms;
        return;
      }

      g_mpu_cal_sum_accel_x += raw_data.accel_x;
      g_mpu_cal_sum_accel_y += raw_data.accel_y;
      g_mpu_cal_sum_accel_z += raw_data.accel_z;
      g_mpu_cal_sum_gyro_x += raw_data.gyro_x;
      g_mpu_cal_sum_gyro_y += raw_data.gyro_y;
      g_mpu_cal_sum_gyro_z += raw_data.gyro_z;

      if (((g_mpu_cal_sample_index + 1U) <= 10U) || (((g_mpu_cal_sample_index + 1U) % APP_MPU6050_CAL_PROGRESS_STEP) == 0U))
      {
        App_Mpu6050SendCalIndex(g_mpu_cal_sample_index);
      }

      g_mpu_cal_sample_index++;
      if (g_mpu_cal_sample_index >= APP_MPU6050_ZERO_SAMPLE_COUNT)
      {
        App_Mpu6050FinalizeCalibration();
        return;
      }

      g_mpu_next_action_ms = now_ms + APP_MPU6050_ZERO_SAMPLE_INTERVAL_MS;
      return;

    case APP_IMU_STATE_RUNNING:
    default:
      return;
  }
}

static void App_Mpu6050ReportRaw(void)
{
  dev_mpu6050_raw_data_t raw_data = {0};
  int32_t pitch_zero_relative_mdeg;

  if (dev_mpu6050_read_raw(&g_mpu6050, &raw_data) != HAL_OK)
  {
    App_UartSendText("MPU,READ=ERR\r\n");
    App_ReportSoftI2cDiag("MPU,READ");
    App_Mpu6050ResetCalibrationContext();
    g_mpu_state = APP_IMU_STATE_RETRY_WAIT;
    g_mpu_retry_last_ms = HAL_GetTick();
    return;
  }

  pitch_zero_relative_mdeg = App_Mpu6050EstimatePitchMdeg(raw_data.accel_x, raw_data.accel_z) - g_mpu_pitch_zero_mdeg;

  App_UartSendFormat("MPU_RAW,CAL=%u,AX=%d,AY=%d,AZ=%d,GX=%d,GY=%d,GZ=%d,GX0=%ld,GY0=%ld,GZ0=%ld,PITCH0=%ld\r\n",
                    g_mpu_calibration_ok,
                    raw_data.accel_x,
                    raw_data.accel_y,
                    raw_data.accel_z,
                    raw_data.gyro_x,
                    raw_data.gyro_y,
                    raw_data.gyro_z,
                    (long)((int32_t)raw_data.gyro_x - g_mpu_gyro_bias_x),
                    (long)((int32_t)raw_data.gyro_y - g_mpu_gyro_bias_y),
                    (long)((int32_t)raw_data.gyro_z - g_mpu_gyro_bias_z),
                    (long)pitch_zero_relative_mdeg);
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
  App_ReportResetFlags();
  App_Mpu6050ResetCalibrationContext();
  g_mpu_state = APP_IMU_STATE_RETRY_WAIT;
  g_mpu_retry_last_ms = HAL_GetTick() - APP_MPU6050_RETRY_INTERVAL_MS;
  g_mpu_next_action_ms = HAL_GetTick();
  g_uart_heartbeat_last_ms = HAL_GetTick();
  g_mpu_report_last_ms = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    App_Mpu6050Task();

    if ((HAL_GetTick() - g_uart_heartbeat_last_ms) >= 1000U)
    {
      g_uart_heartbeat_last_ms = HAL_GetTick();
      App_UartSendText("HB,USART1=OK\r\n");
    }

    if ((g_mpu_state == APP_IMU_STATE_RUNNING) && ((HAL_GetTick() - g_mpu_report_last_ms) >= APP_MPU6050_REPORT_INTERVAL_MS))
    {
      g_mpu_report_last_ms = HAL_GetTick();
      App_Mpu6050ReportRaw();
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
