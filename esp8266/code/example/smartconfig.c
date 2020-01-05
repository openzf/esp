#include <ctype.h>
#include <errno.h>
#include <time.h>
#include "esp_partition.h"
#include "esp_wifi.h"
#include "../lib/dev_smart_config.h"



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

int main(int argc, char const *argv[])
{
	        // 开启smartconfig
    dev_smart_config_init(smart_config_fun_getWifiInfo,smart_config_fun_state);
	return 0;
}