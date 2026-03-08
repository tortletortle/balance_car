#include "drv_pwm.h"

HAL_StatusTypeDef drv_pwm_init(drv_pwm_t *pwm, TIM_HandleTypeDef *htim, uint32_t channel)
{
    if ((pwm == NULL) || (htim == NULL))
    {
        return HAL_ERROR;
    }

    pwm->htim = htim;
    pwm->channel = channel;
    pwm->period = (uint16_t)__HAL_TIM_GET_AUTORELOAD(htim);
    pwm->started = 0U;
    __HAL_TIM_SET_COMPARE(htim, channel, 0U);
    return HAL_OK;
}

HAL_StatusTypeDef drv_pwm_start(drv_pwm_t *pwm)
{
    if ((pwm == NULL) || (pwm->htim == NULL))
    {
        return HAL_ERROR;
    }

    if (HAL_TIM_PWM_Start(pwm->htim, pwm->channel) != HAL_OK)
    {
        return HAL_ERROR;
    }

    pwm->started = 1U;
    return HAL_OK;
}

HAL_StatusTypeDef drv_pwm_stop(drv_pwm_t *pwm)
{
    if ((pwm == NULL) || (pwm->htim == NULL))
    {
        return HAL_ERROR;
    }

    __HAL_TIM_SET_COMPARE(pwm->htim, pwm->channel, 0U);
    if (HAL_TIM_PWM_Stop(pwm->htim, pwm->channel) != HAL_OK)
    {
        return HAL_ERROR;
    }

    pwm->started = 0U;
    return HAL_OK;
}

HAL_StatusTypeDef drv_pwm_set_compare(drv_pwm_t *pwm, uint16_t compare_value)
{
    if ((pwm == NULL) || (pwm->htim == NULL))
    {
        return HAL_ERROR;
    }

    if (compare_value > pwm->period)
    {
        compare_value = pwm->period;
    }

    __HAL_TIM_SET_COMPARE(pwm->htim, pwm->channel, compare_value);
    return HAL_OK;
}

uint16_t drv_pwm_get_period(const drv_pwm_t *pwm)
{
    if (pwm == NULL)
    {
        return 0U;
    }

    return pwm->period;
}
