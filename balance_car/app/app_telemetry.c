#include "app_telemetry.h"

#include <stdarg.h>
#include <stdio.h>

#include "drv_soft_i2c.h"

static int app_telemetry_format_signed_milli(char *buffer, uint32_t buffer_size, int32_t value)
{
    uint32_t absolute_value;

    if ((buffer == NULL) || (buffer_size == 0U))
    {
        return -1;
    }

    if (value < 0)
    {
        absolute_value = (uint32_t)(-((int64_t)value));
        return snprintf(buffer,
                        (size_t)buffer_size,
                        "-%lu.%03lu",
                        (unsigned long)(absolute_value / 1000U),
                        (unsigned long)(absolute_value % 1000U));
    }

    absolute_value = (uint32_t)value;
    return snprintf(buffer,
                    (size_t)buffer_size,
                    "%lu.%03lu",
                    (unsigned long)(absolute_value / 1000U),
                    (unsigned long)(absolute_value % 1000U));
}

static const char *app_telemetry_i2c_stage_text(uint8_t stage)
{
    switch ((drv_soft_i2c_diag_stage_t)stage)
    {
        case DRV_SOFT_I2C_DIAG_STAGE_READ_START_WRITE:
            return "RD_ST1";
        case DRV_SOFT_I2C_DIAG_STAGE_READ_ADDR_WRITE:
            return "RD_AW";
        case DRV_SOFT_I2C_DIAG_STAGE_READ_REG:
            return "RD_REG";
        case DRV_SOFT_I2C_DIAG_STAGE_READ_REG_ACK:
            return "RD_RACK";
        case DRV_SOFT_I2C_DIAG_STAGE_READ_START_READ:
            return "RD_ST2";
        case DRV_SOFT_I2C_DIAG_STAGE_READ_ADDR_READ_SEND:
            return "RD_ARS";
        case DRV_SOFT_I2C_DIAG_STAGE_READ_ADDR_READ_ACK:
            return "RD_ARA";
        case DRV_SOFT_I2C_DIAG_STAGE_READ_DATA:
            return "RD_DATA";
        case DRV_SOFT_I2C_DIAG_STAGE_READ_DATA_ACK:
            return "RD_WACK";
        case DRV_SOFT_I2C_DIAG_STAGE_READ_STOP:
            return "RD_STOP";
        case DRV_SOFT_I2C_DIAG_STAGE_READ_DONE:
            return "RD_DONE";
        case DRV_SOFT_I2C_DIAG_STAGE_IDLE:
        default:
            return "IDLE";
    }
}

static void app_telemetry_send_soft_i2c_diag(app_telemetry_t *telemetry, const char *prefix)
{
    drv_soft_i2c_diag_t diag;

    diag = drv_soft_i2c_diag_get();
    app_telemetry_send_format(telemetry,
                              "%s,DIAG=%s(%u),ACK=%u,IDX=%u,LEN=%u,REG=0x%02X,ADDR=0x%02X\r\n",
                              (prefix != NULL) ? prefix : "I2C",
                              app_telemetry_i2c_stage_text(diag.stage),
                              diag.stage,
                              diag.ack_bit,
                              diag.index,
                              diag.length,
                              diag.reg_addr,
                              diag.device_addr7);
}

static void app_telemetry_send_cal_index(app_telemetry_t *telemetry, uint32_t sample_index)
{
    char text[] = "MPU,CAL,IDX=000\r\n";
    uint32_t value;

    value = sample_index + 1U;
    if (value > 999U)
    {
        value = 999U;
    }

    text[12] = (char)('0' + ((value / 100U) % 10U));
    text[13] = (char)('0' + ((value / 10U) % 10U));
    text[14] = (char)('0' + (value % 10U));
    app_telemetry_send_text(telemetry, text);
}

