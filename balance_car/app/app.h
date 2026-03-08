#ifndef APP_H
#define APP_H

#include <stdint.h>

#include "stm32f1xx_hal.h"
#include "drv_adc.h"
#include "drv_encoder.h"
#include "drv_pwm.h"
#include "drv_soft_i2c.h"
#include "dev_mpu6050.h"
#include "dev_tb6612.h"
#include "mod_imu.h"
#include "mod_motor.h"
#include "mod_battery_monitor.h"
#include "mod_safety.h"
#include "ctrl_attitude_estimator.h"
#include "ctrl_angle_loop.h"
#include "ctrl_speed_loop.h"
#include "app_scheduler.h"
#include "app_state_machine.h"
#include "app_command.h"
#include "app_telemetry.h"

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
    void (*delay_us)(uint32_t delay_us);
} app_hw_config_t;

typedef struct
{
    mod_imu_config_t imu_config;
    ctrl_attitude_estimator_config_t attitude_config;
    ctrl_angle_loop_config_t angle_loop_config;
    ctrl_speed_loop_config_t speed_loop_config;
    mod_motor_config_t motor_config;
    mod_battery_monitor_config_t battery_config;
    mod_safety_config_t safety_config;
    app_scheduler_config_t scheduler_config;
    app_command_config_t command_config;
    int32_t initial_target_pitch_mdeg;
    uint32_t imu_stale_timeout_ms;
} app_logic_config_t;

typedef struct
{
    app_hw_config_t hw;
    app_logic_config_t logic;
} app_config_t;

typedef struct
{
    app_config_t config;
    drv_adc_t battery_adc;
    drv_encoder_t encoder_a;
    drv_encoder_t encoder_b;
    drv_pwm_t motor_pwm_a;
    drv_pwm_t motor_pwm_b;
    drv_soft_i2c_bus_t imu_soft_i2c_bus;
    dev_mpu6050_t mpu6050;
    dev_tb6612_t tb6612;
    mod_imu_t imu_module;
    mod_motor_t motor_module;
    mod_battery_monitor_t battery_module;
    mod_safety_t safety_module;
    ctrl_attitude_estimator_t attitude_estimator;
    ctrl_angle_loop_t angle_loop;
    ctrl_speed_loop_t speed_loop;
    app_scheduler_t scheduler;
    app_state_machine_t state_machine;
    app_command_t command;
    app_telemetry_t telemetry;
    uint8_t battery_ready;
    uint8_t encoders_ready;
    uint8_t arm_request;
    uint8_t vofa_enabled;
    uint8_t imu_ready_seen;
    uint8_t uart_rx_it_byte;
    int32_t target_pitch_mdeg;
    uint32_t last_attitude_update_ms;
    ctrl_attitude_estimator_output_t last_attitude_output;
    ctrl_angle_loop_output_t last_angle_output;
    ctrl_speed_loop_output_t last_speed_output;
} app_t;

extern const app_logic_config_t g_app_default_logic_config;

HAL_StatusTypeDef app_init(app_t *app, const app_config_t *config, uint32_t now_ms);
void app_reset(app_t *app, uint32_t now_ms);
HAL_StatusTypeDef app_task(app_t *app, uint32_t now_ms);
HAL_StatusTypeDef app_on_uart_rx_cplt(app_t *app, UART_HandleTypeDef *huart);
HAL_StatusTypeDef app_on_uart_error(app_t *app, UART_HandleTypeDef *huart);

#endif
