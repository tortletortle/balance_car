#ifndef MOD_MOTOR_H
#define MOD_MOTOR_H

#include <stdint.h>

#include "stm32f1xx_hal.h"
#include "dev_tb6612.h"

typedef struct
{
    int16_t pwm_limit;
    int16_t ramp_step;
    int8_t command_sign_motor_a;
    int8_t command_sign_motor_b;
} mod_motor_config_t;

typedef struct
{
    uint8_t output_enabled;
    int16_t target_pwm_motor_a;
    int16_t target_pwm_motor_b;
    int16_t applied_pwm_motor_a;
    int16_t applied_pwm_motor_b;
} mod_motor_output_t;

typedef struct
{
    dev_tb6612_t *driver;
    mod_motor_config_t config;
    uint8_t output_enabled;
    int16_t target_pwm_motor_a;
    int16_t target_pwm_motor_b;
    int16_t applied_pwm_motor_a;
    int16_t applied_pwm_motor_b;
    mod_motor_output_t last_output;
} mod_motor_t;

extern const mod_motor_config_t g_mod_motor_default_config;

HAL_StatusTypeDef mod_motor_init(mod_motor_t *motor,
                                 dev_tb6612_t *driver,
                                 const mod_motor_config_t *config);
void mod_motor_reset(mod_motor_t *motor);
void mod_motor_set_enable(mod_motor_t *motor, uint8_t enable);
void mod_motor_set_targets(mod_motor_t *motor, int16_t target_pwm_motor_a, int16_t target_pwm_motor_b);
void mod_motor_set_common_target(mod_motor_t *motor, int16_t target_pwm);
HAL_StatusTypeDef mod_motor_update(mod_motor_t *motor, mod_motor_output_t *output);
const mod_motor_output_t *mod_motor_get_last_output(const mod_motor_t *motor);

#endif
