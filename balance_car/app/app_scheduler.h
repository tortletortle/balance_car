#ifndef APP_SCHEDULER_H
#define APP_SCHEDULER_H

#include <stdint.h>

#include "stm32f1xx_hal.h"

typedef enum
{
    APP_SCHED_FLAG_NONE = 0U,
    APP_SCHED_FLAG_HEARTBEAT = 1U << 0,
    APP_SCHED_FLAG_BATTERY = 1U << 1,
    APP_SCHED_FLAG_CONTROL = 1U << 2,
    APP_SCHED_FLAG_STATUS = 1U << 3
} app_scheduler_flag_t;

typedef struct
{
    uint32_t heartbeat_period_ms;
    uint32_t battery_period_ms;
    uint32_t control_period_ms;
    uint32_t status_period_ms;
} app_scheduler_config_t;

typedef struct
{
    app_scheduler_config_t config;
    uint8_t initialized;
    uint32_t heartbeat_last_ms;
    uint32_t battery_last_ms;
    uint32_t control_last_ms;
    uint32_t status_last_ms;
} app_scheduler_t;

extern const app_scheduler_config_t g_app_scheduler_default_config;

HAL_StatusTypeDef app_scheduler_init(app_scheduler_t *scheduler,
                                     const app_scheduler_config_t *config);
void app_scheduler_reset(app_scheduler_t *scheduler, uint32_t now_ms);
uint32_t app_scheduler_poll(app_scheduler_t *scheduler, uint32_t now_ms);

#endif
