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

// ------------------ http任务 -------
// 设置的回调函数
void http_rec_callback(char data)
{
//  drv_com0_printf("%c",data);
    paser_msg(data);
}
// http头回调
void mm_head(HTTP_RESPONSE_TYPEDEF http_response)
{
    printf("system call:%s\r\n",http_response.http_response_line.msg_responseLine_protocol);
    printf("system call head num:%d\r\n",http_response.http_response_head.head_num);

    for (int i = 0; i < http_response.http_response_head.head_num;i++)
    {
        printf("system call %d:head:%s--val:%s\r\n",i,http_response.http_response_head.head_list[i].key,http_response.http_response_head.head_list[i].val);
    }
}
// http消息回调
void mm_msg(char data)
{
  drv_com0_printf("%c",data);
}
// http状态回调函数
void http_sentState_callback(char data)
{
    switch (data)
    {
        case ERROR_CODE_SOCKET:
            /* code */
            break;
         case ERROR_CODE_CONNECT:
            /* code */
            break;
        case ERROR_CODE_SENT:
            /* code */
            break;
        case ERROR_CODE_REC:
            /* code */
            break;
        case ERROR_DNS:
            /* code */
            break;
        case OK_RECCODE:
          parser_init();
            /* code */
            break;
    }
}

// http任务
 void http_task(void *pvParameters)
{

char *get_url = "http://192.168.1.101/home?username=kirito&password=hello";

    // 注册接收回调函数和发送状态
    parser_regist_printCallback(drv_com0_printf);
    parser_regist_msg_callback(mm_msg);
    parser_regist_http_head_callback(mm_head);
    http_init(http_rec_callback, http_sentState_callback);

    while (1)
    {

       http_send_GET_quest(get_url);
       
        for(int countdown = 2; countdown >= 0; countdown--) {
                vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

    }
}



void main(int argc, char const *argv[])
{
	
	return 0;
}