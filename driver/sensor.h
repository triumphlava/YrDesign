#ifndef __SENSOR_H
#define __SENSOR_H

#include "stm32f10x.h"

//use Sensor_Init() in main
//use Sensor_Scan(uint8_t *digital) when you want to get the digital value of the sensor

void Sensor_Init(void);
void Sensor_Scan(uint8_t *digital);

#endif
