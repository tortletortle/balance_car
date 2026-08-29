#include "ctrl_angle_loop.h"

const ctrl_angle_loop_config_t g_ctrl_angle_loop_default_config =
{
    5U,
    3200,
    16,
    1280,
    8U,
    10U,
    140,
    6U,
    10U,
    600,
    1780,
    20,
    20,
    2
};

static int32_t ctrl_angle_loop_abs_i32(int32_t value)
{
    if (value >= 0)
    {
        return value;
    }

    return -value;
}

static int32_t ctrl_angle_loop_clamp_i32(int32_t value, int32_t min_value, int32_t max_value)
{
    if (value < min_value)
    {
        return min_value;
    }

    if (value > max_value)
    {
        return max_value;
    }

    return value;
}

static int16_t ctrl_angle_loop_clamp_i16(int32_t value, int16_t min_value, int16_t max_value)
{
    if (value < min_value)
    {
        return min_value;
    }

    if (value > max_value)
    {
        return max_value;
    }

    return (int16_t)value;
}

static int32_t ctrl_angle_loop_round_div_i64(int64_t numerator, int32_t denominator)
{
    int64_t half_denominator;

    if (denominator <= 0)
    {
        return 0;
    }

    half_denominator = denominator / 2;
    if (numerator >= 0)
    {
        return (int32_t)((numerator + half_denominator) / denominator);
    }

    return (int32_t)((numerator - half_denominator) / denominator);
}

static void ctrl_angle_loop_write_output(ctrl_angle_loop_t *loop,
                                         int32_t pitch_error_mdeg,
                                         int32_t pitch_error_ddeg,
                                         int32_t pitch_actual_ddeg,
                                         int32_t pitch_delta_ddeg,
                                         int32_t gyro_delta_ddeg,
                                         int32_t d_input_raw_ddeg,
                                         int32_t angle_cmd,
                                         ctrl_angle_loop_output_t *output)
{
    ctrl_angle_loop_output_t temp_output;

    temp_output.initialized = loop->initialized;
    temp_output.pitch_error_mdeg = pitch_error_mdeg;
    temp_output.pitch_error_ddeg = pitch_error_ddeg;
    temp_output.pitch_actual_ddeg = pitch_actual_ddeg;
    temp_output.pitch_delta_ddeg = pitch_delta_ddeg;
    temp_output.gyro_delta_ddeg = gyro_delta_ddeg;
    temp_output.d_input_raw_ddeg = d_input_raw_ddeg;
    temp_output.d_input_filt_ddeg = loop->d_input_filt_ddeg;
    temp_output.i_accum = loop->i_accum;
    temp_output.angle_cmd = (int16_t)angle_cmd;

    loop->last_output = temp_output;
    if (output != NULL)
    {
        *output = temp_output;
    }
}

HAL_StatusTypeDef ctrl_angle_loop_init(ctrl_angle_loop_t *loop,
                                       const ctrl_angle_loop_config_t *config)
{
    if ((loop == NULL) || (config == NULL))
    {
        return HAL_ERROR;
    }

    if ((config->loop_period_ms == 0U) ||
        (config->kp_q8 < 0) ||
        (config->ki_q8 < 0) ||
        (config->kd_q8 < 0) ||
        (config->d_gyro_weight_den == 0U) ||
        (config->d_gyro_weight_num > config->d_gyro_weight_den) ||
        (config->d_input_limit_ddeg < 0) ||
        (config->d_filter_alpha_den == 0U) ||
        (config->d_filter_alpha_num > config->d_filter_alpha_den) ||
        (config->i_accum_limit < 0) ||
        (config->cmd_limit < 0) ||
        (config->out_offset_cmd < 0) ||
        (config->out_offset_enable_cmd < 0) ||
        (config->error_deadband_ddeg < 0))
    {
        return HAL_ERROR;
    }

    loop->config = *config;
    ctrl_angle_loop_reset(loop);
    return HAL_OK;
}

void ctrl_angle_loop_reset(ctrl_angle_loop_t *loop)
{
    if (loop == NULL)
    {
        return;
    }

    loop->initialized = 0U;
    loop->prev_actual_ddeg = 0;
    loop->i_accum = 0;
    loop->d_input_filt_ddeg = 0;
    loop->last_output.initialized = 0U;
    loop->last_output.pitch_error_mdeg = 0;
    loop->last_output.pitch_error_ddeg = 0;
    loop->last_output.pitch_actual_ddeg = 0;
    loop->last_output.pitch_delta_ddeg = 0;
    loop->last_output.gyro_delta_ddeg = 0;
    loop->last_output.d_input_raw_ddeg = 0;
    loop->last_output.d_input_filt_ddeg = 0;
    loop->last_output.i_accum = 0;
    loop->last_output.angle_cmd = 0;
}

