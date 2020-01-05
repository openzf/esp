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


#define MQTT_CLIENT_THREAD_NAME         "mqtt_client_thread"
#define MQTT_CLIENT_THREAD_STACK_WORDS  4096
#define MQTT_CLIENT_THREAD_PRIO         8
static const char *TAG = "example";



// mqtt system_config
#define CONFIG_MQTT_PAYLOAD_BUFFER 1460
#define CONFIG_DEFAULT_MQTT_VERSION 3
#define CONFIG_MQTT_KEEP_ALIVE 30
#define CONFIG_DEFAULT_MQTT_SESSION 1


Mqtt_info_TypeDef G_Mqtt_info;
char *payload = NULL;



/**
* @ Function Name : dev_mqtt_sub_topic
* @ Author        : ygl
* @ Brief         : mqtt 订阅主题
* @ Date          : 2018.11.18
* @ Input         : char *sub_topic :主题名字
                    Mqtt_QS mqtt_qs: 消息质量
                    void (*arg_messageArrived)(MessageData *data) :接收消息函数指针
* @ Output		  : int :  失败返回-1 正常为0
* @ Modify        : ...
**/
int dev_mqtt_sub_topic(char *sub_topic, Mqtt_QS mqtt_qs, void (*arg_messageArrived)(MessageData *data))
{
    int rc = 0;
    // 订阅主题,和回调函数
    if ((rc = MQTTSubscribe(&G_Mqtt_info.client, sub_topic, mqtt_qs, arg_messageArrived)) != 0) {
       drv_com0_printf( "sub error");
        return -1;
    }
    return 0;
}

/**
* @ Function Name : dev_mqtt_push_topic
* @ Author        : ygl
* @ Brief         : mqtt 发布主题
* @ Date          : 2018.11.18
* @ Input         : char *sub_topic :主题名字
                    Mqtt_QS mqtt_qs: 消息质量
                     char *data :发送的数据
* @ Output		  : int :  失败返回-1 正常为0
* @ Modify        : ...
**/
int dev_mqtt_push_topic(char *push_topic, Mqtt_QS mqtt_qs, char *data)
{
    int rc = 0;
    MQTTMessage message;
    message.qos = mqtt_qs;
    message.retained = 0;
    message.payload = payload;
    sprintf(payload, "%s",data);
    message.payloadlen = strlen(payload);

    // 推送
    if ((rc = MQTTPublish(&G_Mqtt_info.client,push_topic, &message)) != 0) {
       drv_com0_printf( "push error");
        return -1;
    }    
    return 0;
}

/**
* @ Function Name : dev_mqtt_disconnect
* @ Author        : ygl
* @ Brief         : mqtt 断开连接
* @ Date          : 2018.11.18
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
void dev_mqtt_disconnect()
{
    G_Mqtt_info.network.disconnect(&G_Mqtt_info.network);
}

/**
* @ Function Name : dev_mqtt_connect
* @ Author        : ygl
* @ Brief         : mqtt 连接服务器
* @ Date          : 2018.11.18
* @ Input         : char *mqtt_server :服务器地址
                    char *client_id :客户端ID
                    char *usr_name :名字
                    char *usr_passwd :密码

* @ Output		  : int :  失败返回-1 正常为0
* @ Modify        : ...
**/
int  dev_mqtt_connect(char *mqtt_server,int port,char *client_id,char *usr_name,char *usr_passwd)
{
    int rc = 0;
 
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    NetworkInit(&G_Mqtt_info.network);
    if (MQTTClientInit(&G_Mqtt_info.client, &G_Mqtt_info.network, 0, NULL, 0, NULL, 0) == false) {
        drv_com0_printf("mqtt init err");
        return -1;
    }

   // 缓冲区大小
    payload = malloc(CONFIG_MQTT_PAYLOAD_BUFFER);

    if (!payload) {
       drv_com0_printf("mqtt malloc err");
        return -1;
    } else {
        memset(payload, 0x0, CONFIG_MQTT_PAYLOAD_BUFFER);
    }

    // IP和端口
    if ((rc = NetworkConnect(&G_Mqtt_info.network, mqtt_server, port)) != 0) {
       drv_com0_printf( "Return code from network connect is %d", rc);    
         return -1;
    }
  

    // 版本
    connectData.MQTTVersion = CONFIG_DEFAULT_MQTT_VERSION;
    // ID
    connectData.clientID.cstring = client_id;
    // 保持连接30S
    connectData.keepAliveInterval = CONFIG_MQTT_KEEP_ALIVE;
    // 用户名字和密码
    connectData.username.cstring = usr_name;
    connectData.password.cstring = usr_passwd;
    // 绘画
    connectData.cleansession = CONFIG_DEFAULT_MQTT_SESSION;
    // 连接mqtt
    if ((rc = MQTTConnect(&G_Mqtt_info.client, &connectData)) != 0) {
        drv_com0_printf( "Return code from MQTT connect is %d", rc);
        G_Mqtt_info.network.disconnect(&G_Mqtt_info.network);
         return -1;
    }

   #if defined(MQTT_TASK)
            // 如果配置了task
            if ((rc = MQTTStartTask(&G_Mqtt_info.client)) != pdPASS) {
                drv_com0_printf( "Return code from start tasks is %d", rc);
                 return -1;
            } else {
               
            }
    #endif
    drv_com0_printf( "MQTT Connected");
    return 0;
}