HAL_StatusTypeDef app_telemetry_init(app_telemetry_t *telemetry, drv_uart_t *uart)
{
    if ((telemetry == NULL) || (uart == NULL))
    {
        return HAL_ERROR;
    }

    telemetry->uart = uart;
    return HAL_OK;
}

void app_telemetry_send_text(app_telemetry_t *telemetry, const char *text)
{
    uint16_t length;

    if ((telemetry == NULL) || (telemetry->uart == NULL) || (text == NULL))
    {
        return;
    }

    length = 0U;
    while (text[length] != '\0')
    {
        length++;
    }

    if (length > 0U)
    {
        (void)drv_uart_transmit(telemetry->uart, (const uint8_t *)text, length, 100U);
    }
}

void app_telemetry_send_format(app_telemetry_t *telemetry, const char *format, ...)
{
    char buffer[192];
    int length;
    va_list args;

    if ((telemetry == NULL) || (telemetry->uart == NULL) || (format == NULL))
    {
        return;
    }

    va_start(args, format);
    length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (length <= 0)
    {
        return;
    }

    if (length >= (int)sizeof(buffer))
    {
        length = (int)sizeof(buffer) - 1;
    }

    (void)drv_uart_transmit(telemetry->uart, (const uint8_t *)buffer, (uint16_t)length, 100U);
}

void app_telemetry_send_boot(app_telemetry_t *telemetry)
{
    app_telemetry_send_text(telemetry, "BOOT,USART1=OK\r\n");
}

void app_telemetry_send_reset_flags(app_telemetry_t *telemetry)
{
    uint32_t csr_value;
    uint8_t pin_reset;
    uint8_t por_reset;
    uint8_t software_reset;
    uint8_t iwdg_reset;
    uint8_t wwdg_reset;
    uint8_t lpwr_reset;

    csr_value = RCC->CSR;
    pin_reset = (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET) ? 1U : 0U;
    por_reset = (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET) ? 1U : 0U;
    software_reset = (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) != RESET) ? 1U : 0U;
    iwdg_reset = (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET) ? 1U : 0U;
    wwdg_reset = (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) != RESET) ? 1U : 0U;
    lpwr_reset = (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST) != RESET) ? 1U : 0U;

    app_telemetry_send_format(telemetry,
                              "RST,CSR=0x%08lX,PIN=%u,POR=%u,SFT=%u,IWDG=%u,WWDG=%u,LPWR=%u\r\n",
                              (unsigned long)csr_value,
                              pin_reset,
                              por_reset,
                              software_reset,
                              iwdg_reset,
                              wwdg_reset,
                              lpwr_reset);
    __HAL_RCC_CLEAR_RESET_FLAGS();
}

void app_telemetry_send_heartbeat(app_telemetry_t *telemetry)
{
    app_telemetry_send_text(telemetry, "HB,USART1=OK\r\n");
}

void app_telemetry_send_help(app_telemetry_t *telemetry)
{
    app_telemetry_send_text(telemetry, "CMD,HELP1=STATUS|ARM|DISARM|CLEAR|TARGET <mdeg>|VOFA ON|VOFA OFF\r\n");
    app_telemetry_send_text(telemetry, "CMD,HELP2=ANGLE|PID|ANGLE KP <q8>|ANGLE KI <q8>|ANGLE KD <q8>|ANGLE LIMIT <cmd>|ANGLE DEAD <ddeg>\r\n");
}

void app_telemetry_send_angle_config(app_telemetry_t *telemetry,
                                     const ctrl_angle_loop_config_t *config)
{
    if ((telemetry == NULL) || (config == NULL))
    {
        return;
    }

    app_telemetry_send_format(telemetry,
                              "ANGLE,CFG,PERIOD=%lu,KPQ8=%d,KIQ8=%d,KDQ8=%d,DLIM=%d,DGYRO=%u/%u,DFILT=%u/%u,ILIM=%ld,CLIM=%d,DEAD=%d\r\n",
                              (unsigned long)config->loop_period_ms,
                              config->kp_q8,
                              config->ki_q8,
                              config->kd_q8,
                              config->d_input_limit_ddeg,
                              (unsigned int)config->d_gyro_weight_num,
                              (unsigned int)config->d_gyro_weight_den,
                              (unsigned int)config->d_filter_alpha_num,
                              (unsigned int)config->d_filter_alpha_den,
                              (long)config->i_accum_limit,
                              config->cmd_limit,
                              config->error_deadband_ddeg);
}

