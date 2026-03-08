#include "app_command.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static uint16_t app_command_advance_index(uint16_t index)
{
    index++;
    if (index >= APP_COMMAND_RX_FIFO_SIZE)
    {
        index = 0U;
    }
    return index;
}

const app_command_config_t g_app_command_default_config =
{
    63U
};

static void app_command_clear_result(app_command_result_t *result)
{
    if (result == NULL)
    {
        return;
    }

    result->help_requested = 0U;
    result->status_requested = 0U;
    result->clear_fault_requested = 0U;
    result->arm_request_valid = 0U;
    result->arm_request = 0U;
    result->target_pitch_valid = 0U;
    result->target_pitch_mdeg = 0;
    result->vofa_enable_valid = 0U;
    result->vofa_enable = 0U;
    result->line_parsed = 0U;
    result->line_valid = 0U;
}

static void app_command_trim(char *line)
{
    size_t length;
    size_t index;
    size_t start;

    if (line == NULL)
    {
        return;
    }

    length = strlen(line);
    while ((length > 0U) && isspace((unsigned char)line[length - 1U]))
    {
        line[length - 1U] = '\0';
        length--;
    }

    start = 0U;
    while (line[start] != '\0' && isspace((unsigned char)line[start]))
    {
        start++;
    }

    if (start == 0U)
    {
        return;
    }

    for (index = 0U; line[start + index] != '\0'; index++)
    {
        line[index] = line[start + index];
    }
    line[index] = '\0';
}

static void app_command_uppercase(char *line)
{
    size_t index;

    if (line == NULL)
    {
        return;
    }

    for (index = 0U; line[index] != '\0'; index++)
    {
        line[index] = (char)toupper((unsigned char)line[index]);
    }
}

static uint8_t app_command_parse_target(char *line, app_command_result_t *result)
{
    char *arg_text;
    char *end_ptr;
    long value;

    if (strncmp(line, "TARGET", 6U) == 0)
    {
        arg_text = line + 6U;
    }
    else if ((line[0] == 'T') && ((line[1] == ' ') || (line[1] == '\t')))
    {
        arg_text = line + 1U;
    }
    else
    {
        return 0U;
    }

    app_command_trim(arg_text);
    if (arg_text[0] == '\0')
    {
        result->line_valid = 0U;
        return 0U;
    }

    value = strtol(arg_text, &end_ptr, 10);
    if ((end_ptr == arg_text) || (*end_ptr != '\0'))
    {
        result->line_valid = 0U;
        return 0U;
    }

    result->target_pitch_valid = 1U;
    result->target_pitch_mdeg = (int32_t)value;
    return 1U;
}

static uint8_t app_command_parse_vofa(char *line, app_command_result_t *result)
{
    char *arg_text;

    if (strncmp(line, "VOFA", 4U) != 0)
    {
        return 0U;
    }

    arg_text = line + 4U;
    app_command_trim(arg_text);
    if ((strcmp(arg_text, "ON") == 0) || (strcmp(arg_text, "1") == 0))
    {
        result->vofa_enable_valid = 1U;
        result->vofa_enable = 1U;
        return 1U;
    }

    if ((strcmp(arg_text, "OFF") == 0) || (strcmp(arg_text, "0") == 0))
    {
        result->vofa_enable_valid = 1U;
        result->vofa_enable = 0U;
        return 1U;
    }

    result->line_valid = 0U;
    return 0U;
}

static uint8_t app_command_parse_line(char *line, app_command_result_t *result)
{
    app_command_trim(line);
    if (line[0] == '\0')
    {
        return 0U;
    }

    result->line_parsed = 1U;
    result->line_valid = 1U;
    app_command_uppercase(line);

    if ((strcmp(line, "HELP") == 0) || (strcmp(line, "H") == 0) || (strcmp(line, "?") == 0))
    {
        result->help_requested = 1U;
        return 1U;
    }

    if ((strcmp(line, "STATUS") == 0) || (strcmp(line, "S") == 0))
    {
        result->status_requested = 1U;
        return 1U;
    }

    if ((strcmp(line, "ARM") == 0) || (strcmp(line, "A") == 0))
    {
        result->arm_request_valid = 1U;
        result->arm_request = 1U;
        return 1U;
    }

    if ((strcmp(line, "DISARM") == 0) || (strcmp(line, "D") == 0) || (strcmp(line, "STOP") == 0))
    {
        result->arm_request_valid = 1U;
        result->arm_request = 0U;
        return 1U;
    }

    if ((strcmp(line, "CLEAR") == 0) || (strcmp(line, "C") == 0))
    {
        result->clear_fault_requested = 1U;
        return 1U;
    }

    if (app_command_parse_vofa(line, result) != 0U)
    {
        return 1U;
    }

    if (app_command_parse_target(line, result) != 0U)
    {
        return 1U;
    }

    result->line_valid = 0U;
    return 0U;
}

HAL_StatusTypeDef app_command_init(app_command_t *command, const app_command_config_t *config)
{
    if ((command == NULL) || (config == NULL) || (config->max_line_length >= sizeof(command->line_buffer)))
    {
        return HAL_ERROR;
    }

    command->config = *config;
    command->initialized = 1U;
    app_command_reset(command);
    return HAL_OK;
}

void app_command_reset(app_command_t *command)
{
    if (command == NULL)
    {
        return;
    }

    command->line_length = 0U;
    command->rx_write_index = 0U;
    command->rx_read_index = 0U;
    command->rx_overflow_count = 0U;
    command->line_buffer[0] = '\0';
}

void app_command_feed_byte(app_command_t *command, uint8_t rx_byte)
{
    uint16_t next_write_index;

    if ((command == NULL) || (command->initialized == 0U))
    {
        return;
    }

    next_write_index = app_command_advance_index(command->rx_write_index);
    if (next_write_index == command->rx_read_index)
    {
        command->rx_overflow_count++;
        return;
    }

    command->rx_fifo[command->rx_write_index] = rx_byte;
    command->rx_write_index = next_write_index;
}

HAL_StatusTypeDef app_command_poll(app_command_t *command,
                                   UART_HandleTypeDef *huart,
                                   app_command_result_t *result)
{
    uint8_t rx_byte;

    (void)huart;

    if ((command == NULL) || (result == NULL) || (command->initialized == 0U))
    {
        return HAL_ERROR;
    }

    app_command_clear_result(result);
    while (command->rx_read_index != command->rx_write_index)
    {
        rx_byte = command->rx_fifo[command->rx_read_index];
        command->rx_read_index = app_command_advance_index(command->rx_read_index);

        if ((rx_byte == '\r') || (rx_byte == '\n'))
        {
            if (command->line_length == 0U)
            {
                continue;
            }

            command->line_buffer[command->line_length] = '\0';
            app_command_parse_line(command->line_buffer, result);
            command->line_length = 0U;
            command->line_buffer[0] = '\0';
            return HAL_OK;
        }

        if (command->line_length < command->config.max_line_length)
        {
            command->line_buffer[command->line_length++] = (char)rx_byte;
        }
        else
        {
            command->line_length = 0U;
            command->line_buffer[0] = '\0';
            result->line_parsed = 1U;
            result->line_valid = 0U;
            return HAL_OK;
        }
    }

    return HAL_OK;
}
