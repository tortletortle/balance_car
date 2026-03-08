#include "mod_imu.h"

#include <math.h>

const mod_imu_config_t g_mod_imu_default_config =
{
    300U,
    5U,
    10U,
    50U,
    200U,
    500U,
    &g_dev_mpu6050_default_init_config
};

static mod_imu_event_t mod_imu_make_event(mod_imu_event_id_t id, uint32_t sample_index, uint8_t who_am_i)
{
    mod_imu_event_t event;

    event.id = id;
    event.sample_index = sample_index;
    event.who_am_i = who_am_i;
    return event;
}

static int32_t mod_imu_estimate_pitch_mdeg(int32_t accel_x, int32_t accel_z)
{
    float pitch_deg;

    pitch_deg = -atan2f((float)accel_x, (float)accel_z) * 57.2957795f;
    return (int32_t)(pitch_deg * 1000.0f);
}

static void mod_imu_reset_calibration_context(mod_imu_t *imu)
{
    imu->calibration_ok = 0U;
    imu->cal_sample_index = 0U;
    imu->cal_sum_accel_x = 0;
    imu->cal_sum_accel_y = 0;
    imu->cal_sum_accel_z = 0;
    imu->cal_sum_gyro_x = 0;
    imu->cal_sum_gyro_y = 0;
    imu->cal_sum_gyro_z = 0;
}

static void mod_imu_finalize_calibration(mod_imu_t *imu)
{
    imu->calibration.accel_zero_x = (int32_t)(imu->cal_sum_accel_x / (int64_t)imu->config.zero_sample_count);
    imu->calibration.accel_zero_y = (int32_t)(imu->cal_sum_accel_y / (int64_t)imu->config.zero_sample_count);
    imu->calibration.accel_zero_z = (int32_t)(imu->cal_sum_accel_z / (int64_t)imu->config.zero_sample_count);
    imu->calibration.gyro_bias_x = (int32_t)(imu->cal_sum_gyro_x / (int64_t)imu->config.zero_sample_count);
    imu->calibration.gyro_bias_y = (int32_t)(imu->cal_sum_gyro_y / (int64_t)imu->config.zero_sample_count);
    imu->calibration.gyro_bias_z = (int32_t)(imu->cal_sum_gyro_z / (int64_t)imu->config.zero_sample_count);
    imu->calibration.pitch_zero_mdeg = mod_imu_estimate_pitch_mdeg(
        imu->calibration.accel_zero_x,
        imu->calibration.accel_zero_z);
    imu->calibration_ok = 1U;
    imu->state = MOD_IMU_STATE_RUNNING;
}

HAL_StatusTypeDef mod_imu_init(mod_imu_t *imu, dev_mpu6050_t *device, const mod_imu_config_t *config)
{
    if ((imu == NULL) || (device == NULL) || (config == NULL) || (config->device_init_config == NULL))
    {
        return HAL_ERROR;
    }

    if ((config->zero_sample_count == 0U) || (config->cal_progress_step == 0U))
    {
        return HAL_ERROR;
    }

    imu->device = device;
    imu->config = *config;
    imu->state = MOD_IMU_STATE_RETRY_WAIT;
    imu->retry_last_ms = 0U;
    imu->next_action_ms = 0U;
    imu->calibration.gyro_bias_x = 0;
    imu->calibration.gyro_bias_y = 0;
    imu->calibration.gyro_bias_z = 0;
    imu->calibration.accel_zero_x = 0;
    imu->calibration.accel_zero_y = 0;
    imu->calibration.accel_zero_z = 0;
    imu->calibration.pitch_zero_mdeg = 0;
    mod_imu_reset_calibration_context(imu);

    return HAL_OK;
}

void mod_imu_reset(mod_imu_t *imu, uint32_t now_ms)
{
    if (imu == NULL)
    {
        return;
    }

    mod_imu_reset_calibration_context(imu);
    imu->state = MOD_IMU_STATE_RETRY_WAIT;
    imu->retry_last_ms = now_ms - imu->config.retry_interval_ms;
    imu->next_action_ms = now_ms;
}

