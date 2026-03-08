#include "ctrl_speed_loop.h"

const ctrl_speed_loop_config_t g_ctrl_speed_loop_default_config =
{
    0U,
    100,
    1,
    -1,
    2304,
    64,
    4000,
    1780,
    900,
    980,
    80
};

static int16_t ctrl_speed_loop_clamp_i16(int32_t value, int16_t limit)
{
    if (value > limit)
    {
        return limit;
    }

    if (value < -limit)
    {
        return (int16_t)(-limit);
    }

    return (int16_t)value;
}

static int32_t ctrl_speed_loop_clamp_i32(int32_t value, int32_t limit)
{
    if (value > limit)
    {
        return limit;
    }

    if (value < -limit)
    {
        return -limit;
    }

    return value;
}

static int16_t ctrl_speed_loop_apply_startup_assist(const ctrl_speed_loop_t *loop,
                                                    int16_t measured_delta,
                                                    int16_t base_cmd,
                                                    int16_t pwm_out)
{
    int16_t abs_base_cmd;

    abs_base_cmd = (base_cmd >= 0) ? base_cmd : (int16_t)(-base_cmd);
    if (abs_base_cmd < loop->config.startup_assist_min_cmd)
    {
        return pwm_out;
    }

    if ((base_cmd > 0) && (measured_delta <= 0) && (pwm_out > 0) &&
        (pwm_out < loop->config.startup_pwm_fwd))
    {
        return loop->config.startup_pwm_fwd;
    }

    if ((base_cmd < 0) && (measured_delta >= 0) && (pwm_out < 0) &&
        (pwm_out > -loop->config.startup_pwm_rev))
    {
        return (int16_t)(-loop->config.startup_pwm_rev);
    }

    return pwm_out;
}

HAL_StatusTypeDef ctrl_speed_loop_init(ctrl_speed_loop_t *loop,
                                       const ctrl_speed_loop_config_t *config)
{
    if ((loop == NULL) || (config == NULL) || (config->cmd_to_delta_div == 0))
    {
        return HAL_ERROR;
    }

    loop->config = *config;
    loop->initialized = 1U;
    ctrl_speed_loop_reset(loop);
    return HAL_OK;
}

void ctrl_speed_loop_reset(ctrl_speed_loop_t *loop)
{
    if (loop == NULL)
    {
        return;
    }

    loop->i_accum = 0;
    loop->last_output.initialized = 0U;
    loop->last_output.target_delta = 0;
    loop->last_output.measured_delta = 0;
    loop->last_output.error_delta = 0;
    loop->last_output.ff_pwm = 0;
    loop->last_output.correction_pwm = 0;
    loop->last_output.i_accum = 0;
    loop->last_output.pwm_out = 0;
}

HAL_StatusTypeDef ctrl_speed_loop_update(ctrl_speed_loop_t *loop,
                                         const ctrl_speed_loop_input_t *input,
                                         ctrl_speed_loop_output_t *output)
{
    int32_t target_delta;
    int32_t measured_delta;
    int32_t error_delta;
    int32_t correction_pwm;
    int32_t pwm_out;

    if ((loop == NULL) || (input == NULL) || (loop->initialized == 0U))
    {
        return HAL_ERROR;
    }

    target_delta = input->base_cmd / loop->config.cmd_to_delta_div;
    measured_delta = (((int32_t)input->encoder_delta_a * loop->config.encoder_a_sign) +
                      ((int32_t)input->encoder_delta_b * loop->config.encoder_b_sign)) / 2;
    error_delta = target_delta - measured_delta;

    if (loop->config.enable_closed_loop != 0U)
    {
        loop->i_accum += error_delta;
        loop->i_accum = ctrl_speed_loop_clamp_i32(loop->i_accum, loop->config.i_accum_limit);
        correction_pwm = ((int32_t)loop->config.kp_q8 * error_delta) +
                         ((int32_t)loop->config.ki_q8 * loop->i_accum);
        correction_pwm /= 256;
    }
    else
    {
        loop->i_accum = 0;
        correction_pwm = 0;
    }

    pwm_out = input->base_cmd + correction_pwm;
    pwm_out = ctrl_speed_loop_apply_startup_assist(loop,
                                                   (int16_t)measured_delta,
                                                   input->base_cmd,
                                                   (int16_t)pwm_out);
    pwm_out = ctrl_speed_loop_clamp_i16(pwm_out, loop->config.pwm_limit);

    loop->last_output.initialized = 1U;
    loop->last_output.target_delta = (int16_t)target_delta;
    loop->last_output.measured_delta = (int16_t)measured_delta;
    loop->last_output.error_delta = (int16_t)error_delta;
    loop->last_output.ff_pwm = input->base_cmd;
    loop->last_output.correction_pwm = (int16_t)correction_pwm;
    loop->last_output.i_accum = loop->i_accum;
    loop->last_output.pwm_out = (int16_t)pwm_out;

    if (output != NULL)
    {
        *output = loop->last_output;
    }

    return HAL_OK;
}

const ctrl_speed_loop_output_t *ctrl_speed_loop_get_last_output(const ctrl_speed_loop_t *loop)
{
    if (loop == NULL)
    {
        return NULL;
    }

    return &loop->last_output;
}
