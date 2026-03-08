#ifndef DRV_PWM_H
#define DRV_PWM_H

#include <stdint.h>

#include "stm32f1xx_hal.h"

typedef struct
{
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    uint16_t period;
    uint8_t started;
} drv_pwm_t;

HAL_StatusTypeDef drv_pwm_init(drv_pwm_t *pwm, TIM_HandleTypeDef *htim, uint32_t channel);
HAL_StatusTypeDef drv_pwm_start(drv_pwm_t *pwm);
HAL_StatusTypeDef drv_pwm_stop(drv_pwm_t *pwm);
HAL_StatusTypeDef drv_pwm_set_compare(drv_pwm_t *pwm, uint16_t compare_value);
uint16_t drv_pwm_get_period(const drv_pwm_t *pwm);

#endif
