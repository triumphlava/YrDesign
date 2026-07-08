#ifndef __APP_TASKS_H
#define __APP_TASKS_H

#include "stdint.h"

void app_scheduler_tick(void);

extern volatile uint8_t g_tracking_flag;
extern volatile uint8_t g_button_flag;

#endif
