#include "dev_smart_config.h"

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"





static wifi_config_t G_wifi_config;
static void (*wifiInfo_callback_fun)(wifi_config_t wifi_config);
static void (*config_state_callback_fun)(SMART_WIFI_STATE smart_wifi_state);
void smartconfig_example_task(void * parm);


static void dev_smart_config_regist_getWifiInfo_callback(void (*arg_msg_callback)(wifi_config_t wifi_config));
// 注册配置状态的wfii回调
static void dev_smart_config_regist_state_callback(void (*arg_config_state_callback_fun)(SMART_WIFI_STATE smart_wifi_state));


static EventGroupHandle_t wifi_event_group;
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const char *TAG = "sc";

/**
* @ Function Name :  event_handler
* @ Author        : ygl
* @ Brief         : 事件处理
* @ Date          : 2019.03.05
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        config_state_callback_fun(SMART_CONFIG_CONNECT_OK);
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        // 先断开连接
        esp_wifi_disconnect();
        // 重新配置
        dev_smart_config_reconfig();
        config_state_callback_fun(SMART_CONFIG_CONNECT_ERROR);
        break;
    default:
        break;
    }
    return ESP_OK;
}

/**
* @ Function Name : initialise_wifi
* @ Author        : ygl
* @ Brief         : 初始化wifi
* @ Date          : 2019.03.05
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

/**
* @ Function Name : sc_callback
* @ Author        : ygl
* @ Brief         : 智能配置模式, 连接回调
* @ Date          : 2019.03.05
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
static void sc_callback(smartconfig_status_t status, void *pdata)
{
    switch (status) {
        case SC_STATUS_WAIT:
            break;
        case SC_STATUS_FIND_CHANNEL:
            // 准备就绪
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            // 当手机发送广播的时候,会受到两次这个消息
            break;
        case SC_STATUS_LINK:
            config_state_callback_fun(SMART_CONFIG_CONNECT_START);
            // 然后开始连接
            // 存放到全局
            G_wifi_config = *((wifi_config_t*)pdata);
            wifi_config_t *wifi_config;
            wifi_config = pdata;
            // 先断开连接, 然后连接wifi
            ESP_ERROR_CHECK( esp_wifi_disconnect() );
            ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config) );
            ESP_ERROR_CHECK( esp_wifi_connect() );
            break;
        case SC_STATUS_LINK_OVER:
            wifiInfo_callback_fun(G_wifi_config);
            // 连接成功
            xEventGroupSetBits(wifi_event_group, ESPTOUCH_DONE_BIT);
            break;
        default:
            break;
    }
}


/**
* @ Function Name : smartconfig_example_task
* @ Author        : ygl
* @ Brief         : 智能配置模式任务
* @ Date          : 2019.03.05
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
void smartconfig_example_task(void * parm)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );
    while (1) {
        uxBits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY); 
        if(uxBits & CONNECTED_BIT) {
            // 连接成功的时候
        }
        if(uxBits & ESPTOUCH_DONE_BIT) {
            // 最后连接成功,会断开
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}



/**
* @ Function Name : dev_smart_config_regist_getWifiInfo_callback
* @ Author        : ygl
* @ Brief         : 注册wifiSID密码信息回调
* @ Date          : 2019.03.05
* @ Input         : void (*arg_msg_callback)(wifi_config_t wifi_config) :回调函数指针
* @ Output		  : null
* @ Modify        : ...
**/
static void dev_smart_config_regist_getWifiInfo_callback(void (*arg_msg_callback)(wifi_config_t wifi_config))
{
    wifiInfo_callback_fun = arg_msg_callback;
}

/**
* @ Function Name : dev_smart_config_regist_state_callback
* @ Author        : ygl
* @ Brief         : 注册配置状态的wfii回调
* @ Date          : 2019.03.05
* @ Input         : void (*arg_msg_callback)(wifi_config_t wifi_config) :回调函数指针
* @ Output		  : null
* @ Modify        : ...
**/
static void dev_smart_config_regist_state_callback(void (*arg_config_state_callback_fun)(SMART_WIFI_STATE smart_wifi_state))
{
    config_state_callback_fun = arg_config_state_callback_fun;
}


/// --- 对外接口

/**
* @ Function Name :  dev_smart_config_reconfig
* @ Author        : ygl
* @ Brief         : 智能配置模式, 重新配置
* @ Date          : 2019.03.05
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
void dev_smart_config_reconfig()
{
    esp_smartconfig_stop();
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );
}

/**
* @ Function Name :  dev_smart_config_init
* @ Author        : ygl
* @ Brief         : 开始智能配置模式
* @ Date          : 2019.03.05
* @ Input         : void (*arg_msg_callback)(wifi_config_t wifi_config) : wifi连接成功之后回调
                    void (*arg_config_state_callback_fun)(SMART_WIFI_STATE smart_wifi_state) : 连接状态回调
* @ Output		  : null
* @ Modify        : ...
**/
void dev_smart_config_init(void (*arg_msg_callback)(wifi_config_t wifi_config),void (*arg_config_state_callback_fun)(SMART_WIFI_STATE smart_wifi_state))
{
    dev_smart_config_regist_getWifiInfo_callback(arg_msg_callback);
    dev_smart_config_regist_state_callback(arg_config_state_callback_fun);
    initialise_wifi();
}