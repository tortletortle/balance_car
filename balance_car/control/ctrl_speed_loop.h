#ifndef CTRL_SPEED_LOOP_H
#define CTRL_SPEED_LOOP_H

#include <stdint.h>

#include "stm32f1xx_hal.h"

typedef struct
{
    uint8_t enable_closed_loop;
    int16_t cmd_to_delta_div;
    int8_t encoder_a_sign;
    int8_t encoder_b_sign;
    int16_t kp_q8;
    int16_t ki_q8;
    int32_t i_accum_limit;
    int16_t pwm_limit;
    int16_t startup_pwm_fwd;
    int16_t startup_pwm_rev;
    int16_t startup_assist_min_cmd;
} ctrl_speed_loop_config_t;

typedef struct
{
    int16_t encoder_delta_a;
    int16_t encoder_delta_b;
    int16_t base_cmd;
} ctrl_speed_loop_input_t;

typedef struct
{
    uint8_t initialized;
    int16_t target_delta;
    int16_t measured_delta;
    int16_t error_delta;
    int16_t ff_pwm;
    int16_t correction_pwm;
    int32_t i_accum;
    int16_t pwm_out;
} ctrl_speed_loop_output_t;

typedef struct
{
    ctrl_speed_loop_config_t config;
    uint8_t initialized;
    int32_t i_accum;
    ctrl_speed_loop_output_t last_output;
} ctrl_speed_loop_t;

extern const ctrl_speed_loop_config_t g_ctrl_speed_loop_default_config;

HAL_StatusTypeDef ctrl_speed_loop_init(ctrl_speed_loop_t *loop,
                                       const ctrl_speed_loop_config_t *config);
void ctrl_speed_loop_reset(ctrl_speed_loop_t *loop);
HAL_StatusTypeDef ctrl_speed_loop_update(ctrl_speed_loop_t *loop,
                                         const ctrl_speed_loop_input_t *input,
                                         ctrl_speed_loop_output_t *output);
const ctrl_speed_loop_output_t *ctrl_speed_loop_get_last_output(const ctrl_speed_loop_t *loop);

#endif
