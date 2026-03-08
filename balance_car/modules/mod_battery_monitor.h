#ifndef MOD_BATTERY_MONITOR_H
#define MOD_BATTERY_MONITOR_H

#include <stdint.h>

#include "stm32f1xx_hal.h"
#include "drv_adc.h"

typedef struct
{
    uint8_t average_sample_count;
    uint8_t filter_shift;
    uint32_t vref_mv;
    uint32_t divider_scale_num;
    uint32_t divider_scale_den;
    uint32_t undervoltage_mv;
    uint32_t recover_mv;
} mod_battery_monitor_config_t;

typedef struct
{
    uint8_t sample_valid;
    uint16_t raw_value;
    uint32_t sense_mv;
    uint32_t battery_mv;
    uint32_t filtered_battery_mv;
    uint8_t undervoltage;
} mod_battery_monitor_output_t;

typedef struct
{
    drv_adc_t *adc;
    mod_battery_monitor_config_t config;
    uint8_t initialized;
    uint8_t sample_valid;
    uint32_t filtered_battery_mv;
    uint8_t undervoltage;
    mod_battery_monitor_output_t last_output;
} mod_battery_monitor_t;

extern const mod_battery_monitor_config_t g_mod_battery_monitor_default_config;

HAL_StatusTypeDef mod_battery_monitor_init(mod_battery_monitor_t *monitor,
                                           drv_adc_t *adc,
                                           const mod_battery_monitor_config_t *config);
void mod_battery_monitor_reset(mod_battery_monitor_t *monitor);
HAL_StatusTypeDef mod_battery_monitor_update(mod_battery_monitor_t *monitor,
                                             mod_battery_monitor_output_t *output);
const mod_battery_monitor_output_t *mod_battery_monitor_get_last_output(
    const mod_battery_monitor_t *monitor);

#endif
