#ifndef __TRACKING_H
#define __TRACKING_H

#include "stm32f10x.h"
#include "stdint.h"
#include "stdbool.h"

void nav_start(uint8_t target_room);
uint8_t nav_get_state(void);
void tracking_init(void);
void tracking_update(void);

#endif
