#include "drv_com.h"
#include "stdarg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"


// 接收回调函数
void drv_com_m_handle(unsigned char data){
	drv_com0_printf("%c",data);
}

// ------------------串口定时输出,还有设置GPIO -------
static void echo_task()
{
    GPIO_SET_OUTPUT(2);
   
    drv_com_init(COM_0, 115200, drv_com_m_handle);
    while (1)
    {
         // drv_com0_printf("hello kirito\r\n");
         vTaskDelay(1000 /portTICK_RATE_MS);
         GPIO_Set_Level(2,0);
         vTaskDelay(1000 /portTICK_RATE_MS);
         GPIO_Set_Level(2,1);
    }
}