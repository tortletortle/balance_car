#ifndef APP_STATE_MACHINE_H
#define APP_STATE_MACHINE_H

#include <stdint.h>

#include "stm32f1xx_hal.h"

typedef enum
{
    APP_STATE_INIT = 0U,
    APP_STATE_READY,
    APP_STATE_ARMED,
    APP_STATE_FAULT,
    APP_STATE_ESTOP
} app_state_machine_state_t;

typedef struct
{
    uint8_t arm_request;
    uint8_t clear_request;
    uint8_t imu_ready;
    uint8_t estop_active;
    uint8_t fault_latched;
} app_state_machine_input_t;

typedef struct
{
    app_state_machine_state_t current_state;
    app_state_machine_state_t previous_state;
    uint8_t state_changed;
    uint8_t output_allowed;
} app_state_machine_output_t;

typedef struct
{
    uint8_t initialized;
    app_state_machine_output_t last_output;
} app_state_machine_t;

HAL_StatusTypeDef app_state_machine_init(app_state_machine_t *machine);
void app_state_machine_reset(app_state_machine_t *machine);
HAL_StatusTypeDef app_state_machine_update(app_state_machine_t *machine,
                                           const app_state_machine_input_t *input,
                                           app_state_machine_output_t *output);
app_state_machine_state_t app_state_machine_get_state(const app_state_machine_t *machine);
const char *app_state_machine_get_state_name(app_state_machine_state_t state);

#endif
