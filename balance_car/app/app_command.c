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
    result->angle_config_requested = 0U;
    result->motor_test_valid = 0U;
    result->motor_test_stop = 0U;
    result->motor_test_update_a = 0U;
    result->motor_test_update_b = 0U;
    result->motor_test_pwm_a = 0;
    result->motor_test_pwm_b = 0;
    result->clear_fault_requested = 0U;
    result->arm_request_valid = 0U;
    result->arm_request = 0U;
    result->target_pitch_valid = 0U;
    result->target_pitch_mdeg = 0;
    result->vofa_enable_valid = 0U;
    result->vofa_enable = 0U;
    result->angle_kp_valid = 0U;
    result->angle_kp_q8 = 0;
    result->angle_ki_valid = 0U;
    result->angle_ki_q8 = 0;
    result->angle_kd_valid = 0U;
    result->angle_kd_q8 = 0;
    result->angle_cmd_limit_valid = 0U;
    result->angle_cmd_limit = 0;
    result->angle_deadband_valid = 0U;
    result->angle_error_deadband_ddeg = 0;
    result->line_parsed = 0U;
    result->line_valid = 0U;
}

static uint8_t app_command_parse_nonnegative_i16(const char *text, int16_t *value)
{
    char *end_ptr;
    long parsed_value;

    if ((text == NULL) || (value == NULL))
    {
        return 0U;
    }

    parsed_value = strtol(text, &end_ptr, 10);
    if ((end_ptr == text) || (*end_ptr != '\0') || (parsed_value < 0L) || (parsed_value > 32767L))
    {
        return 0U;
    }

    *value = (int16_t)parsed_value;
    return 1U;
}

static uint8_t app_command_parse_i16(const char *text, int16_t *value)
{
    char *end_ptr;
    long parsed_value;

    if ((text == NULL) || (value == NULL))
    {
        return 0U;
    }

    parsed_value = strtol(text, &end_ptr, 10);
    if ((end_ptr == text) || (*end_ptr != '\0') || (parsed_value < -32768L) || (parsed_value > 32767L))
    {
        return 0U;
    }

    *value = (int16_t)parsed_value;
    return 1U;
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

static uint8_t app_command_parse_angle(char *line, app_command_result_t *result)
{
    char *arg_text;
    char *value_text;
    int16_t value;

    if (strncmp(line, "ANGLE", 5U) == 0)
    {
        arg_text = line + 5U;
    }
    else if (strncmp(line, "PID", 3U) == 0)
    {
        arg_text = line + 3U;
    }
    else
    {
        return 0U;
    }

    app_command_trim(arg_text);
    if (arg_text[0] == '\0')
    {
        result->angle_config_requested = 1U;
        return 1U;
    }

    value_text = arg_text;
    while ((*value_text != '\0') && !isspace((unsigned char)*value_text))
    {
        value_text++;
    }

    if (*value_text == '\0')
    {
        result->line_valid = 0U;
        return 0U;
    }

    *value_text = '\0';
    value_text++;
    app_command_trim(value_text);
    if ((value_text[0] == '\0') || (app_command_parse_nonnegative_i16(value_text, &value) == 0U))
    {
        result->line_valid = 0U;
        return 0U;
    }

    if (strcmp(arg_text, "KP") == 0)
    {
        result->angle_kp_valid = 1U;
        result->angle_kp_q8 = value;
        return 1U;
    }

    if (strcmp(arg_text, "KI") == 0)
    {
        result->angle_ki_valid = 1U;
        result->angle_ki_q8 = value;
        return 1U;
    }

    if (strcmp(arg_text, "KD") == 0)
    {
        result->angle_kd_valid = 1U;
        result->angle_kd_q8 = value;
        return 1U;
    }

    if (strcmp(arg_text, "LIMIT") == 0)
    {
        result->angle_cmd_limit_valid = 1U;
        result->angle_cmd_limit = value;
        return 1U;
    }

    if (strcmp(arg_text, "DEAD") == 0)
    {
        result->angle_deadband_valid = 1U;
        result->angle_error_deadband_ddeg = value;
        return 1U;
    }

    result->line_valid = 0U;
    return 0U;
}

static uint8_t app_command_parse_motor(char *line, app_command_result_t *result)
{
    char *arg_text;
    char *value_text;
    int16_t value;

    if (strncmp(line, "MOTOR", 5U) == 0)
    {
        arg_text = line + 5U;
    }
    else if (strncmp(line, "WHEEL", 5U) == 0)
    {
        arg_text = line + 5U;
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

    if (strcmp(arg_text, "STOP") == 0)
    {
        result->motor_test_valid = 1U;
        result->motor_test_stop = 1U;
        result->motor_test_update_a = 1U;
        result->motor_test_update_b = 1U;
        result->motor_test_pwm_a = 0;
        result->motor_test_pwm_b = 0;
        return 1U;
    }

    if (app_command_parse_i16(arg_text, &value) != 0U)
    {
        result->motor_test_valid = 1U;
        result->motor_test_update_a = 1U;
        result->motor_test_update_b = 1U;
        result->motor_test_pwm_a = value;
        result->motor_test_pwm_b = value;
        return 1U;
    }

    value_text = arg_text;
    while ((*value_text != '\0') && !isspace((unsigned char)*value_text))
    {
        value_text++;
    }

    if (*value_text == '\0')
    {
        result->line_valid = 0U;
        return 0U;
    }

    *value_text = '\0';
    value_text++;
    app_command_trim(value_text);
    if ((value_text[0] == '\0') || (app_command_parse_i16(value_text, &value) == 0U))
    {
        result->line_valid = 0U;
        return 0U;
    }

    result->motor_test_valid = 1U;
    if ((strcmp(arg_text, "A") == 0) || (strcmp(arg_text, "LEFT") == 0))
    {
        result->motor_test_update_a = 1U;
        result->motor_test_pwm_a = value;
        return 1U;
    }

    if ((strcmp(arg_text, "B") == 0) || (strcmp(arg_text, "RIGHT") == 0))
    {
        result->motor_test_update_b = 1U;
        result->motor_test_pwm_b = value;
        return 1U;
    }

    if ((strcmp(arg_text, "BOTH") == 0) || (strcmp(arg_text, "ALL") == 0))
    {
        result->motor_test_update_a = 1U;
        result->motor_test_update_b = 1U;
        result->motor_test_pwm_a = value;
        result->motor_test_pwm_b = value;
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

    if (app_command_parse_angle(line, result) != 0U)
    {
        return 1U;
    }

    if (app_command_parse_motor(line, result) != 0U)
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
                                   app_command_result_t *result)
{
    uint8_t rx_byte;

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
