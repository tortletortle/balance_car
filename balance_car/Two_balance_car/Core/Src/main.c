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
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include "drv_adc.h"
#include "drv_encoder.h"
#include "drv_pwm.h"
#include "drv_soft_i2c.h"
#include "dev_tb6612.h"
#include "mod_imu.h"
#include "mod_motor.h"
#include "ctrl_attitude_estimator.h"
#include "ctrl_angle_loop.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_IMU_BRINGUP_ONLY                1U
#define APP_DELAY_US_USE_DWT                0U
#define APP_DELAY_US_NOP_INNER_LOOP         6U
#define APP_ANGLE_LOOP_TARGET_PITCH_MDEG    0
#define APP_MOTOR_OUTPUT_ENABLE             0U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint32_t g_uart_heartbeat_last_ms = 0U;
static uint32_t g_attitude_report_last_ms = 0U;
#if APP_IMU_BRINGUP_ONLY == 0U
static drv_adc_t g_battery_adc;
static drv_encoder_t g_encoder_tim2;
static drv_encoder_t g_encoder_tim4;
#endif
static drv_pwm_t g_motor_pwm_motor_a;
static drv_pwm_t g_motor_pwm_motor_b;
static drv_soft_i2c_bus_t g_imu_soft_i2c_bus;
static dev_mpu6050_t g_mpu6050;
static dev_tb6612_t g_tb6612;
static mod_imu_t g_imu_module;
static mod_motor_t g_motor_module;
static ctrl_attitude_estimator_t g_attitude_estimator;
static ctrl_angle_loop_t g_angle_loop;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void App_UartSendText(const char *text);
static void App_UartSendFormat(const char *format, ...);
static void App_ReportResetFlags(void);
static void App_ImuSendCalIndex(uint32_t sample_index);
static void App_ImuHandleEvent(mod_imu_event_t event);
static void App_AttitudeEstimatorReset(void);
static int App_FormatSignedMilli(char *buffer, uint32_t buffer_size, int32_t value);
static void App_VofaSendAngleLoop(const ctrl_attitude_estimator_output_t *attitude_output,
                                  int32_t target_pitch_mdeg,
                                  const ctrl_angle_loop_output_t *angle_output);
static void App_AttitudeEstimateAndReportVofa(void);
static uint8_t App_IsEstopActive(void);
static void App_MotorApplyAngleCommand(int16_t angle_cmd);
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

