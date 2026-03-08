#include "drv_soft_i2c.h"

static uint16_t drv_soft_i2c_get_delay_us(const drv_soft_i2c_bus_t *bus)
{
    if ((bus == NULL) || (bus->bit_delay_us == 0U))
    {
        return 5U;
    }

    return bus->bit_delay_us;
}

static uint8_t drv_soft_i2c_is_valid(const drv_soft_i2c_bus_t *bus)
{
    if ((bus == NULL) || (bus->scl_port == NULL) || (bus->sda_port == NULL) || (bus->delay_us == NULL))
    {
        return 0U;
    }

    return 1U;
}

static void drv_soft_i2c_delay(const drv_soft_i2c_bus_t *bus)
{
    bus->delay_us(drv_soft_i2c_get_delay_us(bus));
}

static void drv_soft_i2c_enable_gpio_clock(GPIO_TypeDef *port)
{
    if (port == GPIOA)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else if (port == GPIOB)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else if (port == GPIOC)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    else if (port == GPIOD)
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
    else if (port == GPIOE)
    {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    }
}

static void drv_soft_i2c_write_scl(const drv_soft_i2c_bus_t *bus, GPIO_PinState pin_state)
{
    HAL_GPIO_WritePin(bus->scl_port, bus->scl_pin, pin_state);
    drv_soft_i2c_delay(bus);
}

static void drv_soft_i2c_write_sda(const drv_soft_i2c_bus_t *bus, GPIO_PinState pin_state)
{
    HAL_GPIO_WritePin(bus->sda_port, bus->sda_pin, pin_state);
    drv_soft_i2c_delay(bus);
}

static GPIO_PinState drv_soft_i2c_read_sda(const drv_soft_i2c_bus_t *bus)
{
    return HAL_GPIO_ReadPin(bus->sda_port, bus->sda_pin);
}

