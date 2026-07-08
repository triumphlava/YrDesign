#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f10x.h"
#include "stdbool.h"

typedef struct {
    uint8_t judge;  /* 按键状态标志位 */
    uint8_t state;   /* 按键计数器 */
    uint8_t time;    /* 按键按下时间计数 */
    uint8_t single;  /* 单次按键标志位 */
    uint8_t long_press; /* 长按标志位 */
} button_t;

extern button_t button[2];  

void button_init(void);
void button_scan(void);

#endif

