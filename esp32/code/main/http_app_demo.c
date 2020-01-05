#include "http_app_demo.h"
#include "drv_com.h"

#include "dev_http.h"
#include "dev_sntp.h"
#include "drv_gpio.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "cJSON.h"
#include "esp_log.h"
#include "esp_system.h"

#include "esp_wifi.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

static struct tm time_buffer;
/**
* @ Function Name : arg_state_fun
* @ Author        : ygl
* @ Brief         : http 状态回调
* @ Date          : 2018.11.18
* @ Input         : char state ;状态
* @ Output		  : null
* @ Modify        : ...
**/
static void arg_state_fun(char state)
{

}

/**
* @ Function Name : arg_msg_fun
* @ Author        : ygl
* @ Brief         : http 状态回调
* @ Date          : 2018.11.18
* @ Input         : char *data ;数据指针
                    int len :数据长度
* @ Output		  : null
* @ Modify        : ...
**/
static void arg_msg_fun(char *data, int len)
{
    drv_com0_printf("data:\r\n");
    for (int i = 0; i < len; i++)
  {
    drv_com0_printf("%c",data[i]);
  }

    dev_sntp_get_time(&time_buffer);
    int min = time_buffer.tm_min;
    int hour = time_buffer.tm_hour;
    drv_com0_printf("time:%d:%d!\r\n",hour,min);
    // char *rc = strstr(data, "\xe9\x9b\xa8");
    // if (rc != NULL){
    //     // 只有白天才有反应
    //     if (hour >7 && hour < 22){
    //         drv_com0_printf("%d:%d  today is rain!\r\n",hour,min);

    //         GPIO_Set_Level(2,0);
    //         vTaskDelay(1000 /portTICK_RATE_MS);
    //         GPIO_Set_Level(2,1);
    //         vTaskDelay(1000 /portTICK_RATE_MS);
    //         GPIO_Set_Level(2,0);
    //     }
    // }

    // cJSON *pJsonRoot = cJSON_Parse(data);
    // //如果是否json格式数据
    // if (pJsonRoot !=NULL) 
    // {
    //     cJSON *weather = cJSON_GetObjectItem(pJsonRoot, "weather");
    //     if (weather)
    //     {
    //         char *weather_buffer = weather->valuestring;
    //        //  ESP_LOGI("sdf", "webather: %s", weather_buffer);
    //     }
    //     free(weather);
    // }
    // free(pJsonRoot);

}

/**
* @ Function Name :  http_task
* @ Author        : ygl
* @ Brief         : http 任务
* @ Date          : 2018.11.18
* @ Input         : 
* @ Output		  : null
* @ Modify        : ...
**/
 void http_task(void *pvParameters)
{

    // char *get_url = "http://192.168.1.101/home?username=kirito&password=hello";
    // char *post_url = "http://192.168.1.101/home";
    // char *post_data = "username=zero&password=kotlin";

    //char *get_url = "http://timor.tech/api/holiday/info/$dat";
    char *get_url = "http://www.weather.com.cn/data/cityinfo/101270107.html";
    char *post_url = "http://192.168.1.101/home";
    char *post_data = "username=zero&password=kotlin";

    http_init();
    http_regist_state_callback(arg_state_fun);
    http_regist_msg_callback(arg_msg_fun);

     dev_sntp_init();
    //GPIO_SET_OUTPUT(2);
 
    while (1)
    {
        http_send_GET_quest(get_url);
        ESP_LOGI("H", "Free ram size: %d", esp_get_free_heap_size());
        // http_send_POST_quest(post_url, CONTENT_TYPE_A_FORM, post_data);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // for(int countdown = 2; countdown >= 0; countdown--) {
        //     vTaskDelay(1000 / portTICK_PERIOD_MS);
        // }
    
    }
}


/**
* @ Function Name :  http_app_init
* @ Author        : ygl
* @ Brief         : http app初始化
* @ Date          : 2018.11.18
* @ Input         : 
* @ Output		  : null
* @ Modify        : ...
**/
void http_app_init()
{  
     xTaskCreate(http_task, "http_echo_task", 15360, NULL, 10, NULL);
}

