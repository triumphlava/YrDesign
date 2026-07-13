#ifndef __TRACKING_H
#define __TRACKING_H

#include "stm32f10x.h"
#include "stdint.h"

/* Call nav_start(room) after receiving target room (1..8).
   nav_get_state() returns: 0=idle, 1=forward, 2=pause, 3=turning,
   4=return, 5=done */
void nav_start(uint8_t target_room);
uint8_t nav_get_state(void);

void tracking_init(void);
void tracking_update(void);

#endif
