#ifndef CTRL_ANGLE_LOOP_H
#define CTRL_ANGLE_LOOP_H

#include <stdint.h>

#include "stm32f1xx_hal.h"

typedef struct
{
    uint32_t loop_period_ms;
    int16_t kp_q8;
    int16_t ki_q8;
    int16_t kd_q8;
    uint8_t d_gyro_weight_num;
    uint8_t d_gyro_weight_den;
    int16_t d_input_limit_ddeg;
    uint8_t d_filter_alpha_num;
    uint8_t d_filter_alpha_den;
    int32_t i_accum_limit;
    int16_t cmd_limit;
    int16_t out_offset_cmd;
    int16_t out_offset_enable_cmd;
    int16_t error_deadband_ddeg;
} ctrl_angle_loop_config_t;

typedef struct
{
    int32_t target_pitch_mdeg;
    int32_t pitch_mdeg;
    int32_t pitch_rate_mdps;
} ctrl_angle_loop_input_t;

typedef struct
{
    uint8_t initialized;
    int32_t pitch_error_mdeg;
    int32_t pitch_error_ddeg;
    int32_t pitch_actual_ddeg;
    int32_t pitch_delta_ddeg;
    int32_t gyro_delta_ddeg;
    int32_t d_input_raw_ddeg;
    int32_t d_input_filt_ddeg;
    int32_t i_accum;
    int16_t angle_cmd;
} ctrl_angle_loop_output_t;

typedef struct
{
    ctrl_angle_loop_config_t config;
    uint8_t initialized;
    int32_t prev_actual_ddeg;
    int32_t i_accum;
    int32_t d_input_filt_ddeg;
    ctrl_angle_loop_output_t last_output;
} ctrl_angle_loop_t;

extern const ctrl_angle_loop_config_t g_ctrl_angle_loop_default_config;

HAL_StatusTypeDef ctrl_angle_loop_init(ctrl_angle_loop_t *loop,
                                       const ctrl_angle_loop_config_t *config);
void ctrl_angle_loop_reset(ctrl_angle_loop_t *loop);
HAL_StatusTypeDef ctrl_angle_loop_update(ctrl_angle_loop_t *loop,
                                         const ctrl_angle_loop_input_t *input,
                                         ctrl_angle_loop_output_t *output);
const ctrl_angle_loop_output_t *ctrl_angle_loop_get_last_output(const ctrl_angle_loop_t *loop);

#endif
