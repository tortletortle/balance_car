#ifndef APP_COMMAND_H
#define APP_COMMAND_H

#include <stdint.h>

#include "stm32f1xx_hal.h"

typedef struct
{
    uint16_t max_line_length;
} app_command_config_t;

typedef struct
{
    uint8_t help_requested;
    uint8_t status_requested;
    uint8_t clear_fault_requested;
    uint8_t arm_request_valid;
    uint8_t arm_request;
    uint8_t target_pitch_valid;
    int32_t target_pitch_mdeg;
    uint8_t vofa_enable_valid;
    uint8_t vofa_enable;
    uint8_t line_parsed;
    uint8_t line_valid;
} app_command_result_t;

typedef struct
{
    app_command_config_t config;
    uint8_t initialized;
    uint16_t line_length;
    char line_buffer[64];
} app_command_t;

extern const app_command_config_t g_app_command_default_config;

HAL_StatusTypeDef app_command_init(app_command_t *command, const app_command_config_t *config);
void app_command_reset(app_command_t *command);
HAL_StatusTypeDef app_command_poll(app_command_t *command,
                                   UART_HandleTypeDef *huart,
                                   app_command_result_t *result);

#endif
