#include "system_module.h"

#include "esp_partition.h"
#include "esp_wifi.h"
#include "dev_wifi.h"
#include "dev_nvs.h"
#include "dev_smart_config.h"
#include "drv_com.h"
#include "drv_gpio.h"
#include "driver/gpio.h"
#include "esp_log.h"
static const char *TAG = "system_module";

static void (*system_ready_fun)(void);



// ------------------wifi 任务 -------
/**
* @ Function Name : wifi_scan
* @ Author        : ygl
* @ Brief         : wifi扫描状态
* @ Date          : 2018.11.18
* @ Input         : WIFI_STATE wifi_state :状态
* @ Output		  : null
* @ Modify        : ...
**/
static void wifi_scan(WIFI_STATE wifi_state)
{
    if(wifi_state == WIFI_CONNECTED){
      drv_com0_printf("wifi connected\r\n");
       drv_com0_printf("{\"wifi\":\"connected\"}");
       vTaskDelay(200 /portTICK_RATE_MS);
       drv_com0_printf("{\"wifi\":\"connected\"}");
       system_ready_fun();
    }
    if (wifi_state == WIFI_DISCONNECTED){
         drv_com0_printf("wifi disconnected");
         
    }
}


/**
* @ Function Name : smart_config_fun_getWifiInfo
* @ Author        : ygl
* @ Brief         : 获得wifi信息回调函数
* @ Date          : 2018.11.18
* @ Input         : wifi_config_t wifi_config :wifi配置消息
* @ Output		  : null
* @ Modify        : ...
**/
static void smart_config_fun_getWifiInfo(wifi_config_t wifi_config)
{
    static const char *NVS_CUSTOMER = "wifi_info";
    static const char *DATA = "wifi_1";
    int wifi_config_len = sizeof(wifi_config_t);

    // 存储在flash中
    dev_nvs_write_blob(NVS_CUSTOMER, DATA, ( wifi_config_t *)&wifi_config, wifi_config_len);
   drv_com0_printf("MAIN_SSID:%s",wifi_config.sta.ssid);
   drv_com0_printf("MAIN_PASSWORD:%s",wifi_config.sta.password);
   drv_com0_printf("***system wifi has store!***\r\n");
}


/**
* @ Function Name : smart_config_fun_state
* @ Author        : ygl
* @ Brief         : wifi状态回调函数
* @ Date          : 2018.11.18
* @ Input         : SMART_WIFI_STATE smart_wifi_state :状态
* @ Output		  : null
* @ Modify        : ...
**/
static void smart_config_fun_state(SMART_WIFI_STATE smart_wifi_state)
{
    switch (smart_wifi_state)
    {
        case SMART_CONFIG_CONNECT_START:
       drv_com0_printf("smart begin connect \r\n");
        drv_com0_printf("{\"smartconfig\":\"ing\"}");
        vTaskDelay(200 /portTICK_RATE_MS);
        drv_com0_printf("{\"smartconfig\":\"ing\"}");
            break;
        case SMART_CONFIG_CONNECT_ERROR:
            // 智能配置如果连接失败,会重新开始
            drv_com0_printf("smart connect error\r\n");
            drv_com0_printf("{\"smartconfig\":\"error\"}");
            vTaskDelay(200 /portTICK_RATE_MS);
            drv_com0_printf("{\"smartconfig\":\"error\"}");
            break;
        case SMART_CONFIG_CONNECT_OK:
            vTaskDelay(1500 /portTICK_RATE_MS);
            drv_com0_printf("smart connect ok\r\n");
            drv_com0_printf("{\"smartconfig\":\"suc\"}");
            vTaskDelay(200 /portTICK_RATE_MS);
            drv_com0_printf("{\"smartconfig\":\"suc\"}");

            vTaskDelay(1500 /portTICK_RATE_MS);
            drv_com0_printf("wifi connected\r\n");
            drv_com0_printf("{\"wifi\":\"connected\"}");
            vTaskDelay(200 /portTICK_RATE_MS);
            drv_com0_printf("{\"wifi\":\"connected\"}");
            break;
        default:
            break;
    }
}

/**
* @ Function Name : system_read_flash_wifi
* @ Author        : ygl
* @ Brief         : 配置wfii
* @ Date          : 2018.11.18
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
static void system_read_flash_wifi()
{
    // 如果读取到了wifi信息,就连接wfii 

    static const char *NVS_CUSTOMER = "wifi_info";
    static const char *DATA = "wifi_1";
    wifi_config_t wifi_config;
    int wifi_config_len = sizeof(wifi_config_t);
    // 初始化flash
    dev_nvs_init();

    int state = 0;
    // 设置GPIO4为输入
    GPIO_SET_INPUT(4,PIN_PU);


    for (int i = 0; i < 5;i++){
        if(GPIO_Get_Level(4) == 0){
            state = 1;
            break;
        }
         vTaskDelay(10 /portTICK_RATE_MS);
    }
    if (state){
            GPIO_SET_OUTPUT(2);
            GPIO_Set_Level(2, 0);
            drv_com0_printf("****system rest!!! begin smartconfig!***\r\n");
            // 开启smartconfig
           
        drv_com0_printf("{\"smartconfig\":\"wait\"}");
       vTaskDelay(200 /portTICK_RATE_MS);
        drv_com0_printf("{\"smartconfig\":\"wait\"}");
           dev_smart_config_init(smart_config_fun_getWifiInfo, smart_config_fun_state);
    }else{
        // 读取flash中的wifi信息成功
        if (dev_nvs_read_blob(NVS_CUSTOMER, DATA, (wifi_config_t *)&wifi_config, &wifi_config_len) == 0)
        {
            // 直接连接
            drv_com0_printf("****system read wifi ok !***\r\n");
            dev_wifi_connect(wifi_config.sta.ssid, wifi_config.sta.password, wifi_scan);
           
        }
        else
        {
            GPIO_SET_OUTPUT(2);
            GPIO_Set_Level(2, 0);
            // 开启smartconfig
            drv_com0_printf("****system read wifi erro,begin smartconfig!***\r\n");
            dev_smart_config_init(smart_config_fun_getWifiInfo, smart_config_fun_state);
           
        }
    }
}


/**
* @ Function Name : system_module_init
* @ Author        : ygl
* @ Brief         : 系统模块初始化
* @ Date          : 2018.11.18
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
void system_module_init(void (*arg_system_ready_fun)(void))
{
    system_ready_fun = arg_system_ready_fun;
    system_read_flash_wifi();
}