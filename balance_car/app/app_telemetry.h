#ifndef APP_TELEMETRY_H
#define APP_TELEMETRY_H

#include <stdint.h>

#include "stm32f1xx_hal.h"
#include "drv_uart.h"
#include "app_command.h"
#include "app_state_machine.h"
#include "mod_imu.h"
#include "mod_battery_monitor.h"
#include "mod_safety.h"
#include "ctrl_attitude_estimator.h"
#include "ctrl_angle_loop.h"
#include "ctrl_speed_loop.h"

typedef struct
{
    drv_uart_t *uart;
} app_telemetry_t;

HAL_StatusTypeDef app_telemetry_init(app_telemetry_t *telemetry, drv_uart_t *uart);
void app_telemetry_send_text(app_telemetry_t *telemetry, const char *text);
void app_telemetry_send_format(app_telemetry_t *telemetry, const char *format, ...);
void app_telemetry_send_boot(app_telemetry_t *telemetry);
void app_telemetry_send_reset_flags(app_telemetry_t *telemetry);
void app_telemetry_send_heartbeat(app_telemetry_t *telemetry);
void app_telemetry_send_help(app_telemetry_t *telemetry);
void app_telemetry_send_angle_config(app_telemetry_t *telemetry,
                                     const ctrl_angle_loop_config_t *config);
void app_telemetry_send_motor_bench(app_telemetry_t *telemetry,
                                    uint8_t active,
                                    int16_t pwm_a,
                                    int16_t pwm_b);
void app_telemetry_send_state(app_telemetry_t *telemetry,
                              app_state_machine_state_t state,
                              uint8_t arm_request,
                              uint32_t active_fault_flags,
                              uint32_t latched_fault_flags);
void app_telemetry_send_status(app_telemetry_t *telemetry,
                               app_state_machine_state_t state,
                               uint8_t arm_request,
                               uint8_t estop_active,
                               const mod_battery_monitor_output_t *battery,
                               const mod_safety_output_t *safety,
                               int32_t target_pitch_mdeg,
                               const ctrl_attitude_estimator_output_t *attitude,
                               const ctrl_angle_loop_output_t *angle,
                               const ctrl_speed_loop_output_t *speed,
                               int16_t encoder_delta_a,
                               int16_t encoder_delta_b);
void app_telemetry_send_command_result(app_telemetry_t *telemetry,
                                       const app_command_result_t *command_result);
void app_telemetry_send_imu_event(app_telemetry_t *telemetry,
                                  mod_imu_t *imu_module,
                                  mod_imu_event_t event);
void app_telemetry_send_angle_loop_vofa(app_telemetry_t *telemetry,
                                        const ctrl_attitude_estimator_output_t *attitude_output,
                                        int32_t target_pitch_mdeg,
                                        const ctrl_angle_loop_output_t *angle_output);

#endif
