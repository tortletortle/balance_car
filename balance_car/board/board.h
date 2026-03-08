#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

#include "stm32f1xx_hal.h"

typedef void (*board_delay_us_fn_t)(uint32_t delay_us);

typedef struct
{
    ADC_HandleTypeDef *hadc_battery;
    uint32_t adc_battery_channel;
    TIM_HandleTypeDef *htim_encoder_a;
    int8_t encoder_a_direction;
    TIM_HandleTypeDef *htim_encoder_b;
    int8_t encoder_b_direction;
    TIM_HandleTypeDef *htim_pwm;
    uint32_t pwm_channel_motor_a;
    uint32_t pwm_channel_motor_b;
    UART_HandleTypeDef *huart_debug;
    GPIO_TypeDef *imu_scl_port;
    uint16_t imu_scl_pin;
    GPIO_TypeDef *imu_sda_port;
    uint16_t imu_sda_pin;
    GPIO_TypeDef *estop_port;
    uint16_t estop_pin;
    uint8_t estop_active_low;
    uint8_t motor_output_enable;
    uint32_t imu_i2c_bit_delay_us;
    board_delay_us_fn_t delay_us;
} board_hw_config_t;

HAL_StatusTypeDef board_build_hw_config(board_hw_config_t *config);

#endif
