#ifndef DRV_ENCODER_H
#define DRV_ENCODER_H

#include <stdint.h>

#include "stm32f1xx_hal.h"

typedef struct
{
    TIM_HandleTypeDef *htim;
    int8_t direction;
} drv_encoder_t;

HAL_StatusTypeDef drv_encoder_init(drv_encoder_t *encoder, TIM_HandleTypeDef *htim, int8_t direction);
HAL_StatusTypeDef drv_encoder_start(drv_encoder_t *encoder);
HAL_StatusTypeDef drv_encoder_stop(drv_encoder_t *encoder);
void drv_encoder_reset(drv_encoder_t *encoder);
int16_t drv_encoder_get_delta(drv_encoder_t *encoder);
int32_t drv_encoder_get_count(const drv_encoder_t *encoder);

#endif
