#ifndef __SENSOR_H
#define __SENSOR_H

#include "stm32f10x.h"

#define SENSOR_CALIB  0    /* 1 = print analog values for calibration via UART */
                            /* 0 = normal mode, print 0/1 digital values          */


/* Initialize grayscale sensor (BSP + GANV driver + calibration). Call once at startup. */
void Sensor_Init(void);

/* Scan 8 channels, write 0/1 into digital[8]. Call periodically in main loop. */
void Sensor_Scan(uint8_t *digital);

/* Print 8 current analog values via UART (calibration). Auto rate-limited to ~500ms.
   Only works when SENSOR_CALIB = 1, otherwise does nothing. */
void Sensor_CalibOutput(void);

#endif