HAL_StatusTypeDef ctrl_angle_loop_update(ctrl_angle_loop_t *loop,
                                         const ctrl_angle_loop_input_t *input,
                                         ctrl_angle_loop_output_t *output)
{
    int32_t pitch_error_mdeg;
    int32_t pitch_error_ddeg;
    int32_t pitch_actual_ddeg;
    int32_t pitch_delta_ddeg;
    int32_t gyro_delta_ddeg;
    int32_t d_input_raw_ddeg;
    int32_t out_cmd;
    uint8_t first_update;

    if ((loop == NULL) || (input == NULL))
    {
        return HAL_ERROR;
    }

    pitch_error_mdeg = input->target_pitch_mdeg - input->pitch_mdeg;
    pitch_error_ddeg = ctrl_angle_loop_round_div_i64(pitch_error_mdeg, 50);
    pitch_actual_ddeg = ctrl_angle_loop_round_div_i64(input->pitch_mdeg, 50);
    first_update = (loop->initialized == 0U) ? 1U : 0U;

    if (first_update != 0U)
    {
        loop->initialized = 1U;
        loop->prev_actual_ddeg = pitch_actual_ddeg;
        pitch_delta_ddeg = 0;
        gyro_delta_ddeg = 0;
    }
    else
    {
        pitch_delta_ddeg = pitch_actual_ddeg - loop->prev_actual_ddeg;
        gyro_delta_ddeg = ctrl_angle_loop_round_div_i64(
            (int64_t)input->pitch_rate_mdps * (int64_t)loop->config.loop_period_ms,
            50000);
    }

    if (ctrl_angle_loop_abs_i32(pitch_error_ddeg) <= loop->config.error_deadband_ddeg)
    {
        pitch_error_ddeg = 0;
    }

    if (loop->config.ki_q8 != 0)
    {
        loop->i_accum += pitch_error_ddeg;
    }
    else
    {
        loop->i_accum = 0;
    }

    loop->i_accum = ctrl_angle_loop_clamp_i32(loop->i_accum,
                                              -loop->config.i_accum_limit,
                                              loop->config.i_accum_limit);

    d_input_raw_ddeg =
        ((int32_t)loop->config.d_gyro_weight_num * gyro_delta_ddeg +
         (int32_t)(loop->config.d_gyro_weight_den - loop->config.d_gyro_weight_num) * pitch_delta_ddeg) /
        (int32_t)loop->config.d_gyro_weight_den;
    d_input_raw_ddeg = ctrl_angle_loop_clamp_i32(d_input_raw_ddeg,
                                                 -loop->config.d_input_limit_ddeg,
                                                 loop->config.d_input_limit_ddeg);

    loop->d_input_filt_ddeg =
        ((int32_t)(loop->config.d_filter_alpha_den - loop->config.d_filter_alpha_num) * loop->d_input_filt_ddeg +
         (int32_t)loop->config.d_filter_alpha_num * d_input_raw_ddeg) /
        (int32_t)loop->config.d_filter_alpha_den;

    out_cmd = ((int32_t)loop->config.kp_q8 * pitch_error_ddeg +
               (int32_t)loop->config.ki_q8 * loop->i_accum -
               (int32_t)loop->config.kd_q8 * loop->d_input_filt_ddeg) / 256;

    if (out_cmd > loop->config.out_offset_enable_cmd)
    {
        out_cmd += loop->config.out_offset_cmd;
    }
    else if (out_cmd < (-loop->config.out_offset_enable_cmd))
    {
        out_cmd -= loop->config.out_offset_cmd;
    }

    out_cmd = ctrl_angle_loop_clamp_i16(out_cmd,
                                        (int16_t)(-loop->config.cmd_limit),
                                        loop->config.cmd_limit);

    loop->prev_actual_ddeg = pitch_actual_ddeg;

    ctrl_angle_loop_write_output(loop,
                                 pitch_error_mdeg,
                                 pitch_error_ddeg,
                                 pitch_actual_ddeg,
                                 pitch_delta_ddeg,
                                 gyro_delta_ddeg,
                                 d_input_raw_ddeg,
                                 out_cmd,
                                 output);
    return HAL_OK;
}

const ctrl_angle_loop_output_t *ctrl_angle_loop_get_last_output(const ctrl_angle_loop_t *loop)
{
    if (loop == NULL)
    {
        return NULL;
    }

    return &loop->last_output;
}
