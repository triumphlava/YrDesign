#ifndef __SENSOR_H
#define __SENSOR_H

#include "stm32f10x.h"

#define SENSOR_CALIB  0

void Sensor_Init(void);
void Sensor_Scan(uint8_t *digital);
void Sensor_CalibOutput(void);

#endif
