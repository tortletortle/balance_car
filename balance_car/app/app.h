#ifndef APP_H
#define APP_H

#include <stdint.h>

#include "stm32f1xx_hal.h"
#include "board.h"
#include "drv_adc.h"
#include "drv_encoder.h"
#include "drv_pwm.h"
#include "drv_soft_i2c.h"
#include "drv_uart.h"
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
    const char *name;
    int8_t encoder_hw_direction_a;
    int8_t encoder_hw_direction_b;
    int8_t encoder_speed_sign_a;
    int8_t encoder_speed_sign_b;
    int8_t motor_command_sign_a;
    int8_t motor_command_sign_b;
    int16_t motor_scale_a_x1000;
    int16_t motor_scale_b_x1000;
    int16_t motor_dir_scale_fwd_x1000;
    int16_t motor_dir_scale_rev_x1000;
    uint8_t speed_enable_closed_loop;
    int16_t speed_cmd_to_delta_div;
    int16_t speed_kp_q8;
    int16_t speed_ki_q8;
    int32_t speed_i_accum_limit;
    int16_t speed_pwm_limit;
    int16_t motor_pwm_limit;
    int16_t motor_deadzone_pwm_fwd;
    int16_t motor_deadzone_pwm_rev;
    int16_t deadzone_comp_min_cmd;
    int16_t motor_ramp_step;
    uint16_t encoder_counts_per_rev;
    uint16_t gear_ratio_x100;
    uint16_t wheel_radius_mm;
} app_motor_profile_t;

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
    app_motor_profile_t motor_profile;
    int32_t initial_target_pitch_mdeg;
    uint32_t imu_stale_timeout_ms;
} app_logic_config_t;

typedef struct
{
    board_hw_config_t hw;
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
    drv_uart_t debug_uart;
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
    uint8_t motor_bench_active;
    uint8_t vofa_enabled;
    uint8_t imu_ready_seen;
    uint8_t uart_rx_it_byte;
    int16_t motor_bench_pwm_a;
    int16_t motor_bench_pwm_b;
    int32_t target_pitch_mdeg;
    uint32_t last_attitude_update_ms;
    int16_t last_encoder_delta_a;
    int16_t last_encoder_delta_b;
    ctrl_attitude_estimator_output_t last_attitude_output;
    ctrl_angle_loop_output_t last_angle_output;
    ctrl_speed_loop_output_t last_speed_output;
} app_t;

extern const app_motor_profile_t g_app_default_motor_profile;

HAL_StatusTypeDef app_logic_config_load_default(app_logic_config_t *config);

HAL_StatusTypeDef app_init(app_t *app, const app_config_t *config, uint32_t now_ms);
void app_reset(app_t *app, uint32_t now_ms);
HAL_StatusTypeDef app_task(app_t *app, uint32_t now_ms);
HAL_StatusTypeDef app_on_uart_rx_cplt(app_t *app, UART_HandleTypeDef *huart);
HAL_StatusTypeDef app_on_uart_error(app_t *app, UART_HandleTypeDef *huart);

#endif
