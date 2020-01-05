
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "cJSON.h"

#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"


// ------------------nvs存储任务 -------
 void nvs_task(void *pvParameters)
 { 
    
    dev_nvs_init();

    nvs_handle handle;
    static const char *NVS_CUSTOMER = "customer data";
    static const char *DATA1 = "param 1";
    static const char *DATA2 = "param 2";
    static const char *DATA3 = "param 3";

    dev_nvs_write_i32(NVS_CUSTOMER, DATA1, 1);
    int data1;
    dev_nvs_read_i32(NVS_CUSTOMER, DATA1, &data1);
    drv_com0_printf("nvs:%d\r\n",data1);

    char strtemp[30];
    int  len = 28;
    dev_nvs_write_string(NVS_CUSTOMER, DATA2, "hello guojin");
    dev_nvs_read_string(NVS_CUSTOMER, DATA2,&strtemp, &len);
    drv_com0_printf("nvs:%s\r\n",strtemp);

    int data3 = 5;
    int data3_len = 4;
    int data3_out;
    dev_nvs_write_blob(NVS_CUSTOMER, DATA3, (int *)&data3, 4);
    dev_nvs_read_blob(NVS_CUSTOMER, DATA3, (int *)&data3_out, &data3_len);
    drv_com0_printf("nvs:%d\r\n",data3_out);

    while (1)
    {
        vTaskDelay(1000 / portTICK_RATE_MS);
        ;
    }
 }