mod_imu_event_t mod_imu_task(mod_imu_t *imu, uint32_t now_ms)
{
    uint8_t who_am_i;
    dev_mpu6050_raw_data_t raw_data;

    if ((imu == NULL) || (imu->device == NULL))
    {
        return mod_imu_make_event(MOD_IMU_EVENT_NONE, 0U, 0U);
    }

    who_am_i = 0U;
    raw_data.accel_x = 0;
    raw_data.accel_y = 0;
    raw_data.accel_z = 0;
    raw_data.temperature = 0;
    raw_data.gyro_x = 0;
    raw_data.gyro_y = 0;
    raw_data.gyro_z = 0;

    switch (imu->state)
    {
        case MOD_IMU_STATE_RETRY_WAIT:
            if ((now_ms - imu->retry_last_ms) < imu->config.retry_interval_ms)
            {
                return mod_imu_make_event(MOD_IMU_EVENT_NONE, 0U, 0U);
            }

            imu->retry_last_ms = now_ms;
            if (dev_mpu6050_init(imu->device, imu->config.device_init_config) != HAL_OK)
            {
                return mod_imu_make_event(MOD_IMU_EVENT_INIT_RETRY, 0U, 0U);
            }

            if (dev_mpu6050_read_who_am_i(imu->device, &who_am_i) != HAL_OK)
            {
                return mod_imu_make_event(MOD_IMU_EVENT_WHOAMI_READ_ERR, 0U, 0U);
            }

            mod_imu_reset_calibration_context(imu);
            imu->next_action_ms = now_ms + imu->config.startup_settle_ms;
            imu->state = MOD_IMU_STATE_SETTLE;
            return mod_imu_make_event(MOD_IMU_EVENT_WHOAMI_OK, 0U, who_am_i);

        case MOD_IMU_STATE_SETTLE:
            if ((int32_t)(now_ms - imu->next_action_ms) < 0)
            {
                return mod_imu_make_event(MOD_IMU_EVENT_NONE, 0U, 0U);
            }

            if (dev_mpu6050_read_raw(imu->device, &raw_data) != HAL_OK)
            {
                imu->state = MOD_IMU_STATE_RETRY_WAIT;
                imu->retry_last_ms = now_ms;
                return mod_imu_make_event(MOD_IMU_EVENT_RAW_TEST_ERR, 0U, 0U);
            }

            imu->next_action_ms = now_ms;
            imu->state = MOD_IMU_STATE_CALIBRATING;
            return mod_imu_make_event(MOD_IMU_EVENT_RAW_TEST_OK, 0U, 0U);

        case MOD_IMU_STATE_CALIBRATING:
            if ((int32_t)(now_ms - imu->next_action_ms) < 0)
            {
                return mod_imu_make_event(MOD_IMU_EVENT_NONE, 0U, 0U);
            }

            if (dev_mpu6050_read_raw(imu->device, &raw_data) != HAL_OK)
            {
                uint32_t failed_index;

                failed_index = imu->cal_sample_index;
                mod_imu_reset_calibration_context(imu);
                imu->state = MOD_IMU_STATE_RETRY_WAIT;
                imu->retry_last_ms = now_ms;
                return mod_imu_make_event(MOD_IMU_EVENT_CAL_READ_ERR, failed_index, 0U);
            }

            imu->cal_sum_accel_x += raw_data.accel_x;
            imu->cal_sum_accel_y += raw_data.accel_y;
            imu->cal_sum_accel_z += raw_data.accel_z;
            imu->cal_sum_gyro_x += raw_data.gyro_x;
            imu->cal_sum_gyro_y += raw_data.gyro_y;
            imu->cal_sum_gyro_z += raw_data.gyro_z;

            if (((imu->cal_sample_index + 1U) <= 10U) || (((imu->cal_sample_index + 1U) % imu->config.cal_progress_step) == 0U))
            {
                uint32_t progress_index;

                progress_index = imu->cal_sample_index;
                imu->cal_sample_index++;
                if (imu->cal_sample_index >= imu->config.zero_sample_count)
                {
                    mod_imu_finalize_calibration(imu);
                    return mod_imu_make_event(MOD_IMU_EVENT_CAL_DONE, progress_index, 0U);
                }

                imu->next_action_ms = now_ms + imu->config.zero_sample_interval_ms;
                return mod_imu_make_event(MOD_IMU_EVENT_CAL_PROGRESS, progress_index, 0U);
            }

            imu->cal_sample_index++;
            if (imu->cal_sample_index >= imu->config.zero_sample_count)
            {
                mod_imu_finalize_calibration(imu);
                return mod_imu_make_event(MOD_IMU_EVENT_CAL_DONE, 0U, 0U);
            }

            imu->next_action_ms = now_ms + imu->config.zero_sample_interval_ms;
            return mod_imu_make_event(MOD_IMU_EVENT_NONE, 0U, 0U);

        case MOD_IMU_STATE_RUNNING:
        default:
            return mod_imu_make_event(MOD_IMU_EVENT_NONE, 0U, 0U);
    }
}

HAL_StatusTypeDef mod_imu_read_report(mod_imu_t *imu, uint32_t now_ms, mod_imu_report_t *report)
{
    if ((imu == NULL) || (imu->device == NULL) || (report == NULL))
    {
        return HAL_ERROR;
    }

    if (dev_mpu6050_read_raw(imu->device, &report->raw_data) != HAL_OK)
    {
        mod_imu_reset_calibration_context(imu);
        imu->state = MOD_IMU_STATE_RETRY_WAIT;
        imu->retry_last_ms = now_ms;
        imu->next_action_ms = now_ms;
        return HAL_ERROR;
    }

    report->calibration_ok = imu->calibration_ok;
    report->gyro_x_zero_relative = (int32_t)report->raw_data.gyro_x - imu->calibration.gyro_bias_x;
    report->gyro_y_zero_relative = (int32_t)report->raw_data.gyro_y - imu->calibration.gyro_bias_y;
    report->gyro_z_zero_relative = (int32_t)report->raw_data.gyro_z - imu->calibration.gyro_bias_z;
    report->pitch_zero_relative_mdeg = mod_imu_estimate_pitch_mdeg(
        report->raw_data.accel_x,
        report->raw_data.accel_z) - imu->calibration.pitch_zero_mdeg;

    return HAL_OK;
}

uint8_t mod_imu_is_running(const mod_imu_t *imu)
{
    if (imu == NULL)
    {
        return 0U;
    }

    return (imu->state == MOD_IMU_STATE_RUNNING) ? 1U : 0U;
}

const mod_imu_calibration_t *mod_imu_get_calibration(const mod_imu_t *imu)
{
    if (imu == NULL)
    {
        return NULL;
    }

    return &imu->calibration;
}
