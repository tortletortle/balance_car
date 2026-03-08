#include "dev_tb6612.h"

const dev_tb6612_config_t g_dev_tb6612_default_config =
{
    DRV_EN_GPIO_Port,
    DRV_EN_Pin,
    AIN1_GPIO_Port,
    AIN1_Pin,
    AIN2_GPIO_Port,
    AIN2_Pin,
    BIN1_GPIO_Port,
    BIN1_Pin,
    BIN2_GPIO_Port,
    BIN2_Pin,
    0U,
    0U
};

static int16_t dev_tb6612_abs_i16(int16_t value)
{
    return (value >= 0) ? value : (int16_t)(-value);
}

static int16_t dev_tb6612_apply_sign_inversion(int16_t value, uint8_t invert)
{
    if (invert == 0U)
    {
        return value;
    }

    return (int16_t)(-value);
}

static void dev_tb6612_apply_direction(GPIO_TypeDef *in1_port,
                                       uint16_t in1_pin,
                                       GPIO_TypeDef *in2_port,
                                       uint16_t in2_pin,
                                       int16_t pwm_value)
{
    if (pwm_value == 0)
    {
        HAL_GPIO_WritePin(in1_port, in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(in2_port, in2_pin, GPIO_PIN_RESET);
        return;
    }

    if (pwm_value > 0)
    {
        HAL_GPIO_WritePin(in1_port, in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(in2_port, in2_pin, GPIO_PIN_SET);
        return;
    }

    HAL_GPIO_WritePin(in1_port, in1_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(in2_port, in2_pin, GPIO_PIN_RESET);
}

HAL_StatusTypeDef dev_tb6612_init(dev_tb6612_t *device,
                                  drv_pwm_t *pwm_motor_a,
                                  drv_pwm_t *pwm_motor_b,
                                  const dev_tb6612_config_t *config)
{
    if ((device == NULL) || (pwm_motor_a == NULL) || (pwm_motor_b == NULL) || (config == NULL))
    {
        return HAL_ERROR;
    }

    device->pwm_motor_a = pwm_motor_a;
    device->pwm_motor_b = pwm_motor_b;
    device->config = *config;
    device->enabled = 0U;
    device->last_pwm_motor_a = 0;
    device->last_pwm_motor_b = 0;
    return HAL_OK;
}

HAL_StatusTypeDef dev_tb6612_start(dev_tb6612_t *device)
{
    if ((device == NULL) || (device->pwm_motor_a == NULL) || (device->pwm_motor_b == NULL))
    {
        return HAL_ERROR;
    }

    if (drv_pwm_start(device->pwm_motor_a) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (drv_pwm_start(device->pwm_motor_b) != HAL_OK)
    {
        return HAL_ERROR;
    }

    dev_tb6612_stop(device);
    return HAL_OK;
}

void dev_tb6612_set_enable(dev_tb6612_t *device, uint8_t enable)
{
    if (device == NULL)
    {
        return;
    }

    device->enabled = (enable != 0U) ? 1U : 0U;
    HAL_GPIO_WritePin(device->config.drv_en_port,
                      device->config.drv_en_pin,
                      (device->enabled != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void dev_tb6612_stop(dev_tb6612_t *device)
{
    if (device == NULL)
    {
        return;
    }

    (void)drv_pwm_set_compare(device->pwm_motor_a, 0U);
    (void)drv_pwm_set_compare(device->pwm_motor_b, 0U);
    dev_tb6612_apply_direction(device->config.ain1_port,
                               device->config.ain1_pin,
                               device->config.ain2_port,
                               device->config.ain2_pin,
                               0);
    dev_tb6612_apply_direction(device->config.bin1_port,
                               device->config.bin1_pin,
                               device->config.bin2_port,
                               device->config.bin2_pin,
                               0);
    device->last_pwm_motor_a = 0;
    device->last_pwm_motor_b = 0;
    dev_tb6612_set_enable(device, 0U);
}

HAL_StatusTypeDef dev_tb6612_set_dual_pwm(dev_tb6612_t *device, int16_t pwm_motor_a, int16_t pwm_motor_b)
{
    uint16_t duty_motor_a;
    uint16_t duty_motor_b;
    uint16_t period_motor_a;
    uint16_t period_motor_b;

    if ((device == NULL) || (device->pwm_motor_a == NULL) || (device->pwm_motor_b == NULL))
    {
        return HAL_ERROR;
    }

    pwm_motor_a = dev_tb6612_apply_sign_inversion(pwm_motor_a, device->config.invert_motor_a);
    pwm_motor_b = dev_tb6612_apply_sign_inversion(pwm_motor_b, device->config.invert_motor_b);

    period_motor_a = drv_pwm_get_period(device->pwm_motor_a);
    period_motor_b = drv_pwm_get_period(device->pwm_motor_b);
    duty_motor_a = (uint16_t)dev_tb6612_abs_i16(pwm_motor_a);
    duty_motor_b = (uint16_t)dev_tb6612_abs_i16(pwm_motor_b);

    if (duty_motor_a > period_motor_a)
    {
        duty_motor_a = period_motor_a;
    }

    if (duty_motor_b > period_motor_b)
    {
        duty_motor_b = period_motor_b;
    }

    if (device->enabled == 0U)
    {
        dev_tb6612_stop(device);
        return HAL_OK;
    }

    dev_tb6612_apply_direction(device->config.ain1_port,
                               device->config.ain1_pin,
                               device->config.ain2_port,
                               device->config.ain2_pin,
                               pwm_motor_a);
    dev_tb6612_apply_direction(device->config.bin1_port,
                               device->config.bin1_pin,
                               device->config.bin2_port,
                               device->config.bin2_pin,
                               pwm_motor_b);

    if (drv_pwm_set_compare(device->pwm_motor_a, duty_motor_a) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (drv_pwm_set_compare(device->pwm_motor_b, duty_motor_b) != HAL_OK)
    {
        return HAL_ERROR;
    }

    device->last_pwm_motor_a = pwm_motor_a;
    device->last_pwm_motor_b = pwm_motor_b;
    return HAL_OK;
}
