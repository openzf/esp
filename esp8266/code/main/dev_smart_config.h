#ifndef __DEV_SMART_CONFIG_H__
#define __DEV_SMART_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"

typedef enum
{
    SMART_CONFIG_CONNECT_START = 0x1,
    SMART_CONFIG_CONNECT_ERROR = 0x2,
    SMART_CONFIG_CONNECT_OK = 3
}SMART_WIFI_STATE;

void dev_smart_config_init(void (*arg_msg_callback)(wifi_config_t wifi_config), void (*arg_config_state_callback_fun)(SMART_WIFI_STATE smart_wifi_state));
void dev_smart_config_reconfig();

#ifdef __cplusplus
}
#endif
#endif