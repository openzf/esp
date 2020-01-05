#ifndef __DEV_MQTT_H__
#define  __DEV_MQTT_H__




#ifdef __cplusplus
extern "C" {
#endif



#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "MQTTClient.h"

typedef enum
 {
   Mqtt_QS_0 = 0,
   Mqtt_QS_1 = 1,
   Mqtt_QS_2= 2,
}Mqtt_QS;

typedef struct Mqtt_info_TypeDef
{
    int connect_state;
    MQTTClient client;
    Network network;
}Mqtt_info_TypeDef;

void dev_mqtt_create();
void dev_mqtt_disconnect();

int dev_mqtt_connect(char *mqtt_server, int port, char *client_id, char *usr_name, char *usr_passwd);
int dev_mqtt_sub_topic(char *sub_topic, Mqtt_QS mqtt_qs, void (*arg_messageArrived)(MessageData *data));
int dev_mqtt_push_topic(char *push_topic, Mqtt_QS mqtt_qs, char *data);

#ifdef __cplusplus
}
#endif
#endif