#ifndef CTRL_ATTITUDE_ESTIMATOR_H
#define CTRL_ATTITUDE_ESTIMATOR_H

#include <stdint.h>

#include "stm32f1xx_hal.h"

typedef struct
{
    float complementary_alpha;
    float pitch_rate_lsb_per_dps;
    uint32_t max_integration_gap_ms;
} ctrl_attitude_estimator_config_t;

typedef struct
{
    int32_t pitch_accel_mdeg;
    int32_t pitch_rate_lsb;
} ctrl_attitude_estimator_input_t;

typedef struct
{
    uint8_t initialized;
    int32_t pitch_accel_mdeg;
    int32_t pitch_gyro_mdeg;
    int32_t pitch_fused_mdeg;
    int32_t pitch_rate_mdps;
} ctrl_attitude_estimator_output_t;

typedef struct
{
    ctrl_attitude_estimator_config_t config;
    uint8_t initialized;
    uint32_t last_update_ms;
    float pitch_gyro_deg;
    float pitch_fused_deg;
    ctrl_attitude_estimator_output_t last_output;
} ctrl_attitude_estimator_t;

extern const ctrl_attitude_estimator_config_t g_ctrl_attitude_estimator_default_config;

HAL_StatusTypeDef ctrl_attitude_estimator_init(ctrl_attitude_estimator_t *estimator,
                                               const ctrl_attitude_estimator_config_t *config);
void ctrl_attitude_estimator_reset(ctrl_attitude_estimator_t *estimator);
HAL_StatusTypeDef ctrl_attitude_estimator_update(ctrl_attitude_estimator_t *estimator,
                                                 const ctrl_attitude_estimator_input_t *input,
                                                 uint32_t now_ms,
                                                 ctrl_attitude_estimator_output_t *output);
const ctrl_attitude_estimator_output_t *ctrl_attitude_estimator_get_last_output(
    const ctrl_attitude_estimator_t *estimator);

#endif