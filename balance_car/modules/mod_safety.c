#include "mod_safety.h"

const mod_safety_config_t g_mod_safety_default_config =
{
    1U
};

static uint32_t mod_safety_build_fault_flags(const mod_safety_input_t *input)
{
    uint32_t flags;

    flags = MOD_SAFETY_FAULT_NONE;
    if (input->estop_active != 0U)
    {
        flags |= MOD_SAFETY_FAULT_ESTOP_ACTIVE;
    }

    if (input->battery_undervoltage != 0U)
    {
        flags |= MOD_SAFETY_FAULT_BATTERY_UV;
    }

    if (input->imu_running == 0U)
    {
        flags |= MOD_SAFETY_FAULT_IMU_OFFLINE;
    }
    else if (input->imu_fresh == 0U)
    {
        flags |= MOD_SAFETY_FAULT_IMU_STALE;
    }

    return flags;
}

HAL_StatusTypeDef mod_safety_init(mod_safety_t *safety, const mod_safety_config_t *config)
{
    if ((safety == NULL) || (config == NULL))
    {
        return HAL_ERROR;
    }

    safety->config = *config;
    safety->initialized = 1U;
    mod_safety_reset(safety);
    return HAL_OK;
}

void mod_safety_reset(mod_safety_t *safety)
{
    if (safety == NULL)
    {
        return;
    }

    safety->fault_latched_flags = MOD_SAFETY_FAULT_NONE;
    safety->last_output.active_fault_flags = MOD_SAFETY_FAULT_NONE;
    safety->last_output.fault_latched_flags = MOD_SAFETY_FAULT_NONE;
    safety->last_output.fault_latched = 0U;
    safety->last_output.output_allowed = 0U;
}

HAL_StatusTypeDef mod_safety_update(mod_safety_t *safety,
                                    const mod_safety_input_t *input,
                                    mod_safety_output_t *output)
{
    uint32_t active_fault_flags;

    if ((safety == NULL) || (input == NULL) || (safety->initialized == 0U))
    {
        return HAL_ERROR;
    }

    active_fault_flags = mod_safety_build_fault_flags(input);
    if ((input->clear_latched_request != 0U) && (active_fault_flags == MOD_SAFETY_FAULT_NONE))
    {
        safety->fault_latched_flags = MOD_SAFETY_FAULT_NONE;
    }

    if ((safety->config.latch_faults != 0U) && (active_fault_flags != MOD_SAFETY_FAULT_NONE))
    {
        safety->fault_latched_flags |= active_fault_flags;
    }
    else if (safety->config.latch_faults == 0U)
    {
        safety->fault_latched_flags = active_fault_flags;
    }

    safety->last_output.active_fault_flags = active_fault_flags;
    safety->last_output.fault_latched_flags = safety->fault_latched_flags;
    safety->last_output.fault_latched = (safety->fault_latched_flags != MOD_SAFETY_FAULT_NONE) ? 1U : 0U;
    safety->last_output.output_allowed = (active_fault_flags == MOD_SAFETY_FAULT_NONE) &&
                                         (safety->fault_latched_flags == MOD_SAFETY_FAULT_NONE);

    if (output != NULL)
    {
        *output = safety->last_output;
    }

    return HAL_OK;
}

const mod_safety_output_t *mod_safety_get_last_output(const mod_safety_t *safety)
{
    if (safety == NULL)
    {
        return NULL;
    }

    return &safety->last_output;
}
