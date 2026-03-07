#include "drv_adc.h"

static HAL_StatusTypeDef drv_adc_config_channel(drv_adc_t *adc)
{
    ADC_ChannelConfTypeDef channel_config;

    if ((adc == NULL) || (adc->hadc == NULL))
    {
        return HAL_ERROR;
    }

    channel_config.Channel = adc->channel;
    channel_config.Rank = ADC_REGULAR_RANK_1;
    channel_config.SamplingTime = adc->sample_time;

    return HAL_ADC_ConfigChannel(adc->hadc, &channel_config);
}

HAL_StatusTypeDef drv_adc_init(drv_adc_t *adc, ADC_HandleTypeDef *hadc, uint32_t channel)
{
    if ((adc == NULL) || (hadc == NULL))
    {
        return HAL_ERROR;
    }

    adc->hadc = hadc;
    adc->channel = channel;
    adc->sample_time = ADC_SAMPLETIME_239CYCLES_5;
    adc->timeout_ms = 10U;

    if (drv_adc_config_channel(adc) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_ADCEx_Calibration_Start(adc->hadc);
}

HAL_StatusTypeDef drv_adc_read_raw(drv_adc_t *adc, uint16_t *raw_value)
{
    HAL_StatusTypeDef status;

    if ((adc == NULL) || (adc->hadc == NULL) || (raw_value == NULL))
    {
        return HAL_ERROR;
    }

    status = drv_adc_config_channel(adc);
    if (status != HAL_OK)
    {
        return status;
    }

    status = HAL_ADC_Start(adc->hadc);
    if (status != HAL_OK)
    {
        return status;
    }

    status = HAL_ADC_PollForConversion(adc->hadc, adc->timeout_ms);
    if (status == HAL_OK)
    {
        *raw_value = (uint16_t)HAL_ADC_GetValue(adc->hadc);
    }

    (void)HAL_ADC_Stop(adc->hadc);
    return status;
}

HAL_StatusTypeDef drv_adc_read_average(drv_adc_t *adc, uint8_t sample_count, uint16_t *raw_value)
{
    HAL_StatusTypeDef status;
    uint32_t sum;
    uint16_t sample_value;
    uint8_t index;

    if ((raw_value == NULL) || (sample_count == 0U))
    {
        return HAL_ERROR;
    }

    sum = 0U;
    for (index = 0U; index < sample_count; index++)
    {
        status = drv_adc_read_raw(adc, &sample_value);
        if (status != HAL_OK)
        {
            return status;
        }

        sum += sample_value;
    }

    *raw_value = (uint16_t)(sum / sample_count);
    return HAL_OK;
}

uint32_t drv_adc_raw_to_mv(uint16_t raw_value, uint32_t vref_mv)
{
    return ((uint32_t)raw_value * vref_mv) / 4095U;
}