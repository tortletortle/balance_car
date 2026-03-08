#ifndef MOD_IMU_H
#define MOD_IMU_H

#include <stdint.h>

#include "stm32f1xx_hal.h"
#include "dev_mpu6050.h"

typedef enum
{
    MOD_IMU_STATE_RETRY_WAIT = 0U,
    MOD_IMU_STATE_SETTLE,
    MOD_IMU_STATE_CALIBRATING,
    MOD_IMU_STATE_RUNNING
} mod_imu_state_t;

typedef struct
{
    uint32_t zero_sample_count;
    uint32_t zero_sample_interval_ms;
    uint32_t report_interval_ms;
    uint32_t cal_progress_step;
    uint32_t retry_interval_ms;
    uint32_t startup_settle_ms;
    const dev_mpu6050_init_config_t *device_init_config;
} mod_imu_config_t;

typedef struct
{
    int32_t gyro_bias_x;
    int32_t gyro_bias_y;
    int32_t gyro_bias_z;
    int32_t accel_zero_x;
    int32_t accel_zero_y;
    int32_t accel_zero_z;
    int32_t pitch_zero_mdeg;
} mod_imu_calibration_t;

typedef enum
{
    MOD_IMU_EVENT_NONE = 0U,
    MOD_IMU_EVENT_INIT_RETRY,
    MOD_IMU_EVENT_WHOAMI_READ_ERR,
    MOD_IMU_EVENT_WHOAMI_OK,
    MOD_IMU_EVENT_RAW_TEST_ERR,
    MOD_IMU_EVENT_RAW_TEST_OK,
    MOD_IMU_EVENT_CAL_PROGRESS,
    MOD_IMU_EVENT_CAL_READ_ERR,
    MOD_IMU_EVENT_CAL_DONE
} mod_imu_event_id_t;

typedef struct
{
    mod_imu_event_id_t id;
    uint32_t sample_index;
    uint8_t who_am_i;
} mod_imu_event_t;

typedef struct
{
    uint8_t calibration_ok;
    dev_mpu6050_raw_data_t raw_data;
    int32_t gyro_x_zero_relative;
    int32_t gyro_y_zero_relative;
    int32_t gyro_z_zero_relative;
    int32_t pitch_zero_relative_mdeg;
} mod_imu_report_t;

typedef struct
{
    dev_mpu6050_t *device;
    mod_imu_config_t config;
    mod_imu_state_t state;
    uint8_t calibration_ok;
    uint32_t retry_last_ms;
    uint32_t next_action_ms;
    uint32_t cal_sample_index;
    int64_t cal_sum_accel_x;
    int64_t cal_sum_accel_y;
    int64_t cal_sum_accel_z;
    int64_t cal_sum_gyro_x;
    int64_t cal_sum_gyro_y;
    int64_t cal_sum_gyro_z;
    mod_imu_calibration_t calibration;
} mod_imu_t;

extern const mod_imu_config_t g_mod_imu_default_config;

HAL_StatusTypeDef mod_imu_init(mod_imu_t *imu, dev_mpu6050_t *device, const mod_imu_config_t *config);
void mod_imu_reset(mod_imu_t *imu, uint32_t now_ms);
mod_imu_event_t mod_imu_task(mod_imu_t *imu, uint32_t now_ms);
HAL_StatusTypeDef mod_imu_read_report(mod_imu_t *imu, uint32_t now_ms, mod_imu_report_t *report);
uint8_t mod_imu_is_running(const mod_imu_t *imu);
const mod_imu_calibration_t *mod_imu_get_calibration(const mod_imu_t *imu);

#endif
