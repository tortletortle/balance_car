#include "mod_motor.h"

const mod_motor_config_t g_mod_motor_default_config =
{
    1800,
    70,
    1,
    1,
    1000,
    1000
};

static int16_t mod_motor_clamp_i16(int32_t value, int16_t min_value, int16_t max_value)
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

static int16_t mod_motor_step_toward_i16(int16_t current, int16_t target, int16_t step)
{
    if (current < target)
    {
        current = (int16_t)(current + step);
        if (current > target)
        {
            current = target;
        }
    }
    else if (current > target)
    {
        current = (int16_t)(current - step);
        if (current < target)
        {
            current = target;
        }
    }

    return current;
}

static void mod_motor_write_output(mod_motor_t *motor, mod_motor_output_t *output)
{
    mod_motor_output_t temp_output;

    temp_output.output_enabled = motor->output_enabled;
    temp_output.target_pwm_motor_a = motor->target_pwm_motor_a;
    temp_output.target_pwm_motor_b = motor->target_pwm_motor_b;
    temp_output.applied_pwm_motor_a = motor->applied_pwm_motor_a;
    temp_output.applied_pwm_motor_b = motor->applied_pwm_motor_b;

    motor->last_output = temp_output;
    if (output != NULL)
    {
        *output = temp_output;
    }
}

HAL_StatusTypeDef mod_motor_init(mod_motor_t *motor,
                                 dev_tb6612_t *driver,
                                 const mod_motor_config_t *config)
{
    if ((motor == NULL) || (driver == NULL) || (config == NULL))
    {
        return HAL_ERROR;
    }

    if ((config->pwm_limit <= 0) || (config->ramp_step <= 0) ||
        ((config->command_sign_motor_a != 1) && (config->command_sign_motor_a != -1)) ||
        ((config->command_sign_motor_b != 1) && (config->command_sign_motor_b != -1)) ||
        (config->command_scale_a_x1000 <= 0) ||
        (config->command_scale_b_x1000 <= 0))
    {
        return HAL_ERROR;
    }

    motor->driver = driver;
    motor->config = *config;
    mod_motor_reset(motor);
    return HAL_OK;
}

void mod_motor_reset(mod_motor_t *motor)
{
    if (motor == NULL)
    {
        return;
    }

    motor->output_enabled = 0U;
    motor->target_pwm_motor_a = 0;
    motor->target_pwm_motor_b = 0;
    motor->applied_pwm_motor_a = 0;
    motor->applied_pwm_motor_b = 0;
    motor->last_output.output_enabled = 0U;
    motor->last_output.target_pwm_motor_a = 0;
    motor->last_output.target_pwm_motor_b = 0;
    motor->last_output.applied_pwm_motor_a = 0;
    motor->last_output.applied_pwm_motor_b = 0;

    if (motor->driver != NULL)
    {
        dev_tb6612_stop(motor->driver);
    }
}

void mod_motor_set_enable(mod_motor_t *motor, uint8_t enable)
{
    if (motor == NULL)
    {
        return;
    }

    motor->output_enabled = (enable != 0U) ? 1U : 0U;
    if ((motor->output_enabled == 0U) && (motor->driver != NULL))
    {
        motor->applied_pwm_motor_a = 0;
        motor->applied_pwm_motor_b = 0;
        dev_tb6612_stop(motor->driver);
    }
}

void mod_motor_set_targets(mod_motor_t *motor, int16_t target_pwm_motor_a, int16_t target_pwm_motor_b)
{
    if (motor == NULL)
    {
        return;
    }

    target_pwm_motor_a = (int16_t)(target_pwm_motor_a * motor->config.command_sign_motor_a);
    target_pwm_motor_b = (int16_t)(target_pwm_motor_b * motor->config.command_sign_motor_b);
    target_pwm_motor_a = (int16_t)(((int32_t)target_pwm_motor_a * motor->config.command_scale_a_x1000) / 1000);
    target_pwm_motor_b = (int16_t)(((int32_t)target_pwm_motor_b * motor->config.command_scale_b_x1000) / 1000);
    motor->target_pwm_motor_a = mod_motor_clamp_i16(target_pwm_motor_a,
                                                    (int16_t)(-motor->config.pwm_limit),
                                                    motor->config.pwm_limit);
    motor->target_pwm_motor_b = mod_motor_clamp_i16(target_pwm_motor_b,
                                                    (int16_t)(-motor->config.pwm_limit),
                                                    motor->config.pwm_limit);
}

void mod_motor_set_common_target(mod_motor_t *motor, int16_t target_pwm)
{
    mod_motor_set_targets(motor, target_pwm, target_pwm);
}

HAL_StatusTypeDef mod_motor_update(mod_motor_t *motor, mod_motor_output_t *output)
{
    if ((motor == NULL) || (motor->driver == NULL))
    {
        return HAL_ERROR;
    }

    if (motor->output_enabled == 0U)
    {
        motor->applied_pwm_motor_a = 0;
        motor->applied_pwm_motor_b = 0;
        dev_tb6612_stop(motor->driver);
        mod_motor_write_output(motor, output);
        return HAL_OK;
    }

    motor->applied_pwm_motor_a = mod_motor_step_toward_i16(motor->applied_pwm_motor_a,
                                                           motor->target_pwm_motor_a,
                                                           motor->config.ramp_step);
    motor->applied_pwm_motor_b = mod_motor_step_toward_i16(motor->applied_pwm_motor_b,
                                                           motor->target_pwm_motor_b,
                                                           motor->config.ramp_step);

    dev_tb6612_set_enable(motor->driver, 1U);
    if (dev_tb6612_set_dual_pwm(motor->driver,
                                motor->applied_pwm_motor_a,
                                motor->applied_pwm_motor_b) != HAL_OK)
    {
        return HAL_ERROR;
    }

    mod_motor_write_output(motor, output);
    return HAL_OK;
}

const mod_motor_output_t *mod_motor_get_last_output(const mod_motor_t *motor)
{
    if (motor == NULL)
    {
        return NULL;
    }

    return &motor->last_output;
}
