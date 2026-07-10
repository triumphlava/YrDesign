#include "tracking.h"
#include "motor.h"
#include "usart.h"
#include "sensor.h"


/* ================================================================
 *  浼犳劅鍣ㄥ紩鑴氬畾涔?鈥?鎸夊疄闄呮帴绾夸慨鏀? *  鍋囪 5 璺暟瀛楀惊杩规ā鍧楋紝鐧界嚎=0 / 榛戠嚎=1
 * ================================================================ */

uint8_t tracking_sensor[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // 1 for white, 0 for black

void tracking_init(void)
{
    
}


void tracking_update(void)
{
    Sensor_Scan(tracking_sensor);
    usart_send_string("Sensor: ");
    for (int i = 0; i < 8; i++) {
        usart_send_byte(tracking_sensor[i] ? '1' : '0');
    }
    usart_send_string("\r\n");
}

