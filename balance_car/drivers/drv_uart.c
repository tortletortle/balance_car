#include "drv_uart.h"

HAL_StatusTypeDef drv_uart_init(drv_uart_t *uart, UART_HandleTypeDef *huart)
{
    if ((uart == NULL) || (huart == NULL))
    {
        return HAL_ERROR;
    }

    uart->huart = huart;
    return HAL_OK;
}

HAL_StatusTypeDef drv_uart_transmit(drv_uart_t *uart,
                                    const uint8_t *data,
                                    uint16_t length,
                                    uint32_t timeout_ms)
{
    if ((uart == NULL) || (uart->huart == NULL) || (data == NULL) || (length == 0U))
    {
        return HAL_ERROR;
    }

    return HAL_UART_Transmit(uart->huart, (uint8_t *)data, length, timeout_ms);
}

HAL_StatusTypeDef drv_uart_start_receive_byte_it(drv_uart_t *uart, uint8_t *rx_byte)
{
    if ((uart == NULL) || (uart->huart == NULL) || (rx_byte == NULL))
    {
        return HAL_ERROR;
    }

    if (HAL_UART_Receive_IT(uart->huart, rx_byte, 1U) == HAL_OK)
    {
        return HAL_OK;
    }

    (void)HAL_UART_AbortReceive(uart->huart);
    if (HAL_UART_Receive_IT(uart->huart, rx_byte, 1U) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

void drv_uart_clear_overrun(drv_uart_t *uart)
{
    if ((uart == NULL) || (uart->huart == NULL))
    {
        return;
    }

    __HAL_UART_CLEAR_OREFLAG(uart->huart);
}

uint8_t drv_uart_matches_handle(const drv_uart_t *uart, const UART_HandleTypeDef *huart)
{
    if ((uart == NULL) || (uart->huart == NULL) || (huart == NULL))
    {
        return 0U;
    }

    return (uart->huart == huart) ? 1U : 0U;
}
