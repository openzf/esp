#ifndef __DRV_WIFI_H__
#define __DRV_WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define TEMP_WIFI_SSID "ZERO"
#define TEMP_WIFI_PASS "88888888"

typedef enum
 {
    WIFI_CONNECTED = 0x1,
    WIFI_DISCONNECTED = 0x2,
}WIFI_STATE;

void dev_wifi_connect(uint8_t *wifi_ssid, uint8_t *wifi_pass, void (*dev_wifi_state_arg)(WIFI_STATE wifi_state));

#ifdef __cplusplus
}
#endif
#endif