#ifndef DEV_MPU6050_H
#define DEV_MPU6050_H

#include <stdint.h>

#include "stm32f1xx_hal.h"
#include "drv_soft_i2c.h"

#define DEV_MPU6050_DEFAULT_ADDRESS_7BIT   0x68U
#define DEV_MPU6050_WHO_AM_I_EXPECTED      0x68U

typedef struct
{
    uint8_t sample_rate_div;
    uint8_t dlpf_config;
    uint8_t gyro_config;
    uint8_t accel_config;
    uint8_t pwr_mgmt_1;
    uint8_t pwr_mgmt_2;
} dev_mpu6050_init_config_t;

typedef struct
{
    drv_soft_i2c_bus_t *bus;
    uint8_t address_7bit;
} dev_mpu6050_t;

typedef struct
{
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t temperature;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} dev_mpu6050_raw_data_t;

extern const dev_mpu6050_init_config_t g_dev_mpu6050_default_init_config;

HAL_StatusTypeDef dev_mpu6050_bind(dev_mpu6050_t *device, drv_soft_i2c_bus_t *bus, uint8_t address_7bit);
HAL_StatusTypeDef dev_mpu6050_init(dev_mpu6050_t *device, const dev_mpu6050_init_config_t *config);
HAL_StatusTypeDef dev_mpu6050_read_reg(dev_mpu6050_t *device, uint8_t reg_addr, uint8_t *value);
HAL_StatusTypeDef dev_mpu6050_write_reg(dev_mpu6050_t *device, uint8_t reg_addr, uint8_t value);
HAL_StatusTypeDef dev_mpu6050_read_who_am_i(dev_mpu6050_t *device, uint8_t *who_am_i);
HAL_StatusTypeDef dev_mpu6050_read_raw(dev_mpu6050_t *device, dev_mpu6050_raw_data_t *raw_data);

#endif