void app_telemetry_send_state(app_telemetry_t *telemetry,
                              app_state_machine_state_t state,
                              uint8_t arm_request,
                              uint32_t active_fault_flags,
                              uint32_t latched_fault_flags)
{
    app_telemetry_send_format(telemetry,
                              "APP,STATE=%s,ARM=%u,FAULT=0x%02lX,LATCH=0x%02lX\r\n",
                              app_state_machine_get_state_name(state),
                              arm_request,
                              (unsigned long)active_fault_flags,
                              (unsigned long)latched_fault_flags);
}

void app_telemetry_send_status(app_telemetry_t *telemetry,
                               app_state_machine_state_t state,
                               uint8_t arm_request,
                               uint8_t estop_active,
                               const mod_battery_monitor_output_t *battery,
                               const mod_safety_output_t *safety,
                               int32_t target_pitch_mdeg,
                               const ctrl_attitude_estimator_output_t *attitude,
                               const ctrl_angle_loop_output_t *angle,
                               const ctrl_speed_loop_output_t *speed)
{
    uint32_t battery_mv;
    uint32_t battery_uv;
    uint32_t active_fault_flags;
    uint32_t latched_fault_flags;
    int32_t pitch_fused_mdeg;
    int32_t pitch_rate_mdps;
    int32_t pitch_error_mdeg;
    int16_t angle_cmd;
    int16_t pwm_cmd;

    battery_mv = (battery != NULL) ? battery->filtered_battery_mv : 0U;
    battery_uv = (battery != NULL) ? battery->undervoltage : 0U;
    active_fault_flags = (safety != NULL) ? safety->active_fault_flags : 0U;
    latched_fault_flags = (safety != NULL) ? safety->fault_latched_flags : 0U;
    pitch_fused_mdeg = (attitude != NULL) ? attitude->pitch_fused_mdeg : 0;
    pitch_rate_mdps = (attitude != NULL) ? attitude->pitch_rate_mdps : 0;
    pitch_error_mdeg = (angle != NULL) ? angle->pitch_error_mdeg : 0;
    angle_cmd = (angle != NULL) ? angle->angle_cmd : 0;
    pwm_cmd = (speed != NULL) ? speed->pwm_out : 0;

    app_telemetry_send_format(telemetry,
                              "STATUS,STATE=%s,ARM=%u,ESTOP=%u,BAT=%lu,UV=%lu,FAULT=0x%02lX,LATCH=0x%02lX,TGT=%ld,PITCH=%ld,RATE=%ld,ERR=%ld,ACMD=%d,PWM=%d\r\n",
                              app_state_machine_get_state_name(state),
                              arm_request,
                              estop_active,
                              (unsigned long)battery_mv,
                              (unsigned long)battery_uv,
                              (unsigned long)active_fault_flags,
                              (unsigned long)latched_fault_flags,
                              (long)target_pitch_mdeg,
                              (long)pitch_fused_mdeg,
                              (long)pitch_rate_mdps,
                              (long)pitch_error_mdeg,
                              angle_cmd,
                              pwm_cmd);
}

