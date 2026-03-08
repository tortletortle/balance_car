#include "mod_battery_monitor.h"

const mod_battery_monitor_config_t g_mod_battery_monitor_default_config =
{
    8U,
    3U,
    3300U,
    11U,
    1U,
    6600U,
    7000U
};

static uint32_t mod_battery_monitor_scale_mv(uint32_t sense_mv,
                                             uint32_t scale_num,
                                             uint32_t scale_den)
{
    if (scale_den == 0U)
    {
        return 0U;
    }

    return (sense_mv * scale_num) / scale_den;
}

static uint32_t mod_battery_monitor_apply_filter(mod_battery_monitor_t *monitor, uint32_t battery_mv)
{
    uint8_t filter_shift;
    int32_t delta_mv;

    if (monitor->sample_valid == 0U)
    {
        monitor->filtered_battery_mv = battery_mv;
        return battery_mv;
    }

    filter_shift = monitor->config.filter_shift;
    if (filter_shift == 0U)
    {
        monitor->filtered_battery_mv = battery_mv;
        return battery_mv;
    }

    delta_mv = (int32_t)battery_mv - (int32_t)monitor->filtered_battery_mv;
    monitor->filtered_battery_mv = (uint32_t)((int32_t)monitor->filtered_battery_mv + (delta_mv >> filter_shift));
    return monitor->filtered_battery_mv;
}

static uint8_t mod_battery_monitor_eval_undervoltage(mod_battery_monitor_t *monitor, uint32_t filtered_mv)
{
    if (monitor->undervoltage != 0U)
    {
        if (filtered_mv >= monitor->config.recover_mv)
        {
            monitor->undervoltage = 0U;
        }
    }
    else if (filtered_mv <= monitor->config.undervoltage_mv)
    {
        monitor->undervoltage = 1U;
    }

    return monitor->undervoltage;
}

HAL_StatusTypeDef mod_battery_monitor_init(mod_battery_monitor_t *monitor,
                                           drv_adc_t *adc,
                                           const mod_battery_monitor_config_t *config)
{
    if ((monitor == NULL) || (adc == NULL) || (config == NULL) ||
        (config->average_sample_count == 0U) || (config->divider_scale_den == 0U))
    {
        return HAL_ERROR;
    }

    monitor->adc = adc;
    monitor->config = *config;
    monitor->initialized = 1U;
    mod_battery_monitor_reset(monitor);
    return HAL_OK;
}

void mod_battery_monitor_reset(mod_battery_monitor_t *monitor)
{
    if (monitor == NULL)
    {
        return;
    }

    monitor->sample_valid = 0U;
    monitor->filtered_battery_mv = 0U;
    monitor->undervoltage = 0U;
    monitor->last_output.sample_valid = 0U;
    monitor->last_output.raw_value = 0U;
    monitor->last_output.sense_mv = 0U;
    monitor->last_output.battery_mv = 0U;
    monitor->last_output.filtered_battery_mv = 0U;
    monitor->last_output.undervoltage = 0U;
}

HAL_StatusTypeDef mod_battery_monitor_update(mod_battery_monitor_t *monitor,
                                             mod_battery_monitor_output_t *output)
{
    uint16_t raw_value;
    uint32_t sense_mv;
    uint32_t battery_mv;
    uint32_t filtered_mv;

    if ((monitor == NULL) || (monitor->initialized == 0U) || (monitor->adc == NULL))
    {
        return HAL_ERROR;
    }

    if (drv_adc_read_average(monitor->adc, monitor->config.average_sample_count, &raw_value) != HAL_OK)
    {
        return HAL_ERROR;
    }

    sense_mv = drv_adc_raw_to_mv(raw_value, monitor->config.vref_mv);
    battery_mv = mod_battery_monitor_scale_mv(sense_mv,
                                              monitor->config.divider_scale_num,
                                              monitor->config.divider_scale_den);
    filtered_mv = mod_battery_monitor_apply_filter(monitor, battery_mv);
    monitor->sample_valid = 1U;

    monitor->last_output.sample_valid = 1U;
    monitor->last_output.raw_value = raw_value;
    monitor->last_output.sense_mv = sense_mv;
    monitor->last_output.battery_mv = battery_mv;
    monitor->last_output.filtered_battery_mv = filtered_mv;
    monitor->last_output.undervoltage = mod_battery_monitor_eval_undervoltage(monitor, filtered_mv);

    if (output != NULL)
    {
        *output = monitor->last_output;
    }

    return HAL_OK;
}

const mod_battery_monitor_output_t *mod_battery_monitor_get_last_output(
    const mod_battery_monitor_t *monitor)
{
    if (monitor == NULL)
    {
        return NULL;
    }

    return &monitor->last_output;
}
