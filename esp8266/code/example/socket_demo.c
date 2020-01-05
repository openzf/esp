
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "cJSON.h"


// ------------------socket任务 -------
// 状态回调
void fun_tcp_socket_state(TCP_SOCKET_STATE tcp_socket_state)
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
// 回调函数
void tcp_socket_rec_callBack(char *buffer, int len)
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
// socket任务
 void tcp_socket_task(void *pvParameters)
 {

    Dev_tcp_socket_info_TypeDef m_socket_info;

    dev_regeist_tcp_socket_callBack(fun_tcp_socket_state);
    dev_regeist_tcp_socket_rec_callBack(tcp_socket_rec_callBack);
    while(1){
      m_socket_info = dev_tcp_socket_create();
    if (m_socket_info.socket < 0)
    {
        drv_com0_printf("tcp_socket_error\r\n");
        break;
    }
    drv_com0_printf("tcp_socket ok\r\n");
    
        while(1){
            /* code */
            int error = dev_tcp_socket_sent(m_socket_info, "hello kirito socket");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    
    }
    drv_com0_printf("deletae this task\r\n");
    vTaskDelete(NULL);
 }