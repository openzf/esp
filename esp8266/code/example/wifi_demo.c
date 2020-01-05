
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

#include "../lib/dev_wifi.h"

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

int main(int argc, char const *argv[])
{
	// 通过wifi名字和密码连接
	 dev_wifi_connect(wifi_config.sta.ssid,wifi_config.sta.password,wifi_scan);
	return 0;
}