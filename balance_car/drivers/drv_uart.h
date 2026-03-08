#ifndef DRV_UART_H
#define DRV_UART_H

#include <stdint.h>

#include "stm32f1xx_hal.h"

typedef struct
{
    UART_HandleTypeDef *huart;
} drv_uart_t;

HAL_StatusTypeDef drv_uart_init(drv_uart_t *uart, UART_HandleTypeDef *huart);
HAL_StatusTypeDef drv_uart_transmit(drv_uart_t *uart,
                                    const uint8_t *data,
                                    uint16_t length,
                                    uint32_t timeout_ms);
HAL_StatusTypeDef drv_uart_start_receive_byte_it(drv_uart_t *uart, uint8_t *rx_byte);
void drv_uart_clear_overrun(drv_uart_t *uart);
uint8_t drv_uart_matches_handle(const drv_uart_t *uart, const UART_HandleTypeDef *huart);

#endif
