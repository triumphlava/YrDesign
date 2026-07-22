#include "app_tasks.h"
#include "tim.h"
#include "tracking.h"
#include "button_task.h"
#include "motor.h"
#include "button.h"

void app_tasks_init(void)
{
    tim_register_callback(app_scheduler_tick);
}

void app_scheduler_tick(void)
{
    static uint32_t last_5ms = 0;
    static uint32_t last_10ms = 0;

    uint32_t now = sys_tick_ms;

    if (now - last_5ms >= 5) {
        last_5ms = now;
        tracking_update();
    }

    if (now - last_10ms >= 10) {
        last_10ms = now;
        button_scan();
    }
}
