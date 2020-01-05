#include "dev_sntp.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "lwip/apps/sntp.h"


// 全局变量
static time_t now;
static struct tm timeinfo;
static char is_time_sucess = 0;

static const char *TAG = "sntp_example";

/**
* @ Function Name :  initialize_sntp
* @ Author        : ygl
* @ Brief         : 获得时间初始化
* @ Date          : 2019.03.05
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

/**
* @ Function Name :  obtain_time
* @ Author        : ygl
* @ Brief         : 获得时间
* @ Date          : 2019.03.05
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
static void obtain_time(void)
{
    initialize_sntp();
    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}

/**
* @ Function Name :  sntp_task
* @ Author        : ygl
* @ Brief         : 获得时间任务
* @ Date          : 2019.03.05
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
static void sntp_task(void *arg)
{
    time(&now);
    localtime_r(&now, &timeinfo);

    // 查看本地时间 是否设置
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
    }
   // 设置中国时区
    setenv("TZ", "CST-8", 1);
    tzset();
    is_time_sucess = 1;
    while (1) {
        // update 'now' variable with current time
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}



// ---- 对外接口

/**
* @ Function Name : dev_sntp_init
* @ Author        : ygl
* @ Brief         : 初始化
* @ Date          : 2019.03.05
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
void dev_sntp_init(){
    xTaskCreate (sntp_task, "sntp", 4096, NULL, 10, NULL);
}

/**
* @ Function Name :   dev_sntp_get_time
* @ Author        : ygl
* @ Brief         : 获得时间
* @ Date          : 2019.03.05
* @ Input         : struct tm *time_buffer :时间指针
* @ Output		  : null
* @ Modify        : ...
**/
int dev_sntp_get_time(struct tm *time_buffer)
{   
    if(is_time_sucess){
        time(&now);
        localtime_r(&now, &timeinfo);
        if (timeinfo.tm_year < (2016 - 1900)) {
            return -1;
        } else {
            *time_buffer = timeinfo;
            return 0;
        }

    }else{
        return -1;
    }
}


