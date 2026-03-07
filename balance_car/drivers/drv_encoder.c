#include "drv_encoder.h"

static int16_t drv_encoder_apply_direction(const drv_encoder_t *encoder, int16_t count)
{
    if ((encoder != NULL) && (encoder->direction < 0))
    {
        return (int16_t)(-count);
    }

    return count;
}

HAL_StatusTypeDef drv_encoder_init(drv_encoder_t *encoder, TIM_HandleTypeDef *htim, int8_t direction)
{
    if ((encoder == NULL) || (htim == NULL))
    {
        return HAL_ERROR;
    }

    encoder->htim = htim;
    encoder->direction = (direction < 0) ? -1 : 1;
    drv_encoder_reset(encoder);

    return HAL_OK;
}

HAL_StatusTypeDef drv_encoder_start(drv_encoder_t *encoder)
{
    if ((encoder == NULL) || (encoder->htim == NULL))
    {
        return HAL_ERROR;
    }

    drv_encoder_reset(encoder);

    return HAL_TIM_Encoder_Start(encoder->htim, TIM_CHANNEL_ALL);
}

HAL_StatusTypeDef drv_encoder_stop(drv_encoder_t *encoder)
{
    if ((encoder == NULL) || (encoder->htim == NULL))
    {
        return HAL_ERROR;
    }

    return HAL_TIM_Encoder_Stop(encoder->htim, TIM_CHANNEL_ALL);
}

void drv_encoder_reset(drv_encoder_t *encoder)
{
    if ((encoder == NULL) || (encoder->htim == NULL))
    {
        return;
    }

    __HAL_TIM_SET_COUNTER(encoder->htim, 0U);
}

int16_t drv_encoder_get_delta(drv_encoder_t *encoder)
{
    int16_t delta_count;

    if ((encoder == NULL) || (encoder->htim == NULL))
    {
        return 0;
    }

    delta_count = (int16_t)__HAL_TIM_GET_COUNTER(encoder->htim);
    __HAL_TIM_SET_COUNTER(encoder->htim, 0U);

    return drv_encoder_apply_direction(encoder, delta_count);
}

int32_t drv_encoder_get_count(const drv_encoder_t *encoder)
{
    if ((encoder == NULL) || (encoder->htim == NULL))
    {
        return 0;
    }

    return (int32_t)drv_encoder_apply_direction(encoder, (int16_t)__HAL_TIM_GET_COUNTER(encoder->htim));
}
