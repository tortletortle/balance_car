#ifndef DEV_TB6612_H
#define DEV_TB6612_H

#include <stdint.h>

#include "main.h"
#include "stm32f1xx_hal.h"
#include "drv_pwm.h"

typedef struct
{
    GPIO_TypeDef *drv_en_port;
    uint16_t drv_en_pin;
    GPIO_TypeDef *ain1_port;
    uint16_t ain1_pin;
    GPIO_TypeDef *ain2_port;
    uint16_t ain2_pin;
    GPIO_TypeDef *bin1_port;
    uint16_t bin1_pin;
    GPIO_TypeDef *bin2_port;
    uint16_t bin2_pin;
    uint8_t invert_motor_a;
    uint8_t invert_motor_b;
} dev_tb6612_config_t;

typedef struct
{
    drv_pwm_t *pwm_motor_a;
    drv_pwm_t *pwm_motor_b;
    dev_tb6612_config_t config;
    uint8_t enabled;
    int16_t last_pwm_motor_a;
    int16_t last_pwm_motor_b;
} dev_tb6612_t;

extern const dev_tb6612_config_t g_dev_tb6612_default_config;

HAL_StatusTypeDef dev_tb6612_init(dev_tb6612_t *device,
                                  drv_pwm_t *pwm_motor_a,
                                  drv_pwm_t *pwm_motor_b,
                                  const dev_tb6612_config_t *config);
HAL_StatusTypeDef dev_tb6612_start(dev_tb6612_t *device);
void dev_tb6612_set_enable(dev_tb6612_t *device, uint8_t enable);
void dev_tb6612_stop(dev_tb6612_t *device);
HAL_StatusTypeDef dev_tb6612_set_dual_pwm(dev_tb6612_t *device, int16_t pwm_motor_a, int16_t pwm_motor_b);

#endif
