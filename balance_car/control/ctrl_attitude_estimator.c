#include "ctrl_attitude_estimator.h"

const ctrl_attitude_estimator_config_t g_ctrl_attitude_estimator_default_config =
{
    0.98f,
    16.4f,
    100U
};

static int32_t ctrl_attitude_estimator_round_to_i32(float value)
{
    if (value >= 0.0f)
    {
        return (int32_t)(value + 0.5f);
    }

    return (int32_t)(value - 0.5f);
}

static void ctrl_attitude_estimator_write_output(ctrl_attitude_estimator_t *estimator,
                                                 float pitch_accel_deg,
                                                 float pitch_rate_dps,
                                                 ctrl_attitude_estimator_output_t *output)
{
    ctrl_attitude_estimator_output_t temp_output;

    temp_output.initialized = estimator->initialized;
    temp_output.pitch_accel_mdeg = ctrl_attitude_estimator_round_to_i32(pitch_accel_deg * 1000.0f);
    temp_output.pitch_gyro_mdeg = ctrl_attitude_estimator_round_to_i32(estimator->pitch_gyro_deg * 1000.0f);
    temp_output.pitch_fused_mdeg = ctrl_attitude_estimator_round_to_i32(estimator->pitch_fused_deg * 1000.0f);
    temp_output.pitch_rate_mdps = ctrl_attitude_estimator_round_to_i32(pitch_rate_dps * 1000.0f);

    estimator->last_output = temp_output;
    if (output != NULL)
    {
        *output = temp_output;
    }
}

HAL_StatusTypeDef ctrl_attitude_estimator_init(ctrl_attitude_estimator_t *estimator,
                                               const ctrl_attitude_estimator_config_t *config)
{
    if ((estimator == NULL) || (config == NULL))
    {
        return HAL_ERROR;
    }

    if ((config->complementary_alpha < 0.0f) ||
        (config->complementary_alpha > 1.0f) ||
        (config->pitch_rate_lsb_per_dps <= 0.0f))
    {
        return HAL_ERROR;
    }

    estimator->config = *config;
    ctrl_attitude_estimator_reset(estimator);
    return HAL_OK;
}

void ctrl_attitude_estimator_reset(ctrl_attitude_estimator_t *estimator)
{
    if (estimator == NULL)
    {
        return;
    }

    estimator->initialized = 0U;
    estimator->last_update_ms = 0U;
    estimator->pitch_gyro_deg = 0.0f;
    estimator->pitch_fused_deg = 0.0f;
    estimator->last_output.initialized = 0U;
    estimator->last_output.pitch_accel_mdeg = 0;
    estimator->last_output.pitch_gyro_mdeg = 0;
    estimator->last_output.pitch_fused_mdeg = 0;
    estimator->last_output.pitch_rate_mdps = 0;
}

HAL_StatusTypeDef ctrl_attitude_estimator_update(ctrl_attitude_estimator_t *estimator,
                                                 const ctrl_attitude_estimator_input_t *input,
                                                 uint32_t now_ms,
                                                 ctrl_attitude_estimator_output_t *output)
{
    float pitch_accel_deg;
    float pitch_rate_dps;
    uint32_t delta_ms;
    float delta_deg;

    if ((estimator == NULL) || (input == NULL))
    {
        return HAL_ERROR;
    }

    pitch_accel_deg = ((float)input->pitch_accel_mdeg) / 1000.0f;
    pitch_rate_dps = ((float)input->pitch_rate_lsb) / estimator->config.pitch_rate_lsb_per_dps;

    if (estimator->initialized == 0U)
    {
        estimator->initialized = 1U;
        estimator->last_update_ms = now_ms;
        estimator->pitch_gyro_deg = pitch_accel_deg;
        estimator->pitch_fused_deg = pitch_accel_deg;
        ctrl_attitude_estimator_write_output(estimator, pitch_accel_deg, pitch_rate_dps, output);
        return HAL_OK;
    }

    delta_ms = now_ms - estimator->last_update_ms;
    estimator->last_update_ms = now_ms;

    if ((estimator->config.max_integration_gap_ms > 0U) &&
        (delta_ms > estimator->config.max_integration_gap_ms))
    {
        estimator->pitch_gyro_deg = pitch_accel_deg;
        estimator->pitch_fused_deg = pitch_accel_deg;
        ctrl_attitude_estimator_write_output(estimator, pitch_accel_deg, pitch_rate_dps, output);
        return HAL_OK;
    }

    delta_deg = pitch_rate_dps * (((float)delta_ms) / 1000.0f);
    estimator->pitch_gyro_deg += delta_deg;
    estimator->pitch_fused_deg =
        (estimator->config.complementary_alpha * (estimator->pitch_fused_deg + delta_deg)) +
        ((1.0f - estimator->config.complementary_alpha) * pitch_accel_deg);

    ctrl_attitude_estimator_write_output(estimator, pitch_accel_deg, pitch_rate_dps, output);
    return HAL_OK;
}

const ctrl_attitude_estimator_output_t *ctrl_attitude_estimator_get_last_output(
    const ctrl_attitude_estimator_t *estimator)
{
    if (estimator == NULL)
    {
        return NULL;
    }

    return &estimator->last_output;
}