#include "socket_app_demo.h"
#include "drv_com.h"
#include "dev_tcp_socket_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "cJSON.h"

#include "esp_system.h"
#include "driver/gpio.h"


#include <ctype.h>
#include <errno.h>
#include <time.h>
#include "esp_partition.h"
#include "esp_wifi.h"

static const char *TAG = "example";


/**
* @ Function Name :  fun_tcp_socket_state
* @ Author        : ygl
* @ Brief         : tcp socket状态回调函数
* @ Date          : 2018.11.18
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
static void fun_tcp_socket_state(TCP_SOCKET_STATE tcp_socket_state)
{
    switch (tcp_socket_state)
    {
        case TCP_SOCKET_CREATE_ERROR:
             ESP_LOGI(TAG, "TCP_SOCKET_CREATE_ERROR\r\n");
            break;
        case TCP_SOCKET_CREATE_OK:
             ESP_LOGI(TAG, "TCP_SOCKET_CREATE_OK\r\n");
            break;
        case TCP_SOCKET_CONNECT_ERROR:
             ESP_LOGI(TAG, "TCP_SOCKET_CONNECT_ERROR\r\n");
            break;
        case TCP_SOCKET_CONNECT_OK:
             ESP_LOGI(TAG, "TCP_SOCKET_CONNECT_OK\r\n");
            break;
        case TCP_SOCKET_SENT_ERROR:
             ESP_LOGI(TAG, "TCP_SOCKET_SENT_ERROR\r\n");
            break;
        case TCP_SOCKET_SENT_OK:
             ESP_LOGI(TAG, "TCP_SOCKET_SENT_OK\r\n");
            break;
        case TCP_SOCKET_SOCKET_ERROR:
             ESP_LOGI(TAG, "TCP_SOCKET_SOCKET_ERROR\r\n");
            break;
        case TCP_SOCKET_SOCKET_CLOSE:
             ESP_LOGI(TAG, "TCP_SOCKET_SOCKET_CLOSE\r\n");
            break;
        default:
            break;
    }
}

/**
* @ Function Name :  tcp_socket_rec_callBack
* @ Author        : ygl
* @ Brief         : tcp socket消息回调函数
* @ Date          : 2018.11.18
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
static void tcp_socket_rec_callBack(char *buffer, int len)
{
    cJSON *pJsonRoot = cJSON_Parse(buffer);
    //如果是否json格式数据
    if (pJsonRoot !=NULL) {
        ESP_LOGI(TAG, "is json");
        cJSON *pMacAdress = cJSON_GetObjectItem(pJsonRoot, "type");
        //判断mac字段是否json格式
        if (pMacAdress) {
            //判断mac字段是否string类型
            if (cJSON_IsString(pMacAdress))
                ESP_LOGI(TAG, "json type is %s",pMacAdress->valuestring); 
        } else
                ESP_LOGI(TAG, "json par error");

    }
         drv_com0_printf("msg:%s\r\n",buffer);
}

/**
* @ Function Name :  tcp_socket_task
* @ Author        : ygl
* @ Brief         : tcp socket任务
* @ Date          : 2018.11.18
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
 static void tcp_socket_task(void *pvParameters)
 {
     char *m_ip_addr = "192.168.1.100";
     int port = 3333;
     Dev_tcp_socket_info_TypeDef m_socket_info;

     dev_regeist_tcp_socket_callBack(fun_tcp_socket_state);
     dev_regeist_tcp_socket_rec_callBack(tcp_socket_rec_callBack);
     while (1)
     {
         m_socket_info = dev_tcp_socket_create(m_ip_addr,port);
         if (m_socket_info.socket < 0)
         {
             drv_com0_printf("tcp_socket_error\r\n");
             break;
         }
         drv_com0_printf("tcp_socket ok\r\n");

         while (1)
         {
             /* code */
             int error = dev_tcp_socket_sent(m_socket_info, "hello kirito socket");
             vTaskDelay(1000 / portTICK_PERIOD_MS);
         }
    }
    drv_com0_printf("deletae this task\r\n");
    vTaskDelete(NULL);
 }


 
/**
* @ Function Name :  drv_com_m_handle
* @ Author        : ygl
* @ Brief         : 串口接收回调函数
* @ Date          : 2018.11.18
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
static void drv_com_m_handle(unsigned char data){
	drv_com0_printf("%c",data);
}

/**
* @ Function Name :  socket_app_uart_init
* @ Author        : ygl
* @ Brief         : 串口初始化
* @ Date          : 2018.11.18
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
void socket_app_uart_init()
{
    drv_com_init(COM_0, 115200, drv_com_m_handle);
}

/**
* @ Function Name :  socekt_app_init
* @ Author        : ygl
* @ Brief         : app初始化
* @ Date          : 2018.11.18
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
void socekt_app_init()
{  
     xTaskCreate(tcp_socket_task, "uart_echo_task", 4096, NULL, 10, NULL);
}
