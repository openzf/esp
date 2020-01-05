
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "drv_com.h"
#include "drv_gpio.h"
#include "drv_pwm.h"
#include "dev_mqtt.h"

#include "http_app_demo.h"
#include "mqtt_app_demo.h"
#include "socket_app_demo.h"
#include "dev_sntp.h"
#include "dev_https.h"

#include "https_app_demo.h"
#include "i2c_app_demo.h"
#include "spi_app_demo.h"
#include "ota.h"
#include "system_module.h"


#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "user_main";


// 如果采用OTA模式
// #define OTA

// ------------------串口定时输出,还有设置GPIO -------
static void led_task()
{
    struct tm time_buffer;
     char strftime_buf[64];
    GPIO_SET_OUTPUT(2);
    while (1)
    {
         // drv_com0_printf("hello kirito\r\n");
         vTaskDelay(1000 /portTICK_RATE_MS);
         GPIO_Set_Level(2,0);
         vTaskDelay(1000 /portTICK_RATE_MS);
         GPIO_Set_Level(2,1);

        //  if (dev_sntp_get_time(&time_buffer) == 0){
        //     strftime(strftime_buf, sizeof(strftime_buf), "%c", &time_buffer);
        //     drv_com0_printf("%s\r\n",strftime_buf);
        //  }else{
        //      drv_com0_printf("error\r\n");
        //  }
    }
}

// 正常模式
void app_normal_mode_run()
{
        // mqtt app测试
    // app_init();

// http app测试
     

    // socket测试
    //  socekt_app_init();

    // http测试
    //  dev_sntp_init();
     

    // dev_https_init();
     //https_demo_init();
    // i2c_app_demo_init();
     // spi_app_demo_init();
     drv_com0_printf("this is noraml main mode\r\n");
     http_app_init();
     // xTaskCreate(led_task, "sntp", 1024, NULL, 10, NULL);
}

// 系统准备就绪,查看是否有可升级的
void app_system_ready()
{   
    #ifdef OTA
    ota_init(app_normal_mode_run);
    #else
     app_normal_mode_run();
     #endif
}

void app_main()
{
    vTaskDelay(1000 /portTICK_RATE_MS);
    app_uart_init();
    system_module_init(app_system_ready);
}
