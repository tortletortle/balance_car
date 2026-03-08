#include "app.h"

#include <string.h>

const app_logic_config_t g_app_default_logic_config =
{
    {
        300U,
        5U,
        10U,
        50U,
        200U,
        500U,
        &g_dev_mpu6050_default_init_config
    },
    {
        0.98f,
        131.0f,
        50U
    },
    {
        10U,
        2816,
        16,
        256,
        1U,
        2U,
        400,
        3U,
        4U,
        12000,
        1500,
        0,
        0,
        5
    },
    {
        0U,
        100,
        1,
        -1,
        2304,
        64,
        4000,
        1780,
        900,
        980,
        80
    },
    {
        1780,
        80
    },
    {
        8U,
        3U,
        3300U,
        11U,
        1U,
        6600U,
        7000U
    },
    {
        1U
    },
    {
        1000U,
        50U,
        10U,
        200U
    },
    {
        63U
    },
    0,
    50U
};

static HAL_StatusTypeDef app_uart_start_receive_it(app_t *app)
{
    if (app == NULL)
    {
        return HAL_ERROR;
    }

    return drv_uart_start_receive_byte_it(&app->debug_uart, &app->uart_rx_it_byte);
}

static uint8_t app_is_estop_active(const app_t *app)
{
    GPIO_PinState pin_state;

    if ((app == NULL) || (app->config.hw.estop_port == NULL))
    {
        return 0U;
    }

    pin_state = HAL_GPIO_ReadPin(app->config.hw.estop_port, app->config.hw.estop_pin);
    if (app->config.hw.estop_active_low != 0U)
    {
        return (pin_state == GPIO_PIN_RESET) ? 1U : 0U;
    }

    return (pin_state == GPIO_PIN_SET) ? 1U : 0U;
}

static void app_reset_control_chain(app_t *app, uint32_t now_ms)
{
    ctrl_attitude_estimator_reset(&app->attitude_estimator);
    ctrl_angle_loop_reset(&app->angle_loop);
    ctrl_speed_loop_reset(&app->speed_loop);
    mod_motor_reset(&app->motor_module);
    app->last_attitude_update_ms = now_ms;
    memset(&app->last_attitude_output, 0, sizeof(app->last_attitude_output));
    memset(&app->last_angle_output, 0, sizeof(app->last_angle_output));
    memset(&app->last_speed_output, 0, sizeof(app->last_speed_output));
}

