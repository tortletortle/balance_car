#ifndef MOD_SAFETY_H
#define MOD_SAFETY_H

#include <stdint.h>

#include "stm32f1xx_hal.h"

typedef enum
{
    MOD_SAFETY_FAULT_NONE = 0U,
    MOD_SAFETY_FAULT_ESTOP_ACTIVE = 1U << 0,
    MOD_SAFETY_FAULT_BATTERY_UV = 1U << 1,
    MOD_SAFETY_FAULT_IMU_OFFLINE = 1U << 2,
    MOD_SAFETY_FAULT_IMU_STALE = 1U << 3
} mod_safety_fault_flag_t;

typedef struct
{
    uint8_t latch_faults;
} mod_safety_config_t;

typedef struct
{
    uint8_t estop_active;
    uint8_t battery_undervoltage;
    uint8_t imu_running;
    uint8_t imu_fresh;
    uint8_t clear_latched_request;
} mod_safety_input_t;

typedef struct
{
    uint32_t active_fault_flags;
    uint32_t fault_latched_flags;
    uint8_t fault_latched;
    uint8_t output_allowed;
} mod_safety_output_t;

typedef struct
{
    mod_safety_config_t config;
    uint8_t initialized;
    uint32_t fault_latched_flags;
    mod_safety_output_t last_output;
} mod_safety_t;

extern const mod_safety_config_t g_mod_safety_default_config;

HAL_StatusTypeDef mod_safety_init(mod_safety_t *safety, const mod_safety_config_t *config);
void mod_safety_reset(mod_safety_t *safety);
HAL_StatusTypeDef mod_safety_update(mod_safety_t *safety,
                                    const mod_safety_input_t *input,
                                    mod_safety_output_t *output);
const mod_safety_output_t *mod_safety_get_last_output(const mod_safety_t *safety);

#endif
