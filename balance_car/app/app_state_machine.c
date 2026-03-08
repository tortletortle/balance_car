#include "app_state_machine.h"

static app_state_machine_state_t app_state_machine_next_state(app_state_machine_state_t current_state,
                                                              const app_state_machine_input_t *input)
{
    if (input->estop_active != 0U)
    {
        return APP_STATE_ESTOP;
    }

    if (input->fault_latched != 0U)
    {
        return APP_STATE_FAULT;
    }

    switch (current_state)
    {
        case APP_STATE_INIT:
            return (input->imu_ready != 0U) ? APP_STATE_READY : APP_STATE_INIT;

        case APP_STATE_READY:
            if (input->arm_request != 0U)
            {
                return APP_STATE_ARMED;
            }
            return APP_STATE_READY;

        case APP_STATE_ARMED:
            return (input->arm_request != 0U) ? APP_STATE_ARMED : APP_STATE_READY;

        case APP_STATE_FAULT:
            if ((input->clear_request != 0U) && (input->imu_ready != 0U))
            {
                return APP_STATE_READY;
            }
            return APP_STATE_FAULT;

        case APP_STATE_ESTOP:
            if ((input->estop_active == 0U) && (input->imu_ready != 0U))
            {
                return APP_STATE_READY;
            }
            return APP_STATE_ESTOP;

        default:
            return APP_STATE_INIT;
    }
}

HAL_StatusTypeDef app_state_machine_init(app_state_machine_t *machine)
{
    if (machine == NULL)
    {
        return HAL_ERROR;
    }

    machine->initialized = 1U;
    app_state_machine_reset(machine);
    return HAL_OK;
}

void app_state_machine_reset(app_state_machine_t *machine)
{
    if (machine == NULL)
    {
        return;
    }

    machine->last_output.current_state = APP_STATE_INIT;
    machine->last_output.previous_state = APP_STATE_INIT;
    machine->last_output.state_changed = 0U;
    machine->last_output.output_allowed = 0U;
}

HAL_StatusTypeDef app_state_machine_update(app_state_machine_t *machine,
                                           const app_state_machine_input_t *input,
                                           app_state_machine_output_t *output)
{
    app_state_machine_state_t next_state;

    if ((machine == NULL) || (input == NULL) || (machine->initialized == 0U))
    {
        return HAL_ERROR;
    }

    next_state = app_state_machine_next_state(machine->last_output.current_state, input);
    machine->last_output.previous_state = machine->last_output.current_state;
    machine->last_output.current_state = next_state;
    machine->last_output.state_changed = (next_state != machine->last_output.previous_state) ? 1U : 0U;
    machine->last_output.output_allowed = (next_state == APP_STATE_ARMED) ? 1U : 0U;

    if (output != NULL)
    {
        *output = machine->last_output;
    }

    return HAL_OK;
}

app_state_machine_state_t app_state_machine_get_state(const app_state_machine_t *machine)
{
    if (machine == NULL)
    {
        return APP_STATE_INIT;
    }

    return machine->last_output.current_state;
}

const char *app_state_machine_get_state_name(app_state_machine_state_t state)
{
    switch (state)
    {
        case APP_STATE_INIT:
            return "INIT";
        case APP_STATE_READY:
            return "READY";
        case APP_STATE_ARMED:
            return "ARMED";
        case APP_STATE_FAULT:
            return "FAULT";
        case APP_STATE_ESTOP:
            return "ESTOP";
        default:
            return "UNKNOWN";
    }
}