static HAL_StatusTypeDef app_init_drivers(app_t *app)
{
    if ((app == NULL) || (app->config.hw.delay_us == NULL) ||
        (app->config.hw.huart_debug == NULL) || (app->config.hw.htim_pwm == NULL))
    {
        return HAL_ERROR;
    }

    if (drv_uart_init(&app->debug_uart, app->config.hw.huart_debug) != HAL_OK)
    {
        return HAL_ERROR;
    }

    app->imu_soft_i2c_bus.scl_port = app->config.hw.imu_scl_port;
    app->imu_soft_i2c_bus.scl_pin = app->config.hw.imu_scl_pin;
    app->imu_soft_i2c_bus.sda_port = app->config.hw.imu_sda_port;
    app->imu_soft_i2c_bus.sda_pin = app->config.hw.imu_sda_pin;
    app->imu_soft_i2c_bus.bit_delay_us = app->config.hw.imu_i2c_bit_delay_us;
    app->imu_soft_i2c_bus.delay_us = app->config.hw.delay_us;

    if (drv_pwm_init(&app->motor_pwm_a,
                     app->config.hw.htim_pwm,
                     app->config.hw.pwm_channel_motor_a) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (drv_pwm_init(&app->motor_pwm_b,
                     app->config.hw.htim_pwm,
                     app->config.hw.pwm_channel_motor_b) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (dev_tb6612_init(&app->tb6612,
                        &app->motor_pwm_a,
                        &app->motor_pwm_b,
                        &g_dev_tb6612_default_config) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (dev_tb6612_start(&app->tb6612) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (mod_motor_init(&app->motor_module, &app->tb6612, &app->config.logic.motor_config) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (drv_soft_i2c_init(&app->imu_soft_i2c_bus) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (dev_mpu6050_bind(&app->mpu6050,
                         &app->imu_soft_i2c_bus,
                         DEV_MPU6050_DEFAULT_ADDRESS_7BIT) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if ((app->config.hw.hadc_battery != NULL) && (app->config.hw.adc_battery_channel != 0U))
    {
        if (drv_adc_init(&app->battery_adc,
                         app->config.hw.hadc_battery,
                         app->config.hw.adc_battery_channel) == HAL_OK)
        {
            app->battery_ready = 1U;
        }
    }

    if ((app->config.hw.htim_encoder_a != NULL) && (app->config.hw.htim_encoder_b != NULL))
    {
        if ((drv_encoder_init(&app->encoder_a,
                              app->config.hw.htim_encoder_a,
                              app->config.hw.encoder_a_direction) == HAL_OK) &&
            (drv_encoder_init(&app->encoder_b,
                              app->config.hw.htim_encoder_b,
                              app->config.hw.encoder_b_direction) == HAL_OK) &&
            (drv_encoder_start(&app->encoder_a) == HAL_OK) &&
            (drv_encoder_start(&app->encoder_b) == HAL_OK))
        {
            app->encoders_ready = 1U;
        }
    }

    return HAL_OK;
}

static HAL_StatusTypeDef app_init_modules(app_t *app)
{
    ctrl_angle_loop_config_t angle_loop_config;
    app_scheduler_config_t scheduler_config;

    if (mod_imu_init(&app->imu_module, &app->mpu6050, &app->config.logic.imu_config) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (app->battery_ready != 0U)
    {
        if (mod_battery_monitor_init(&app->battery_module,
                                     &app->battery_adc,
                                     &app->config.logic.battery_config) != HAL_OK)
        {
            return HAL_ERROR;
        }
    }

    if (mod_safety_init(&app->safety_module, &app->config.logic.safety_config) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (ctrl_attitude_estimator_init(&app->attitude_estimator,
                                     &app->config.logic.attitude_config) != HAL_OK)
    {
        return HAL_ERROR;
    }

    angle_loop_config = app->config.logic.angle_loop_config;
    if (app->config.logic.imu_config.report_interval_ms > 0U)
    {
        angle_loop_config.loop_period_ms = app->config.logic.imu_config.report_interval_ms;
    }
    if (ctrl_angle_loop_init(&app->angle_loop, &angle_loop_config) != HAL_OK)
    {
        return HAL_ERROR;
    }
    if (ctrl_speed_loop_init(&app->speed_loop, &app->config.logic.speed_loop_config) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (app_state_machine_init(&app->state_machine) != HAL_OK)
    {
        return HAL_ERROR;
    }

    scheduler_config = app->config.logic.scheduler_config;
    if (app->config.logic.imu_config.report_interval_ms > 0U)
    {
        scheduler_config.control_period_ms = app->config.logic.imu_config.report_interval_ms;
    }
    if (app_scheduler_init(&app->scheduler, &scheduler_config) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (app_command_init(&app->command, &app->config.logic.command_config) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (app_telemetry_init(&app->telemetry, &app->debug_uart) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

static void app_apply_motor_output(app_t *app, int16_t pwm_cmd, uint8_t output_allowed)
{
    uint8_t enable_output;

    if (app == NULL)
    {
        return;
    }

    mod_motor_set_common_target(&app->motor_module, pwm_cmd);
    enable_output = ((app->config.hw.motor_output_enable != 0U) && (output_allowed != 0U)) ? 1U : 0U;
    mod_motor_set_enable(&app->motor_module, enable_output);
    (void)mod_motor_update(&app->motor_module, NULL);
}

static HAL_StatusTypeDef app_run_control_step(app_t *app, uint32_t now_ms)
{
    mod_imu_report_t imu_report;
    ctrl_attitude_estimator_input_t estimator_input;
    ctrl_angle_loop_input_t angle_input;
    ctrl_speed_loop_input_t speed_input;
    int16_t encoder_delta_a;
    int16_t encoder_delta_b;

    if (mod_imu_read_report(&app->imu_module, now_ms, &imu_report) != HAL_OK)
    {
        app_telemetry_send_text(&app->telemetry, "MPU,READ=ERR\r\n");
        app_reset_control_chain(app, now_ms);
        return HAL_ERROR;
    }

    if (imu_report.calibration_ok == 0U)
    {
        app_reset_control_chain(app, now_ms);
        return HAL_OK;
    }

    estimator_input.pitch_accel_mdeg = imu_report.pitch_zero_relative_mdeg;
    estimator_input.pitch_rate_lsb = imu_report.gyro_y_zero_relative;
    if (ctrl_attitude_estimator_update(&app->attitude_estimator,
                                       &estimator_input,
                                       now_ms,
                                       &app->last_attitude_output) != HAL_OK)
    {
        app_reset_control_chain(app, now_ms);
        return HAL_ERROR;
    }

    angle_input.target_pitch_mdeg = app->target_pitch_mdeg;
    angle_input.pitch_mdeg = app->last_attitude_output.pitch_fused_mdeg;
    angle_input.pitch_rate_mdps = app->last_attitude_output.pitch_rate_mdps;
    if (ctrl_angle_loop_update(&app->angle_loop, &angle_input, &app->last_angle_output) != HAL_OK)
    {
        app_reset_control_chain(app, now_ms);
        return HAL_ERROR;
    }

    encoder_delta_a = 0;
    encoder_delta_b = 0;
    if (app->encoders_ready != 0U)
    {
        encoder_delta_a = drv_encoder_get_delta(&app->encoder_a);
        encoder_delta_b = drv_encoder_get_delta(&app->encoder_b);
    }

    speed_input.encoder_delta_a = encoder_delta_a;
    speed_input.encoder_delta_b = encoder_delta_b;
    speed_input.base_cmd = app->last_angle_output.angle_cmd;
    if (ctrl_speed_loop_update(&app->speed_loop, &speed_input, &app->last_speed_output) != HAL_OK)
    {
        app_reset_control_chain(app, now_ms);
        return HAL_ERROR;
    }

    app->last_attitude_update_ms = now_ms;
    app->imu_ready_seen = 1U;
    if (app->vofa_enabled != 0U)
    {
        app_telemetry_send_angle_loop_vofa(&app->telemetry,
                                           &app->last_attitude_output,
                                           app->target_pitch_mdeg,
                                           &app->last_angle_output);
    }

    return HAL_OK;
}

static HAL_StatusTypeDef app_update_safety_and_state(app_t *app,
                                                     uint32_t now_ms,
                                                     uint8_t clear_request,
                                                     uint8_t force_status)
{
    mod_safety_input_t safety_input;
    mod_safety_output_t safety_output;
    app_state_machine_input_t state_input;
    app_state_machine_output_t state_output;
    uint8_t imu_running;
    uint8_t imu_fresh;

    imu_running = mod_imu_is_running(&app->imu_module);
    imu_fresh = (imu_running != 0U) &&
                ((now_ms - app->last_attitude_update_ms) <= app->config.logic.imu_stale_timeout_ms);

    safety_input.estop_active = app_is_estop_active(app);
    safety_input.battery_undervoltage = (app->battery_ready != 0U) ? app->battery_module.last_output.undervoltage : 0U;
    safety_input.imu_running = (app->imu_ready_seen != 0U) ? imu_running : 1U;
    safety_input.imu_fresh = (app->imu_ready_seen != 0U) ? imu_fresh : 1U;
    safety_input.clear_latched_request = clear_request;
    if (mod_safety_update(&app->safety_module, &safety_input, &safety_output) != HAL_OK)
    {
        return HAL_ERROR;
    }

    state_input.arm_request = app->arm_request;
    state_input.clear_request = clear_request;
    state_input.imu_ready = imu_running;
    state_input.estop_active = safety_input.estop_active;
    state_input.fault_latched = safety_output.fault_latched;
    if (app_state_machine_update(&app->state_machine, &state_input, &state_output) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if ((state_output.state_changed != 0U) || (force_status != 0U))
    {
        app_telemetry_send_state(&app->telemetry,
                                 state_output.current_state,
                                 app->arm_request,
                                 safety_output.active_fault_flags,
                                 safety_output.fault_latched_flags);
    }

    if (state_output.current_state == APP_STATE_ARMED)
    {
        app_apply_motor_output(app, app->last_speed_output.pwm_out, state_output.output_allowed);
    }
    else
    {
        app_apply_motor_output(app, 0, 0U);
    }

    return HAL_OK;
}
static void app_handle_command_result(app_t *app,
                                      const app_command_result_t *command_result,
                                      uint8_t *clear_request,
                                      uint8_t *force_status)
{
    if ((app == NULL) || (command_result == NULL))
    {
        return;
    }

    app_telemetry_send_command_result(&app->telemetry, command_result);
    if (command_result->arm_request_valid != 0U)
    {
        app->arm_request = command_result->arm_request;
    }

    if (command_result->target_pitch_valid != 0U)
    {
        app->target_pitch_mdeg = command_result->target_pitch_mdeg;
    }

    if (command_result->vofa_enable_valid != 0U)
    {
        app->vofa_enabled = command_result->vofa_enable;
    }

    if ((clear_request != NULL) && (command_result->clear_fault_requested != 0U))
    {
        *clear_request = 1U;
    }

    if ((force_status != NULL) && (command_result->status_requested != 0U))
    {
        *force_status = 1U;
    }
}

HAL_StatusTypeDef app_init(app_t *app, const app_config_t *config, uint32_t now_ms)
{
    if ((app == NULL) || (config == NULL))
    {
        return HAL_ERROR;
    }

    memset(app, 0, sizeof(*app));
    app->config = *config;
    app->target_pitch_mdeg = config->logic.initial_target_pitch_mdeg;
    app->vofa_enabled = 1U;

    if ((app_init_drivers(app) != HAL_OK) || (app_init_modules(app) != HAL_OK))
    {
        return HAL_ERROR;
    }

    app_reset(app, now_ms);
    if (app_uart_start_receive_it(app) != HAL_OK)
    {
        return HAL_ERROR;
    }
    app_telemetry_send_boot(&app->telemetry);
    app_telemetry_send_reset_flags(&app->telemetry);
    app_telemetry_send_format(&app->telemetry,
                              "MOTOR,CHAIN=OK,EXEC=%u,ESTOP=%u\r\n",
                              (unsigned int)app->config.hw.motor_output_enable,
                              (unsigned int)app_is_estop_active(app));
    return HAL_OK;
}

void app_reset(app_t *app, uint32_t now_ms)
{
    if (app == NULL)
    {
        return;
    }

    app->arm_request = 0U;
    app->imu_ready_seen = 0U;
    if (app->battery_ready != 0U)
    {
        mod_battery_monitor_reset(&app->battery_module);
    }
    mod_safety_reset(&app->safety_module);
    app_state_machine_reset(&app->state_machine);
    app_scheduler_reset(&app->scheduler, now_ms);
    app_command_reset(&app->command);
    mod_imu_reset(&app->imu_module, now_ms);
    app_reset_control_chain(app, now_ms);
}

HAL_StatusTypeDef app_task(app_t *app, uint32_t now_ms)
{
    app_command_result_t command_result;
    mod_imu_event_t imu_event;
    uint32_t scheduler_flags;
    uint8_t clear_request;
    uint8_t force_status;

    if (app == NULL)
    {
        return HAL_ERROR;
    }

    clear_request = 0U;
    force_status = 0U;
    if (app_command_poll(&app->command, &command_result) != HAL_OK)
    {
        return HAL_ERROR;
    }
    app_handle_command_result(app, &command_result, &clear_request, &force_status);

    imu_event = mod_imu_task(&app->imu_module, now_ms);
    if (imu_event.id != MOD_IMU_EVENT_NONE)
    {
        if ((imu_event.id == MOD_IMU_EVENT_INIT_RETRY) ||
            (imu_event.id == MOD_IMU_EVENT_WHOAMI_READ_ERR) ||
            (imu_event.id == MOD_IMU_EVENT_RAW_TEST_ERR) ||
            (imu_event.id == MOD_IMU_EVENT_CAL_READ_ERR) ||
            (imu_event.id == MOD_IMU_EVENT_CAL_DONE))
        {
            app_reset_control_chain(app, now_ms);
        }
        app_telemetry_send_imu_event(&app->telemetry, &app->imu_module, imu_event);
        if (imu_event.id == MOD_IMU_EVENT_CAL_DONE)
        {
            app_reset_control_chain(app, now_ms);
            force_status = 1U;
        }
    }

    scheduler_flags = app_scheduler_poll(&app->scheduler, now_ms);
    if ((scheduler_flags & APP_SCHED_FLAG_BATTERY) != 0U)
    {
        if (app->battery_ready != 0U)
        {
            (void)mod_battery_monitor_update(&app->battery_module, NULL);
        }
    }

    if (app_update_safety_and_state(app, now_ms, clear_request, force_status) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if ((scheduler_flags & APP_SCHED_FLAG_HEARTBEAT) != 0U)
    {
        app_telemetry_send_heartbeat(&app->telemetry);
    }

    if (((scheduler_flags & APP_SCHED_FLAG_CONTROL) != 0U) && (mod_imu_is_running(&app->imu_module) != 0U))
    {
        (void)app_run_control_step(app, now_ms);
        (void)app_update_safety_and_state(app, now_ms, 0U, 0U);
    }

    if (((scheduler_flags & APP_SCHED_FLAG_STATUS) != 0U) || (command_result.status_requested != 0U))
    {
        app_telemetry_send_status(&app->telemetry,
                                  app_state_machine_get_state(&app->state_machine),
                                  app->arm_request,
                                  app_is_estop_active(app),
                                  (app->battery_ready != 0U) ? mod_battery_monitor_get_last_output(&app->battery_module) : NULL,
                                  mod_safety_get_last_output(&app->safety_module),
                                  app->target_pitch_mdeg,
                                  ctrl_attitude_estimator_get_last_output(&app->attitude_estimator),
                                  ctrl_angle_loop_get_last_output(&app->angle_loop),
                                  ctrl_speed_loop_get_last_output(&app->speed_loop));
    }

    return HAL_OK;
}

HAL_StatusTypeDef app_on_uart_rx_cplt(app_t *app, UART_HandleTypeDef *huart)
{
    if ((app == NULL) || (huart == NULL))
    {
        return HAL_ERROR;
    }

    if (drv_uart_matches_handle(&app->debug_uart, huart) == 0U)
    {
        return HAL_OK;
    }

    app_command_feed_byte(&app->command, app->uart_rx_it_byte);
    return app_uart_start_receive_it(app);
}

HAL_StatusTypeDef app_on_uart_error(app_t *app, UART_HandleTypeDef *huart)
{
    if ((app == NULL) || (huart == NULL))
    {
        return HAL_ERROR;
    }

    if (drv_uart_matches_handle(&app->debug_uart, huart) == 0U)
    {
        return HAL_OK;
    }

    drv_uart_clear_overrun(&app->debug_uart);
    return app_uart_start_receive_it(app);
}
