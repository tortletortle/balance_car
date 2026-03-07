#ifndef DRV_ADC_H
#define DRV_ADC_H

#include <stdint.h>

#include "stm32f1xx_hal.h"

typedef struct
{
    ADC_HandleTypeDef *hadc;
    uint32_t channel;
    uint32_t sample_time;
    uint32_t timeout_ms;
} drv_adc_t;

HAL_StatusTypeDef drv_adc_init(drv_adc_t *adc, ADC_HandleTypeDef *hadc, uint32_t channel);
HAL_StatusTypeDef drv_adc_read_raw(drv_adc_t *adc, uint16_t *raw_value);
HAL_StatusTypeDef drv_adc_read_average(drv_adc_t *adc, uint8_t sample_count, uint16_t *raw_value);
uint32_t drv_adc_raw_to_mv(uint16_t raw_value, uint32_t vref_mv);

#endif