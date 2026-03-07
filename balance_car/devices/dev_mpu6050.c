#include "dev_mpu6050.h"
#include "dev_mpu6050_reg.h"

const dev_mpu6050_init_config_t g_dev_mpu6050_default_init_config =
{
    0x09U,
    0x06U,
    0x18U,
    0x18U,
    0x01U,
    0x00U
};

static int16_t dev_mpu6050_make_int16(uint8_t msb, uint8_t lsb)
{
    return (int16_t)((((uint16_t)msb) << 8) | lsb);
}

static HAL_StatusTypeDef dev_mpu6050_read_word(dev_mpu6050_t *device, uint8_t reg_addr_high, int16_t *word_value)
{
    HAL_StatusTypeDef status;
    uint8_t high_byte;
    uint8_t low_byte;

    if ((device == NULL) || (word_value == NULL))
    {
        return HAL_ERROR;
    }

    status = dev_mpu6050_read_reg(device, reg_addr_high, &high_byte);
    if (status != HAL_OK)
    {
        return status;
    }

    status = dev_mpu6050_read_reg(device, (uint8_t)(reg_addr_high + 1U), &low_byte);
    if (status != HAL_OK)
    {
        return status;
    }

    *word_value = dev_mpu6050_make_int16(high_byte, low_byte);
    return HAL_OK;
}

HAL_StatusTypeDef dev_mpu6050_bind(dev_mpu6050_t *device, drv_soft_i2c_bus_t *bus, uint8_t address_7bit)
{
    if ((device == NULL) || (bus == NULL))
    {
        return HAL_ERROR;
    }

    device->bus = bus;
    device->address_7bit = address_7bit;
    return HAL_OK;
}

HAL_StatusTypeDef dev_mpu6050_read_reg(dev_mpu6050_t *device, uint8_t reg_addr, uint8_t *value)
{
    if ((device == NULL) || (device->bus == NULL) || (value == NULL))
    {
        return HAL_ERROR;
    }

    return drv_soft_i2c_read_reg8(device->bus, device->address_7bit, reg_addr, value);
}

HAL_StatusTypeDef dev_mpu6050_write_reg(dev_mpu6050_t *device, uint8_t reg_addr, uint8_t value)
{
    if ((device == NULL) || (device->bus == NULL))
    {
        return HAL_ERROR;
    }

    return drv_soft_i2c_write_reg8(device->bus, device->address_7bit, reg_addr, value);
}

HAL_StatusTypeDef dev_mpu6050_read_who_am_i(dev_mpu6050_t *device, uint8_t *who_am_i)
{
    return dev_mpu6050_read_reg(device, DEV_MPU6050_REG_WHO_AM_I, who_am_i);
}

HAL_StatusTypeDef dev_mpu6050_init(dev_mpu6050_t *device, const dev_mpu6050_init_config_t *config)
{
    HAL_StatusTypeDef status;
    uint8_t who_am_i;

    if ((device == NULL) || (device->bus == NULL) || (config == NULL))
    {
        return HAL_ERROR;
    }

    status = dev_mpu6050_read_who_am_i(device, &who_am_i);
    if ((status != HAL_OK) || (who_am_i != DEV_MPU6050_WHO_AM_I_EXPECTED))
    {
        return HAL_ERROR;
    }

    status = dev_mpu6050_write_reg(device, DEV_MPU6050_REG_PWR_MGMT_1, config->pwr_mgmt_1);
    if (status != HAL_OK) { return status; }

    status = dev_mpu6050_write_reg(device, DEV_MPU6050_REG_PWR_MGMT_2, config->pwr_mgmt_2);
    if (status != HAL_OK) { return status; }

    status = dev_mpu6050_write_reg(device, DEV_MPU6050_REG_SMPLRT_DIV, config->sample_rate_div);
    if (status != HAL_OK) { return status; }

    status = dev_mpu6050_write_reg(device, DEV_MPU6050_REG_CONFIG, config->dlpf_config);
    if (status != HAL_OK) { return status; }

    status = dev_mpu6050_write_reg(device, DEV_MPU6050_REG_GYRO_CONFIG, config->gyro_config);
    if (status != HAL_OK) { return status; }

    return dev_mpu6050_write_reg(device, DEV_MPU6050_REG_ACCEL_CONFIG, config->accel_config);
}

HAL_StatusTypeDef dev_mpu6050_read_raw(dev_mpu6050_t *device, dev_mpu6050_raw_data_t *raw_data)
{
    HAL_StatusTypeDef status;

    if ((device == NULL) || (device->bus == NULL) || (raw_data == NULL))
    {
        return HAL_ERROR;
    }

    status = dev_mpu6050_read_word(device, DEV_MPU6050_REG_ACCEL_XOUT_H, &raw_data->accel_x);
    if (status != HAL_OK)
    {
        return status;
    }

    status = dev_mpu6050_read_word(device, (uint8_t)(DEV_MPU6050_REG_ACCEL_XOUT_H + 2U), &raw_data->accel_y);
    if (status != HAL_OK)
    {
        return status;
    }

    status = dev_mpu6050_read_word(device, (uint8_t)(DEV_MPU6050_REG_ACCEL_XOUT_H + 4U), &raw_data->accel_z);
    if (status != HAL_OK)
    {
        return status;
    }

    raw_data->temperature = 0;

    status = dev_mpu6050_read_word(device, DEV_MPU6050_REG_GYRO_XOUT_H, &raw_data->gyro_x);
    if (status != HAL_OK)
    {
        return status;
    }

    status = dev_mpu6050_read_word(device, (uint8_t)(DEV_MPU6050_REG_GYRO_XOUT_H + 2U), &raw_data->gyro_y);
    if (status != HAL_OK)
    {
        return status;
    }

    return dev_mpu6050_read_word(device, (uint8_t)(DEV_MPU6050_REG_GYRO_XOUT_H + 4U), &raw_data->gyro_z);
}