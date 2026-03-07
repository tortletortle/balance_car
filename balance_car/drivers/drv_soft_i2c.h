#ifndef DRV_SOFT_I2C_H
#define DRV_SOFT_I2C_H

#include <stdint.h>

#include "stm32f1xx_hal.h"

typedef void (*drv_soft_i2c_delay_us_fn_t)(uint32_t delay_us);

typedef struct
{
    GPIO_TypeDef *scl_port;
    uint16_t scl_pin;
    GPIO_TypeDef *sda_port;
    uint16_t sda_pin;
    uint16_t bit_delay_us;
    drv_soft_i2c_delay_us_fn_t delay_us;
} drv_soft_i2c_bus_t;

HAL_StatusTypeDef drv_soft_i2c_init(drv_soft_i2c_bus_t *bus);
HAL_StatusTypeDef drv_soft_i2c_start(drv_soft_i2c_bus_t *bus);
HAL_StatusTypeDef drv_soft_i2c_stop(drv_soft_i2c_bus_t *bus);
HAL_StatusTypeDef drv_soft_i2c_write_byte_raw(drv_soft_i2c_bus_t *bus, uint8_t byte_value);
HAL_StatusTypeDef drv_soft_i2c_read_byte_raw(drv_soft_i2c_bus_t *bus, uint8_t *byte_value);
HAL_StatusTypeDef drv_soft_i2c_write_ack(drv_soft_i2c_bus_t *bus, uint8_t ack_bit);
HAL_StatusTypeDef drv_soft_i2c_read_ack(drv_soft_i2c_bus_t *bus, uint8_t *ack_bit);
HAL_StatusTypeDef drv_soft_i2c_write_reg8(drv_soft_i2c_bus_t *bus, uint8_t device_addr7, uint8_t reg_addr, uint8_t value);
HAL_StatusTypeDef drv_soft_i2c_read_reg8(drv_soft_i2c_bus_t *bus, uint8_t device_addr7, uint8_t reg_addr, uint8_t *value);
HAL_StatusTypeDef drv_soft_i2c_write_mem(drv_soft_i2c_bus_t *bus, uint8_t device_addr7, uint8_t reg_addr, const uint8_t *data, uint16_t length);
HAL_StatusTypeDef drv_soft_i2c_read_mem(drv_soft_i2c_bus_t *bus, uint8_t device_addr7, uint8_t reg_addr, uint8_t *data, uint16_t length);

#endif
