#include "board.h"

#include <string.h>

#include "adc.h"
#include "main.h"
#include "tim.h"
#include "usart.h"

#define BOARD_DELAY_US_USE_DWT        0U
#define BOARD_DELAY_US_NOP_INNER_LOOP 6U
#define BOARD_IMU_I2C_BIT_DELAY_US    20U
#define BOARD_MOTOR_OUTPUT_ENABLE     0U

static void board_delay_us_init(void)
{
#if BOARD_DELAY_US_USE_DWT == 1U
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0U;
#endif
}

static void board_delay_us(uint32_t delay_us)
{
#if BOARD_DELAY_US_USE_DWT == 1U
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
        for (inner_count = 0U; inner_count < BOARD_DELAY_US_NOP_INNER_LOOP; inner_count++)
        {
            __NOP();
        }
    }
#endif
}

HAL_StatusTypeDef board_build_hw_config(board_hw_config_t *config)
{
    if (config == NULL)
    {
        return HAL_ERROR;
    }

    board_delay_us_init();
    memset(config, 0, sizeof(*config));

    config->hadc_battery = &hadc1;
    config->adc_battery_channel = ADC_CHANNEL_6;
    config->htim_encoder_a = &htim2;
    config->encoder_a_direction = 1;
    config->htim_encoder_b = &htim4;
    config->encoder_b_direction = 1;
    config->htim_pwm = &htim3;
    config->pwm_channel_motor_a = TIM_CHANNEL_4;
    config->pwm_channel_motor_b = TIM_CHANNEL_3;
    config->huart_debug = &huart1;
    config->imu_scl_port = IMU_SCL_GPIO_Port;
    config->imu_scl_pin = IMU_SCL_Pin;
    config->imu_sda_port = IMU_SDA_GPIO_Port;
    config->imu_sda_pin = IMU_SDA_Pin;
    config->estop_port = ESTOP_GPIO_Port;
    config->estop_pin = ESTOP_Pin;
    config->estop_active_low = 1U;
    config->motor_output_enable = BOARD_MOTOR_OUTPUT_ENABLE;
    config->imu_i2c_bit_delay_us = BOARD_IMU_I2C_BIT_DELAY_US;
    config->delay_us = board_delay_us;

    return HAL_OK;
}
