#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f10x.h"
#include "stdbool.h"

typedef struct {
    uint8_t judge;
    uint8_t state;   
    uint8_t time;
    uint8_t single;
    uint8_t long_press;
} button_t;

void button_init(void);
void button_scan(void);

#endif