void app_telemetry_send_command_result(app_telemetry_t *telemetry,
                                       const app_command_result_t *command_result)
{
    if ((telemetry == NULL) || (command_result == NULL))
    {
        return;
    }

    if ((command_result->line_parsed != 0U) && (command_result->line_valid == 0U))
    {
        app_telemetry_send_text(telemetry, "CMD,ERR\r\n");
        return;
    }

    if (command_result->help_requested != 0U)
    {
        app_telemetry_send_help(telemetry);
    }

    if (command_result->arm_request_valid != 0U)
    {
        app_telemetry_send_format(telemetry,
                                  "CMD,ARM=%u\r\n",
                                  (unsigned int)command_result->arm_request);
    }

    if (command_result->clear_fault_requested != 0U)
    {
        app_telemetry_send_text(telemetry, "CMD,CLEAR=1\r\n");
    }

    if (command_result->target_pitch_valid != 0U)
    {
        app_telemetry_send_format(telemetry,
                                  "CMD,TARGET=%ld\r\n",
                                  (long)command_result->target_pitch_mdeg);
    }

    if (command_result->vofa_enable_valid != 0U)
    {
        app_telemetry_send_format(telemetry,
                                  "CMD,VOFA=%u\r\n",
                                  (unsigned int)command_result->vofa_enable);
    }

    if (command_result->angle_config_requested != 0U)
    {
        app_telemetry_send_text(telemetry, "CMD,ANGLE=SHOW\r\n");
    }

    if (command_result->angle_kp_valid != 0U)
    {
        app_telemetry_send_format(telemetry,
                                  "CMD,ANGLE,KP=%d\r\n",
                                  (int)command_result->angle_kp_q8);
    }

    if (command_result->angle_ki_valid != 0U)
    {
        app_telemetry_send_format(telemetry,
                                  "CMD,ANGLE,KI=%d\r\n",
                                  (int)command_result->angle_ki_q8);
    }

    if (command_result->angle_kd_valid != 0U)
    {
        app_telemetry_send_format(telemetry,
                                  "CMD,ANGLE,KD=%d\r\n",
                                  (int)command_result->angle_kd_q8);
    }

    if (command_result->angle_cmd_limit_valid != 0U)
    {
        app_telemetry_send_format(telemetry,
                                  "CMD,ANGLE,LIMIT=%d\r\n",
                                  (int)command_result->angle_cmd_limit);
    }

    if (command_result->angle_deadband_valid != 0U)
    {
        app_telemetry_send_format(telemetry,
                                  "CMD,ANGLE,DEAD=%d\r\n",
                                  (int)command_result->angle_error_deadband_ddeg);
    }
}

