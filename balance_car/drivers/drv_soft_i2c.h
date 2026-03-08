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

typedef enum
{
    DRV_SOFT_I2C_DIAG_STAGE_IDLE = 0U,
    DRV_SOFT_I2C_DIAG_STAGE_READ_START_WRITE = 1U,
    DRV_SOFT_I2C_DIAG_STAGE_READ_ADDR_WRITE = 2U,
    DRV_SOFT_I2C_DIAG_STAGE_READ_REG = 3U,
    DRV_SOFT_I2C_DIAG_STAGE_READ_REG_ACK = 4U,
    DRV_SOFT_I2C_DIAG_STAGE_READ_START_READ = 5U,
    DRV_SOFT_I2C_DIAG_STAGE_READ_ADDR_READ_SEND = 6U,
    DRV_SOFT_I2C_DIAG_STAGE_READ_ADDR_READ_ACK = 7U,
    DRV_SOFT_I2C_DIAG_STAGE_READ_DATA = 8U,
    DRV_SOFT_I2C_DIAG_STAGE_READ_DATA_ACK = 9U,
    DRV_SOFT_I2C_DIAG_STAGE_READ_STOP = 10U,
    DRV_SOFT_I2C_DIAG_STAGE_READ_DONE = 11U
} drv_soft_i2c_diag_stage_t;

typedef struct
{
    uint8_t stage;
    uint8_t ack_bit;
    uint8_t device_addr7;
    uint8_t reg_addr;
    uint16_t index;
    uint16_t length;
} drv_soft_i2c_diag_t;

void drv_soft_i2c_diag_clear(void);
drv_soft_i2c_diag_t drv_soft_i2c_diag_get(void);

HAL_StatusTypeDef drv_soft_i2c_init(drv_soft_i2c_bus_t *bus);
HAL_StatusTypeDef drv_soft_i2c_bus_recover(drv_soft_i2c_bus_t *bus);
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