static HAL_StatusTypeDef drv_soft_i2c_send_address(drv_soft_i2c_bus_t *bus, uint8_t device_addr7, uint8_t read_request)
{
    uint8_t ack_bit;
    HAL_StatusTypeDef status;
    uint8_t address_byte;

    address_byte = (uint8_t)((device_addr7 << 1) | (read_request & 0x01U));
    status = drv_soft_i2c_write_byte_raw(bus, address_byte);
    if (status != HAL_OK)
    {
        return status;
    }

    status = drv_soft_i2c_read_ack(bus, &ack_bit);
    if ((status != HAL_OK) || (ack_bit != 0U))
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef drv_soft_i2c_init(drv_soft_i2c_bus_t *bus)
{
    GPIO_InitTypeDef gpio_init;

    if (drv_soft_i2c_is_valid(bus) == 0U)
    {
        return HAL_ERROR;
    }

    drv_soft_i2c_enable_gpio_clock(bus->scl_port);
    drv_soft_i2c_enable_gpio_clock(bus->sda_port);

    gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;

    if (bus->sda_port == bus->scl_port)
    {
        gpio_init.Pin = bus->scl_pin | bus->sda_pin;
        HAL_GPIO_Init(bus->scl_port, &gpio_init);
    }
    else
    {
        gpio_init.Pin = bus->scl_pin;
        HAL_GPIO_Init(bus->scl_port, &gpio_init);
        gpio_init.Pin = bus->sda_pin;
        HAL_GPIO_Init(bus->sda_port, &gpio_init);
    }

    HAL_GPIO_WritePin(bus->scl_port, bus->scl_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(bus->sda_port, bus->sda_pin, GPIO_PIN_SET);
    drv_soft_i2c_delay(bus);

    return drv_soft_i2c_bus_recover(bus);
}

HAL_StatusTypeDef drv_soft_i2c_bus_recover(drv_soft_i2c_bus_t *bus)
{
    uint8_t pulse_index;

    if (drv_soft_i2c_is_valid(bus) == 0U)
    {
        return HAL_ERROR;
    }

    drv_soft_i2c_write_sda(bus, GPIO_PIN_SET);
    drv_soft_i2c_write_scl(bus, GPIO_PIN_SET);

    for (pulse_index = 0U; pulse_index < 9U; pulse_index++)
    {
        drv_soft_i2c_write_scl(bus, GPIO_PIN_RESET);
        drv_soft_i2c_write_scl(bus, GPIO_PIN_SET);
    }

    drv_soft_i2c_write_sda(bus, GPIO_PIN_RESET);
    drv_soft_i2c_write_scl(bus, GPIO_PIN_SET);
    drv_soft_i2c_write_sda(bus, GPIO_PIN_SET);

    return HAL_OK;
}

HAL_StatusTypeDef drv_soft_i2c_start(drv_soft_i2c_bus_t *bus)
{
    if (drv_soft_i2c_is_valid(bus) == 0U)
    {
        return HAL_ERROR;
    }

    drv_soft_i2c_write_sda(bus, GPIO_PIN_SET);
    drv_soft_i2c_write_scl(bus, GPIO_PIN_SET);
    drv_soft_i2c_write_sda(bus, GPIO_PIN_RESET);
    drv_soft_i2c_write_scl(bus, GPIO_PIN_RESET);

    return HAL_OK;
}

HAL_StatusTypeDef drv_soft_i2c_stop(drv_soft_i2c_bus_t *bus)
{
    if (drv_soft_i2c_is_valid(bus) == 0U)
    {
        return HAL_ERROR;
    }

    drv_soft_i2c_write_sda(bus, GPIO_PIN_RESET);
    drv_soft_i2c_write_scl(bus, GPIO_PIN_SET);
    drv_soft_i2c_write_sda(bus, GPIO_PIN_SET);

    return HAL_OK;
}

HAL_StatusTypeDef drv_soft_i2c_write_byte_raw(drv_soft_i2c_bus_t *bus, uint8_t byte_value)
{
    uint8_t bit_index;

    if (drv_soft_i2c_is_valid(bus) == 0U)
    {
        return HAL_ERROR;
    }

    for (bit_index = 0U; bit_index < 8U; bit_index++)
    {
        drv_soft_i2c_write_sda(bus, ((byte_value & (0x80U >> bit_index)) != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        drv_soft_i2c_write_scl(bus, GPIO_PIN_SET);
        drv_soft_i2c_write_scl(bus, GPIO_PIN_RESET);
    }

    return HAL_OK;
}

HAL_StatusTypeDef drv_soft_i2c_read_byte_raw(drv_soft_i2c_bus_t *bus, uint8_t *byte_value)
{
    uint8_t bit_index;
    uint8_t received_value;

    if ((drv_soft_i2c_is_valid(bus) == 0U) || (byte_value == NULL))
    {
        return HAL_ERROR;
    }

    received_value = 0U;
    drv_soft_i2c_write_sda(bus, GPIO_PIN_SET);

    for (bit_index = 0U; bit_index < 8U; bit_index++)
    {
        drv_soft_i2c_write_scl(bus, GPIO_PIN_SET);
        if (drv_soft_i2c_read_sda(bus) == GPIO_PIN_SET)
        {
            received_value |= (uint8_t)(0x80U >> bit_index);
        }
        drv_soft_i2c_write_scl(bus, GPIO_PIN_RESET);
    }

    *byte_value = received_value;

    return HAL_OK;
}

HAL_StatusTypeDef drv_soft_i2c_write_ack(drv_soft_i2c_bus_t *bus, uint8_t ack_bit)
{
    if (drv_soft_i2c_is_valid(bus) == 0U)
    {
        return HAL_ERROR;
    }

    drv_soft_i2c_write_sda(bus, (ack_bit != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    drv_soft_i2c_write_scl(bus, GPIO_PIN_SET);
    drv_soft_i2c_write_scl(bus, GPIO_PIN_RESET);
    drv_soft_i2c_write_sda(bus, GPIO_PIN_SET);

    return HAL_OK;
}

HAL_StatusTypeDef drv_soft_i2c_read_ack(drv_soft_i2c_bus_t *bus, uint8_t *ack_bit)
{
    if ((drv_soft_i2c_is_valid(bus) == 0U) || (ack_bit == NULL))
    {
        return HAL_ERROR;
    }

    drv_soft_i2c_write_sda(bus, GPIO_PIN_SET);
    drv_soft_i2c_write_scl(bus, GPIO_PIN_SET);
    *ack_bit = (uint8_t)drv_soft_i2c_read_sda(bus);
    drv_soft_i2c_write_scl(bus, GPIO_PIN_RESET);

    return HAL_OK;
}

HAL_StatusTypeDef drv_soft_i2c_write_reg8(drv_soft_i2c_bus_t *bus, uint8_t device_addr7, uint8_t reg_addr, uint8_t value)
{
    return drv_soft_i2c_write_mem(bus, device_addr7, reg_addr, &value, 1U);
}

HAL_StatusTypeDef drv_soft_i2c_read_reg8(drv_soft_i2c_bus_t *bus, uint8_t device_addr7, uint8_t reg_addr, uint8_t *value)
{
    return drv_soft_i2c_read_mem(bus, device_addr7, reg_addr, value, 1U);
}

HAL_StatusTypeDef drv_soft_i2c_write_mem(drv_soft_i2c_bus_t *bus, uint8_t device_addr7, uint8_t reg_addr, const uint8_t *data, uint16_t length)
{
    HAL_StatusTypeDef status;
    uint8_t ack_bit;
    uint16_t index;

    if ((drv_soft_i2c_is_valid(bus) == 0U) || ((length > 0U) && (data == NULL)))
    {
        return HAL_ERROR;
    }

    status = drv_soft_i2c_start(bus);
    if (status != HAL_OK)
    {
        return status;
    }

    status = drv_soft_i2c_send_address(bus, device_addr7, 0U);
    if (status != HAL_OK)
    {
        goto exit_with_stop;
    }

    status = drv_soft_i2c_write_byte_raw(bus, reg_addr);
    if (status != HAL_OK)
    {
        goto exit_with_stop;
    }

    status = drv_soft_i2c_read_ack(bus, &ack_bit);
    if ((status != HAL_OK) || (ack_bit != 0U))
    {
        status = HAL_ERROR;
        goto exit_with_stop;
    }

    for (index = 0U; index < length; index++)
    {
        status = drv_soft_i2c_write_byte_raw(bus, data[index]);
        if (status != HAL_OK)
        {
            goto exit_with_stop;
        }

        status = drv_soft_i2c_read_ack(bus, &ack_bit);
        if ((status != HAL_OK) || (ack_bit != 0U))
        {
            status = HAL_ERROR;
            goto exit_with_stop;
        }
    }

    status = HAL_OK;

exit_with_stop:
    (void)drv_soft_i2c_stop(bus);
    return status;
}

HAL_StatusTypeDef drv_soft_i2c_read_mem(drv_soft_i2c_bus_t *bus, uint8_t device_addr7, uint8_t reg_addr, uint8_t *data, uint16_t length)
{
    HAL_StatusTypeDef status;
    uint8_t ack_bit;
    uint16_t index;

    if ((drv_soft_i2c_is_valid(bus) == 0U) || ((length > 0U) && (data == NULL)))
    {
        return HAL_ERROR;
    }

    status = drv_soft_i2c_start(bus);
    if (status != HAL_OK)
    {
        return status;
    }

    status = drv_soft_i2c_send_address(bus, device_addr7, 0U);
    if (status != HAL_OK)
    {
        goto exit_with_stop;
    }

    status = drv_soft_i2c_write_byte_raw(bus, reg_addr);
    if (status != HAL_OK)
    {
        goto exit_with_stop;
    }

    status = drv_soft_i2c_read_ack(bus, &ack_bit);
    if ((status != HAL_OK) || (ack_bit != 0U))
    {
        status = HAL_ERROR;
        goto exit_with_stop;
    }

    status = drv_soft_i2c_start(bus);
    if (status != HAL_OK)
    {
        goto exit_with_stop;
    }

    status = drv_soft_i2c_send_address(bus, device_addr7, 1U);
    if (status != HAL_OK)
    {
        goto exit_with_stop;
    }

    for (index = 0U; index < length; index++)
    {
        status = drv_soft_i2c_read_byte_raw(bus, &data[index]);
        if (status != HAL_OK)
        {
            goto exit_with_stop;
        }

        status = drv_soft_i2c_write_ack(bus, (index == (length - 1U)) ? 1U : 0U);
        if (status != HAL_OK)
        {
            goto exit_with_stop;
        }
    }

    status = HAL_OK;

exit_with_stop:
    (void)drv_soft_i2c_stop(bus);
    return status;
}