void app_telemetry_send_imu_event(app_telemetry_t *telemetry,
                                  mod_imu_t *imu_module,
                                  mod_imu_event_t event)
{
    const mod_imu_calibration_t *calibration;

    if ((telemetry == NULL) || (imu_module == NULL))
    {
        return;
    }

    calibration = mod_imu_get_calibration(imu_module);
    switch (event.id)
    {
        case MOD_IMU_EVENT_INIT_RETRY:
            app_telemetry_send_text(telemetry, "MPU,INIT=RETRY\r\n");
            break;

        case MOD_IMU_EVENT_WHOAMI_READ_ERR:
            app_telemetry_send_text(telemetry, "MPU,WHOAMI=READ_ERR\r\n");
            app_telemetry_send_soft_i2c_diag(telemetry, "MPU,WHOAMI");
            break;

        case MOD_IMU_EVENT_WHOAMI_OK:
            app_telemetry_send_format(telemetry, "MPU,WHOAMI=0x%02X\r\n", event.who_am_i);
            app_telemetry_send_format(telemetry,
                                      "MPU,CALIBRATING=%lu\r\n",
                                      (unsigned long)imu_module->config.zero_sample_count);
            break;

        case MOD_IMU_EVENT_RAW_TEST_ERR:
            app_telemetry_send_text(telemetry, "MPU,CAL,START\r\n");
            app_telemetry_send_text(telemetry, "MPU,CAL,RAW_TEST=ERR\r\n");
            app_telemetry_send_soft_i2c_diag(telemetry, "MPU,CAL");
            break;

        case MOD_IMU_EVENT_RAW_TEST_OK:
            app_telemetry_send_text(telemetry, "MPU,CAL,START\r\n");
            app_telemetry_send_text(telemetry, "MPU,CAL,RAW_TEST=OK\r\n");
            break;

        case MOD_IMU_EVENT_CAL_PROGRESS:
            app_telemetry_send_cal_index(telemetry, event.sample_index);
            break;

        case MOD_IMU_EVENT_CAL_READ_ERR:
            app_telemetry_send_format(telemetry,
                                      "MPU,CAL,READ_ERR,IDX=%lu\r\n",
                                      (unsigned long)event.sample_index);
            app_telemetry_send_soft_i2c_diag(telemetry, "MPU,CAL");
            break;

        case MOD_IMU_EVENT_CAL_DONE:
            if (calibration == NULL)
            {
                break;
            }
            app_telemetry_send_text(telemetry, "MPU,CAL,SUM_OK\r\n");
            app_telemetry_send_text(telemetry, "MPU,CAL,AVG_OK\r\n");
            app_telemetry_send_text(telemetry, "MPU,CAL,PITCH_OK\r\n");
            app_telemetry_send_format(telemetry,
                                      "MPU_ZERO,GX=%ld,GY=%ld,GZ=%ld\r\n",
                                      (long)calibration->gyro_bias_x,
                                      (long)calibration->gyro_bias_y,
                                      (long)calibration->gyro_bias_z);
            app_telemetry_send_format(telemetry,
                                      "MPU_ACC_ZERO,AX=%ld,AY=%ld,AZ=%ld\r\n",
                                      (long)calibration->accel_zero_x,
                                      (long)calibration->accel_zero_y,
                                      (long)calibration->accel_zero_z);
            app_telemetry_send_format(telemetry,
                                      "MPU_PITCH_ZERO_MDEG=%ld\r\n",
                                      (long)calibration->pitch_zero_mdeg);
            app_telemetry_send_text(telemetry, "MPU,CAL=OK\r\n");
            break;

        case MOD_IMU_EVENT_NONE:
        default:
            break;
    }
}

void app_telemetry_send_angle_loop_vofa(app_telemetry_t *telemetry,
                                        const ctrl_attitude_estimator_output_t *attitude_output,
                                        int32_t target_pitch_mdeg,
                                        const ctrl_angle_loop_output_t *angle_output)
{
    char channel_1[20];
    char channel_2[20];
    char channel_3[20];
    char channel_4[16];
    char channel_5[20];
    char channel_6[20];

    if ((telemetry == NULL) || (attitude_output == NULL) || (angle_output == NULL))
    {
        return;
    }

    if (app_telemetry_format_signed_milli(channel_1, sizeof(channel_1), target_pitch_mdeg) <= 0)
    {
        return;
    }

    if (app_telemetry_format_signed_milli(channel_2, sizeof(channel_2), attitude_output->pitch_fused_mdeg) <= 0)
    {
        return;
    }

    if (app_telemetry_format_signed_milli(channel_3, sizeof(channel_3), angle_output->pitch_error_mdeg) <= 0)
    {
        return;
    }

    if (snprintf(channel_4, sizeof(channel_4), "%d", (int)angle_output->angle_cmd) <= 0)
    {
        return;
    }

    if (app_telemetry_format_signed_milli(channel_5, sizeof(channel_5), attitude_output->pitch_rate_mdps) <= 0)
    {
        return;
    }

    if (app_telemetry_format_signed_milli(channel_6,
                                          sizeof(channel_6),
                                          angle_output->d_input_filt_ddeg * 100) <= 0)
    {
        return;
    }

    app_telemetry_send_format(telemetry,
                              "%s,%s,%s,%s,%s,%s\r\n",
                              channel_1,
                              channel_2,
                              channel_3,
                              channel_4,
                              channel_5,
                              channel_6);
}