static void App_DriversInit(void)
{
  App_DelayUsInit();

  g_imu_soft_i2c_bus.scl_port = IMU_SCL_GPIO_Port;
  g_imu_soft_i2c_bus.scl_pin = IMU_SCL_Pin;
  g_imu_soft_i2c_bus.sda_port = IMU_SDA_GPIO_Port;
  g_imu_soft_i2c_bus.sda_pin = IMU_SDA_Pin;
  g_imu_soft_i2c_bus.bit_delay_us = 20U;
  g_imu_soft_i2c_bus.delay_us = App_DelayUs;

  if (drv_pwm_init(&g_motor_pwm_motor_a, &htim3, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }

  if (drv_pwm_init(&g_motor_pwm_motor_b, &htim3, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }

  if (dev_tb6612_init(&g_tb6612,
                      &g_motor_pwm_motor_a,
                      &g_motor_pwm_motor_b,
                      &g_dev_tb6612_default_config) != HAL_OK)
  {
    Error_Handler();
  }

  if (dev_tb6612_start(&g_tb6612) != HAL_OK)
  {
    Error_Handler();
  }

  if (mod_motor_init(&g_motor_module, &g_tb6612, &g_mod_motor_default_config) != HAL_OK)
  {
    Error_Handler();
  }

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

static uint8_t App_IsEstopActive(void)
{
  return (HAL_GPIO_ReadPin(ESTOP_GPIO_Port, ESTOP_Pin) == GPIO_PIN_RESET) ? 1U : 0U;
}

static void App_MotorApplyAngleCommand(int16_t angle_cmd)
{
  uint8_t motor_output_enable;

  mod_motor_set_common_target(&g_motor_module, angle_cmd);
  motor_output_enable = (APP_MOTOR_OUTPUT_ENABLE != 0U) && (App_IsEstopActive() == 0U);
  mod_motor_set_enable(&g_motor_module, motor_output_enable);
  if (mod_motor_update(&g_motor_module, NULL) != HAL_OK)
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
    case DRV_SOFT_I2C_DIAG_STAGE_READ_ADDR_READ_SEND:
      return "RD_ARS";
    case DRV_SOFT_I2C_DIAG_STAGE_READ_ADDR_READ_ACK:
      return "RD_ARA";
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

static void App_ImuSendCalIndex(uint32_t sample_index)
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

static void App_AttitudeEstimatorReset(void)
{
  ctrl_attitude_estimator_reset(&g_attitude_estimator);
  ctrl_angle_loop_reset(&g_angle_loop);
  mod_motor_reset(&g_motor_module);
  g_attitude_report_last_ms = HAL_GetTick();
}

static int App_FormatSignedMilli(char *buffer, uint32_t buffer_size, int32_t value)
{
  uint32_t absolute_value;

  if ((buffer == NULL) || (buffer_size == 0U))
  {
    return -1;
  }

  if (value < 0)
  {
    absolute_value = (uint32_t)(-((int64_t)value));
    return snprintf(buffer,
                    (size_t)buffer_size,
                    "-%lu.%03lu",
                    (unsigned long)(absolute_value / 1000U),
                    (unsigned long)(absolute_value % 1000U));
  }

  absolute_value = (uint32_t)value;
  return snprintf(buffer,
                  (size_t)buffer_size,
                  "%lu.%03lu",
                  (unsigned long)(absolute_value / 1000U),
                  (unsigned long)(absolute_value % 1000U));
}

static void App_VofaSendAngleLoop(const ctrl_attitude_estimator_output_t *attitude_output,
                                  int32_t target_pitch_mdeg,
                                  const ctrl_angle_loop_output_t *angle_output)
{
  char channel_1[20];
  char channel_2[20];
  char channel_3[20];
  char channel_4[16];
  char channel_5[20];
  char channel_6[20];

  if ((attitude_output == NULL) || (angle_output == NULL))
  {
    return;
  }

  if (App_FormatSignedMilli(channel_1, sizeof(channel_1), target_pitch_mdeg) <= 0)
  {
    return;
  }

  if (App_FormatSignedMilli(channel_2, sizeof(channel_2), attitude_output->pitch_fused_mdeg) <= 0)
  {
    return;
  }

  if (App_FormatSignedMilli(channel_3, sizeof(channel_3), angle_output->pitch_error_mdeg) <= 0)
  {
    return;
  }

  if (snprintf(channel_4, sizeof(channel_4), "%d", (int)angle_output->angle_cmd) <= 0)
  {
    return;
  }

  if (App_FormatSignedMilli(channel_5, sizeof(channel_5), attitude_output->pitch_rate_mdps) <= 0)
  {
    return;
  }

  if (App_FormatSignedMilli(channel_6, sizeof(channel_6), angle_output->d_input_filt_ddeg * 100) <= 0)
  {
    return;
  }

  App_UartSendFormat("%s,%s,%s,%s,%s,%s\r\n",
                     channel_1,
                     channel_2,
                     channel_3,
                     channel_4,
                     channel_5,
                     channel_6);
}

static void App_ImuHandleEvent(mod_imu_event_t event)
{
  const mod_imu_calibration_t *calibration;

  calibration = mod_imu_get_calibration(&g_imu_module);

  switch (event.id)
  {
    case MOD_IMU_EVENT_INIT_RETRY:
      App_AttitudeEstimatorReset();
      App_UartSendText("MPU,INIT=RETRY\r\n");
      return;

    case MOD_IMU_EVENT_WHOAMI_READ_ERR:
      App_AttitudeEstimatorReset();
      App_UartSendText("MPU,WHOAMI=READ_ERR\r\n");
      App_ReportSoftI2cDiag("MPU,WHOAMI");
      return;

    case MOD_IMU_EVENT_WHOAMI_OK:
      App_AttitudeEstimatorReset();
      App_UartSendFormat("MPU,WHOAMI=0x%02X\r\n", event.who_am_i);
      App_UartSendFormat("MPU,CALIBRATING=%lu\r\n", (unsigned long)g_imu_module.config.zero_sample_count);
      return;

    case MOD_IMU_EVENT_RAW_TEST_ERR:
      App_AttitudeEstimatorReset();
      App_UartSendText("MPU,CAL,START\r\n");
      App_UartSendText("MPU,CAL,RAW_TEST=ERR\r\n");
      App_ReportSoftI2cDiag("MPU,CAL");
      return;

    case MOD_IMU_EVENT_RAW_TEST_OK:
      App_UartSendText("MPU,CAL,START\r\n");
      App_UartSendText("MPU,CAL,RAW_TEST=OK\r\n");
      return;

    case MOD_IMU_EVENT_CAL_PROGRESS:
      App_ImuSendCalIndex(event.sample_index);
      return;

    case MOD_IMU_EVENT_CAL_READ_ERR:
      App_AttitudeEstimatorReset();
      App_UartSendFormat("MPU,CAL,READ_ERR,IDX=%lu\r\n", (unsigned long)event.sample_index);
      App_ReportSoftI2cDiag("MPU,CAL");
      return;

    case MOD_IMU_EVENT_CAL_DONE:
      if (calibration == NULL)
      {
        return;
      }

      App_UartSendText("MPU,CAL,SUM_OK\r\n");
      App_UartSendText("MPU,CAL,AVG_OK\r\n");
      App_UartSendText("MPU,CAL,PITCH_OK\r\n");
      App_UartSendFormat("MPU_ZERO,GX=%ld,GY=%ld,GZ=%ld\r\n",
                        (long)calibration->gyro_bias_x,
                        (long)calibration->gyro_bias_y,
                        (long)calibration->gyro_bias_z);
      App_UartSendFormat("MPU_ACC_ZERO,AX=%ld,AY=%ld,AZ=%ld\r\n",
                        (long)calibration->accel_zero_x,
                        (long)calibration->accel_zero_y,
                        (long)calibration->accel_zero_z);
      App_UartSendFormat("MPU_PITCH_ZERO_MDEG=%ld\r\n", (long)calibration->pitch_zero_mdeg);
      App_UartSendText("MPU,CAL=OK\r\n");
      App_AttitudeEstimatorReset();
      return;

    case MOD_IMU_EVENT_NONE:
    default:
      return;
  }
}

static void App_AttitudeEstimateAndReportVofa(void)
{
  mod_imu_report_t imu_report;
  ctrl_attitude_estimator_input_t estimator_input;
  ctrl_attitude_estimator_output_t estimator_output;
  ctrl_angle_loop_input_t angle_input;
  ctrl_angle_loop_output_t angle_output;
  uint32_t now_ms;

  now_ms = HAL_GetTick();
  if (mod_imu_read_report(&g_imu_module, now_ms, &imu_report) != HAL_OK)
  {
    App_UartSendText("MPU,READ=ERR\r\n");
    App_ReportSoftI2cDiag("MPU,READ");
    App_AttitudeEstimatorReset();
    return;
  }

  if (imu_report.calibration_ok == 0U)
  {
    App_AttitudeEstimatorReset();
    return;
  }

  estimator_input.pitch_accel_mdeg = imu_report.pitch_zero_relative_mdeg;
  estimator_input.pitch_rate_lsb = imu_report.gyro_y_zero_relative;
  if (ctrl_attitude_estimator_update(&g_attitude_estimator,
                                     &estimator_input,
                                     now_ms,
                                     &estimator_output) != HAL_OK)
  {
    App_AttitudeEstimatorReset();
    return;
  }

  angle_input.target_pitch_mdeg = APP_ANGLE_LOOP_TARGET_PITCH_MDEG;
  angle_input.pitch_mdeg = estimator_output.pitch_fused_mdeg;
  angle_input.pitch_rate_mdps = estimator_output.pitch_rate_mdps;
  if (ctrl_angle_loop_update(&g_angle_loop, &angle_input, &angle_output) != HAL_OK)
  {
    App_AttitudeEstimatorReset();
    return;
  }

  App_MotorApplyAngleCommand(angle_output.angle_cmd);
  App_VofaSendAngleLoop(&estimator_output, angle_input.target_pitch_mdeg, &angle_output);
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
  App_UartSendFormat("MOTOR,CHAIN=OK,EXEC=%u,ESTOP=%u\r\n",
                     (unsigned int)APP_MOTOR_OUTPUT_ENABLE,
                     (unsigned int)App_IsEstopActive());

  if (mod_imu_init(&g_imu_module, &g_mpu6050, &g_mod_imu_default_config) != HAL_OK)
  {
    Error_Handler();
  }

  if (ctrl_attitude_estimator_init(&g_attitude_estimator,
                                   &g_ctrl_attitude_estimator_default_config) != HAL_OK)
  {
    Error_Handler();
  }

  {
    ctrl_angle_loop_config_t angle_loop_config;

    angle_loop_config = g_ctrl_angle_loop_default_config;
    if (g_imu_module.config.report_interval_ms > 0U)
    {
      angle_loop_config.loop_period_ms = g_imu_module.config.report_interval_ms;
    }

    if (ctrl_angle_loop_init(&g_angle_loop, &angle_loop_config) != HAL_OK)
    {
      Error_Handler();
    }
  }

  mod_imu_reset(&g_imu_module, HAL_GetTick());
  App_AttitudeEstimatorReset();
  g_uart_heartbeat_last_ms = HAL_GetTick();
  g_attitude_report_last_ms = HAL_GetTick();
/* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    uint32_t now_ms;
    mod_imu_event_t imu_event;

    now_ms = HAL_GetTick();
    imu_event = mod_imu_task(&g_imu_module, now_ms);
    App_ImuHandleEvent(imu_event);

    if ((now_ms - g_uart_heartbeat_last_ms) >= 1000U)
    {
      g_uart_heartbeat_last_ms = now_ms;
      App_UartSendText("HB,USART1=OK\r\n");
    }

    if ((mod_imu_is_running(&g_imu_module) != 0U) &&
        (g_imu_module.config.report_interval_ms > 0U) &&
        ((now_ms - g_attitude_report_last_ms) >= g_imu_module.config.report_interval_ms))
    {
      g_attitude_report_last_ms = now_ms;
      App_AttitudeEstimateAndReportVofa();
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
