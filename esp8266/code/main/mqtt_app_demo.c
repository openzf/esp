#include "mqtt_app_demo.h"
#include "dev_mqtt.h"
#include "drv_com.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "MQTTClient.h"

// json解析模块
#include "cJSON.h"


#define CONFIG_MQTT_BROKER "120.78.188.208"
#define CONFIG_MQTT_PORT 1883
#define CONFIG_MQTT_SUB_TOPIC "/jack/sub"
#define CONFIG_DEFAULT_MQTT_SUB_QOS 1

#define CONFIG_MQTT_PUB_TOPIC "/jack/pub"
#define CONFIG_DEFAULT_MQTT_PUB_QOS 1
#define CONFIG_MQTT_PUBLISH_INTERVAL 500

#define CONFIG_MQTT_CLIENT_ID "espressif_sample"
#define CONFIG_MQTT_USERNAME "espressif"
#define CONFIG_MQTT_PASSWORD "admin"

static const char *TAG = "example";


static void appt_mqttsent_to_uar_bridge(char *data);
static void app_uart_sent_to_mqtt_bridge(char *data);

static char str_temp[50];
static int now_state = 0;
static int str_index = 0;


/**
* @ Function Name : messageArrived
* @ Author        : ygl
* @ Brief         : mqtt消息来的回调函数
* @ Date          : 2018.11.18
* @ Input         : MessageData *data :消息
* @ Output		  : null
* @ Modify        : ...
**/
static void messageArrived(MessageData *data)
{
    
    cJSON *pJsonRoot = cJSON_Parse( (char *)data->message->payload);
    //如果是否json格式数据
    if (pJsonRoot !=NULL) {
        // 先解析cmdType
        cJSON *cmdtype = cJSON_GetObjectItem(pJsonRoot, "cmdtype");
        //判断mac字段是否json格式
        if (cmdtype) {
            //判断mac字段是否string类型
            if (cJSON_IsString(cmdtype))
               {

               }
        } else{
                ESP_LOGI(TAG, "cmdType error");
                return;
        }

        // 后解析data
        cJSON *data_val = cJSON_GetObjectItem(pJsonRoot, "data");
        //判断mac字段是否json格式
        if (data_val) {
            //判断mac字段是否string类型
            if (cJSON_IsString(data_val))
               {

               }
        } else{
                ESP_LOGI(TAG, "data_val error");
                return;
        }

        // 如果到这里就说明解析完成, 发送到stm32
        char stm32_str_data[40];
        sprintf(stm32_str_data, "{\"%s\":\"%s\"}", cmdtype->valuestring, data_val->valuestring);
        appt_mqttsent_to_uar_bridge(stm32_str_data);  
        }
    else{
         ESP_LOGI(TAG, "not json:%s",(char *)data->message->payload);
    }

}


/**
* @ Function Name : drv_com_m_handle
* @ Author        : ygl
* @ Brief         : 串口接收的回调函数
* @ Date          : 2018.11.18
* @ Input         : unsigned char data :串口数据
* @ Output		  : null
* @ Modify        : ...
**/
static void drv_com_m_handle(unsigned char data){

    // 如果收到这个就认为是开始
    if (data == '{'){
        now_state = 1;
       
    }
    // 如果收到这个就认为是结束
    else if(data == '}'){
        // 重新初始化, 准备下次接收

         str_temp[str_index++] = '}';
         str_temp[str_index++] = '\0';

         app_uart_sent_to_mqtt_bridge(str_temp);
         
         now_state = 0;
         str_index = 0;
    }
    else
    { // 这个就是普通的数据
        
        now_state = 2;
    }

    // 只有有效的时候才可以添加数据
    if(now_state != 0){
        if(str_index < 48){
           str_temp[str_index++] = data;
         }
    }
}

/**
* @ Function Name : app_uart_sent_to_mqtt_bridge
* @ Author        : ygl
* @ Brief         : 串口发向mqtt
* @ Date          : 2018.11.18
* @ Input         : char *data; 数据指针
* @ Output		  : null
* @ Modify        : ...
**/
static void app_uart_sent_to_mqtt_bridge(char *data)
{
     dev_mqtt_push_topic(CONFIG_MQTT_PUB_TOPIC, Mqtt_QS_1, data);
}

/**
* @ Function Name : appt_mqttsent_to_uar_bridge
* @ Author        : ygl
* @ Brief         : mqtt 发向串口
* @ Date          : 2018.11.18
* @ Input         : char *data; 数据指针
* @ Output		  : null
* @ Modify        : ...
**/
static void appt_mqttsent_to_uar_bridge(char *data)
{
    drv_com0_printf("%s\r\n",data);
}


/**
* @ Function Name : app_task
* @ Author        : ygl
* @ Brief         : mqtt任务
* @ Date          : 2018.11.18
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
static void app_task(void *pvParameters)
{
    char clientID[32] = {0};
    sprintf(clientID, "%s_%u", CONFIG_MQTT_CLIENT_ID, esp_random());

    dev_mqtt_connect(CONFIG_MQTT_BROKER, CONFIG_MQTT_PORT, clientID, CONFIG_MQTT_USERNAME, CONFIG_MQTT_PASSWORD);
    dev_mqtt_sub_topic(CONFIG_MQTT_SUB_TOPIC, Mqtt_QS_1, messageArrived);
 
    while(1)
    {
         dev_mqtt_push_topic("connect", Mqtt_QS_1, "ok");
        vTaskDelay(2000 /portTICK_RATE_MS);
    }
}

// -- 对外接口

/**
* @ Function Name :  app_init
* @ Author        : ygl
* @ Brief         : app初始化
* @ Date          : 2018.11.18
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
void app_init()
{  
     xTaskCreate(app_task, "uart_echo_task", 4096, NULL, 10, NULL);
}

/**
* @ Function Name : app_uart_init
* @ Author        : ygl
* @ Brief         : 系统串口初始化
* @ Date          : 2018.11.18
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
void app_uart_init()
{
    drv_com_init(COM_0, 115200, drv_com_m_handle);
}



