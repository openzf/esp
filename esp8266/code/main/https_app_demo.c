#include "https_app_demo.h"
#include "dev_https.h"
#include "drv_com.h"
#include "dev_sntp.h"

#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"

#include "nvs_flash.h"
#include "cJSON.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"


#include "lwip/apps/sntp.h"
#include "esp_tls.h"



 // #define WEB_URL "https://www.howsmyssl.com/a/check"
#define WEB_URL "https://tcc.taobao.com/cc/json/mobile_tel_segment.htm?tel=18483656663"

static const char *TAG = "example";


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
     //  ESP_LOGI(TAG, "data ok\r\n");
        for (int i = 0; i < len; i++)
  {
         drv_com0_printf("%c",data[i]);
  }

    // cJSON *pJsonRoot = cJSON_Parse(data);
    // //如果是否json格式数据
    // if (pJsonRoot !=NULL) {
    //     ESP_LOGI(TAG, "is json");
    //     cJSON *pMacAdress = cJSON_GetObjectItem(pJsonRoot, "tls_version");
    //     //判断mac字段是否json格式
    //     if (pMacAdress) {
    //         //判断mac字段是否string类型
    //         if (cJSON_IsString(pMacAdress))
    //             ESP_LOGI(TAG, "data is %s",pMacAdress->valuestring); 
    //     } else
    //     ESP_LOGI(TAG, "json par error");
    // }
 }

/**
* @ Function Name :  https_demo_task
* @ Author        : ygl
* @ Brief         : https 任务
* @ Date          : 2018.11.18
* @ Input         : 
* @ Output		  : null
* @ Modify        : ...
**/
static void https_demo_task(void *pvParameters)
{
    dev_https_init();
    dev_https_regist_state_callback(arg_state_fun);
    dev_https_regist_msg_callback(arg_msg_fun);

    while(1){
         https_system_get_request(WEB_URL);
         ESP_LOGI(TAG, "Free ram size: %d\n", esp_get_free_heap_size());
         vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


/**
* @ Function Name :  https_demo_init
* @ Author        : ygl
* @ Brief         : https app初始化
* @ Date          : 2018.11.18
* @ Input         : 
* @ Output		  : null
* @ Modify        : ...
**/
void https_demo_init()
{
    xTaskCreate(&https_demo_task, "https_get_task",15360, NULL, 5, NULL);
}