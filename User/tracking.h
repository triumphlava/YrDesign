#ifndef __TRACKING_H
#define __TRACKING_H

#include "stm32f10x.h"
#include "stdint.h"
#include "stdbool.h"

/* ================================================================
 *  循迹模块 (Line Tracking)
 *  用户将具体循迹逻辑实现在 tracking.c 中
 * ================================================================ */

void tracking_init(void);
void tracking_update(void);

#endif
