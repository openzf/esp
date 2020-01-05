
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


#include <ctype.h>
#include <errno.h>
#include <time.h>
#include "esp_partition.h"
#include "esp_wifi.h"

#include "../lib/drv_com.h"
#include "../lib/dev_wifi.h"
#include "../lib/dev_nvs.h"
#include "../lib/dev_smart_config.h"

// ------------------wifi 任务 -------
void wifi_scan(WIFI_STATE wifi_state)
{
    if(wifi_state == WIFI_CONNECTED){
    drv_com0_printf("hello this is get ip\r\n");
        // xTaskCreate(http_task, "http task", 4096, NULL, 10, NULL);
    //   xTaskCreate(tcp_socket_task, "udp_client", 4096, NULL, 5, NULL);
    //  xTaskCreate(flash_task, "flash task", 4096, NULL, 5, NULL);
    }
    if (wifi_state == WIFI_DISCONNECTED){
        drv_com0_printf("hello this is disconnt \r\n");
    }
}

void smart_config_fun_getWifiInfo(wifi_config_t wifi_config)
{
    static const char *NVS_CUSTOMER = "wifi_info";
    static const char *DATA = "wifi_1";
    int wifi_config_len = sizeof(wifi_config_t);

    // 存储在flash中
    dev_nvs_write_blob(NVS_CUSTOMER, DATA, ( wifi_config_t *)&wifi_config, wifi_config_len);

    drv_com0_printf( "MAIN_SSID:%s",wifi_config.sta.ssid);
    drv_com0_printf( "MAIN_PASSWORD:%s",wifi_config.sta.password);
    drv_com0_printf("***system wifi has store!***\r\n");
}
void smart_config_fun_state(SMART_WIFI_STATE smart_wifi_state)
{
    switch (smart_wifi_state)
    {
        case SMART_CONFIG_CONNECT_START:
            drv_com0_printf("smart begin connect \r\n");
            break;
        case SMART_CONFIG_CONNECT_ERROR:
            // 智能配置如果连接失败,会重新开始
            drv_com0_printf("smart connect error\r\n");
            break;
        case SMART_CONFIG_CONNECT_OK:
            drv_com0_printf("smart connect ok\r\n");
            break;
        default:
            break;
    }
}

// 配置wfii
void app_read_flash_wifi()
{
    // 如果读取到了wifi信息,就连接wfii 

    static const char *NVS_CUSTOMER = "wifi_info";
    static const char *DATA = "wifi_1";
    wifi_config_t wifi_config;
    int wifi_config_len = sizeof(wifi_config_t);
    // 读取flash中的wifi信息成功
    if (dev_nvs_read_blob(NVS_CUSTOMER, DATA, (wifi_config_t *)&wifi_config, &wifi_config_len) == 0)
    {
        // 直接连接
        dev_wifi_connect(wifi_config.sta.ssid,wifi_config.sta.password,wifi_scan);
        drv_com0_printf("****system read wifi ok !***\r\n");
    }else{
        // 开启smartconfig
        dev_smart_config_init(smart_config_fun_getWifiInfo,smart_config_fun_state);
        drv_com0_printf("****system read wifi erro,begin smartconfig!***\r\n");
    }
}


void app_main()
{   
    // 初始化flash
    dev_nvs_init();
    xTaskCreate(echo_task, "uart_echo_task", 1024, NULL, 10, NULL);
    vTaskDelay(1000 /portTICK_RATE_MS);
    app_read_flash_wifi();
    
}