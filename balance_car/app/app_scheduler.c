#include "app_scheduler.h"

const app_scheduler_config_t g_app_scheduler_default_config =
{
    1000U,
    50U,
    10U,
    200U
};

static uint32_t app_scheduler_eval(uint32_t now_ms,
                                   uint32_t period_ms,
                                   uint32_t *last_ms,
                                   uint32_t flag)
{
    if ((period_ms == 0U) || (last_ms == NULL))
    {
        return APP_SCHED_FLAG_NONE;
    }

    if ((now_ms - *last_ms) < period_ms)
    {
        return APP_SCHED_FLAG_NONE;
    }

    *last_ms = now_ms;
    return flag;
}

HAL_StatusTypeDef app_scheduler_init(app_scheduler_t *scheduler,
                                     const app_scheduler_config_t *config)
{
    if ((scheduler == NULL) || (config == NULL))
    {
        return HAL_ERROR;
    }

    scheduler->config = *config;
    scheduler->initialized = 1U;
    app_scheduler_reset(scheduler, 0U);
    return HAL_OK;
}

void app_scheduler_reset(app_scheduler_t *scheduler, uint32_t now_ms)
{
    if (scheduler == NULL)
    {
        return;
    }

    scheduler->heartbeat_last_ms = now_ms;
    scheduler->battery_last_ms = now_ms;
    scheduler->control_last_ms = now_ms;
    scheduler->status_last_ms = now_ms;
}

uint32_t app_scheduler_poll(app_scheduler_t *scheduler, uint32_t now_ms)
{
    uint32_t flags;

    if ((scheduler == NULL) || (scheduler->initialized == 0U))
    {
        return APP_SCHED_FLAG_NONE;
    }

    flags = APP_SCHED_FLAG_NONE;
    flags |= app_scheduler_eval(now_ms,
                                scheduler->config.heartbeat_period_ms,
                                &scheduler->heartbeat_last_ms,
                                APP_SCHED_FLAG_HEARTBEAT);
    flags |= app_scheduler_eval(now_ms,
                                scheduler->config.battery_period_ms,
                                &scheduler->battery_last_ms,
                                APP_SCHED_FLAG_BATTERY);
    flags |= app_scheduler_eval(now_ms,
                                scheduler->config.control_period_ms,
                                &scheduler->control_last_ms,
                                APP_SCHED_FLAG_CONTROL);
    flags |= app_scheduler_eval(now_ms,
                                scheduler->config.status_period_ms,
                                &scheduler->status_last_ms,
                                APP_SCHED_FLAG_STATUS);
    return flags;
}
