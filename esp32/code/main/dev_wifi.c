#include "dev_wifi.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <netdb.h>
#include <sys/socket.h>


static EventGroupHandle_t wifi_event_group;
static const char *TAG = "wifi_connect";
const int CONNECTED_BIT = BIT0;

// wifi状态回调函数
static void (*dev_wifi_state)(WIFI_STATE wifi_state);

/**
* @ Function Name : dev_regeist_wifiState_callBack
* @ Author        : ygl
* @ Brief         : 注册wifi状态回调函数
* @ Date          : 2019.03.05
* @ Input         :(void (*dev_wifi_state_arg)(WIFI_STATE wifi_state) wifi状态回调函数指针
* @ Output		  : null
* @ Modify        : ...
**/
static void dev_regeist_wifiState_callBack(void (*dev_wifi_state_arg)(WIFI_STATE wifi_state)){
    dev_wifi_state = dev_wifi_state_arg;
}

/**
* @ Function Name : event_handler
* @ Author        : ygl
* @ Brief         : system event handler
* @ Date          : 2019.03.05
* @ Input         :
* @ Output		  : null
* @ Modify        : ...
**/
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        dev_wifi_state(WIFI_CONNECTED);
       
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
         dev_wifi_state(WIFI_DISCONNECTED);
         //esp_wifi_connect();
         //xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

/**
* @ Function Name : dev_wifi_connect
* @ Author        : ygl
* @ Brief         : 连接wfii
* @ Date          : 2019.03.05
* @ Input         : uint8_t *wifi_ssid wifi名字指针
                    uint8_t *wifi_pass wifi密码指针
                    void (*dev_wifi_state_arg)(WIFI_STATE wifi_state)  连接状态函数指针
* @ Output		  : null
* @ Modify        : ...
**/
 void dev_wifi_connect(uint8_t *wifi_ssid,uint8_t *wifi_pass,void (*dev_wifi_state_arg)(WIFI_STATE wifi_state))
{
   tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    // 注册状态回调
    dev_regeist_wifiState_callBack(dev_wifi_state_arg);
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = TEMP_WIFI_SSID,
            .password = TEMP_WIFI_PASS,
        },
    };

    // 获取指针地址
    uint8_t *str_ssid = wifi_config.sta.ssid;
    uint8_t *str_pass = wifi_config.sta.password;
    // 复制ssid
    while(*wifi_ssid != '\0'){
		*(str_ssid++) = *(wifi_ssid++);
	}                                
    *str_ssid = '\0';
    // 复制passwd
    while(*wifi_pass != '\0'){
		*(str_pass++) = *(wifi_pass++);
	}
    *str_pass = '\0';
    ESP_LOGI(TAG, "ssi is%s...", wifi_config.sta.ssid);
    ESP_LOGI(TAG, "passwd is%s...", wifi_config.sta.password);
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

