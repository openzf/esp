[TOC]

## esp8266_easy_sdk说明

modify:kirito 2019-11-10

create:kirito 2019.5.16

### 前言

因为ESP8266 RTOS比较难开发, 所以我想做成一个能简单实用的库, 方便调用,主要再次封装了RTOS_SDK的库, 变得和arduino一样好实用

Kirito

----------2019.5.16

### 1 功能简介

1 htttp, 包括 content-length和chunk --- 见 http_app_demo

2 https, 采用的mbedtls  -- 见https_app_demo

3 tcp_socket

4 mqtt

5 http_ota

6  inside_flash , flash的操作, 简单Key_value存储

7 nvs存储系统

8 smartconfig, 智能配置模式, 必须采用esp_touch

9 sntp 在线获得时间

10 wifi, 简单的连接wfii

11 gpio

12 pwm

12 com

13 i2c

14 OTA

已知BUG: https内存可能会溢出

### 2 API指南

#### 1 GPIO

1 输出

```c
// 初始化
 GPIO_SET_OUTPUT(2);


// 输出为高
 GPIO_Set_Level(2, 1);
// 输出为低
GPIO_Set_Level(2, 0);
```

2 输入

```c
// 可以选择 上拉还是下拉输入
// 初始化 -- 上拉
 GPIO_SET_INPUT(2,PIN_PD);
// 初始化 -- 下拉
GPIO_SET_INPUT(2,PIN_PU);

// 读取电平
int level =GPIO_Get_Level(2);
```

#### 2 PWM

可以用的引脚:

```c

PWM_PIN_0 = 12,
PWM_PIN_1 = 13,
PWM_PIN_2 = 14,
PWM_PIN_3 = 15,
```

配置pwm输出, 占空比可变

```c
// 初始化
// 周期单位是1us ,占空比是0-100
drv_pwm_init(PWM_PIN_0,20,50);

// 调用
drv_pwm_set_duty(PWM_PIN_0, 2);
```

#### 3 com

有2个com口, com0 能输入和输出, com1只能接受

结构体:

```c
typedef enum
 {
    COM_0 = 0x0,
    COM_1 = 0x1,
}com_part_t;

typedef struct Drv_com_rec_callback_TypeDef_TAG
{
	com_part_t com_x;
	void (*drv_com_m_handle)(unsigned char data);
}Drv_com_rec_callback_TypeDef;
```

```c
 // 接收回调
void drv_com_x_handle_callback(unsigned char data)
 {
   
 }
// 初始化
drv_com_init(COM_0, 115200, drv_com_x_handle_callback);


char data[80] = "hello world";
// 写入数据
drv_com0_write_bytes(data, strlen(data));
// 可变参数写入
drv_com0_printf("hello %s",data);

```

#### 4 inside_flash

有2个分区, 1个分区存储分区表, 另一个分区存储数据. 就类似硬盘的 MBR

分布表, 用来标记数据的存储位置

```
typedef struct Dev_file_partition_table_TypeDef_TAG
{
    int select;
    int offset;
    File_storage_info_TypeDef file_info[FILE_MAX_STO_LEN]; 
    int file_now_index;
    int file_storage_select_addr;
    int file_storage_offset_addr;
    int len;
    int key;
}Dev_file_partition_table_TypeDef;
```

文件分区表: 用来标志每个文件的存储信息

```c
typedef struct File_storage_info_TypeDef_TAG
{
    char name[NAME_MAX_LEN];
    int select;
    int offset;
    FILE_STORAGE_TYPE type;
    int len;
}File_storage_info_TypeDef;
```

```c
 // 状态回调,写入错误, 就会调用 返回1
void fun_callback(int state)
 {
 }
// 初始化
dev_easySql_init(fun_callback);

// 数字
// 存储 -- 成功0 ,错误-1
if(dev_easySql_write_num("int_key", 2) == 0)
{
  
}
// 读取 -- 成功0 ,错误-1
int int_val;
if(dev_dev_easySql_read_num("int_key", &int_val) == 0) 
{
  
}

// 字符串类似
int dev_easySql_write_string(char *key, char *val);
int dev_easySql_read_string(char *key, char *val);

```

#### 5 NVS

注意:参数 int len是指, 将要存储数据的空间长度, 而不是读取值的长度

nameSpace是命名空间

```c
// 必须初始化
void dev_nvs_init();

int dev_nvs_write_i32(char *nameSpace, char *key, int data);
int dev_nvs_read_i32(char *nameSpace, char *key, int *out_data);
int dev_nvs_write_string(char *nameSpace, char *key, char *value);
int dev_nvs_read_string(char *nameSpace, char *key, char *out_data, int *len);
int dev_nvs_write_blob(char *nameSpace, char *key, void *value, int len);
int dev_nvs_read_blob(char *nameSpace,char *key, void *value,int *len);
```

#### 6 wifi

wifi_状态: 连接状态会通过回调函数调用

```
typedef enum
 {
    WIFI_CONNECTED = 0x1,
    WIFI_DISCONNECTED = 0x2,
}WIFI_STATE;
```

```c
// 回调函数
void dev_wifi_state_callback(WIFI_STATE wifi_state)
{
  
}
// 连接wifi
 dev_wifi_connect("kirito","passwd", dev_wifi_state_callback);

```

#### 7 smartconfig智能配置模式

结构体:

```c
typedef enum
{
    SMART_CONFIG_CONNECT_START = 0x1,
    SMART_CONFIG_CONNECT_ERROR = 0x2,
    SMART_CONFIG_CONNECT_OK = 3
}SMART_WIFI_STATE;
```

```c
// 得到wifi信息回调函数
static void smart_config_fun_getWifiInfo(wifi_config_t wifi_config)
{
   drv_com0_printf("MAIN_SSID:%s",wifi_config.sta.ssid);
   drv_com0_printf("MAIN_PASSWORD:%s",wifi_config.sta.password);
}
// 状态回调
static void smart_config_fun_state(SMART_WIFI_STATE smart_wifi_state)
{
}

// 智能配置模式开启
dev_smart_config_init(smart_config_fun_getWifiInfo,smart_config_fun_state);
// 重新开启, 一般如果连接失败, 要重新开启
dev_smart_config_reconfig();
```

#### 8 sntp 获得网络时间

初始化要等10秒左右

结构体:

```c
struct tm
{
    int tm_sec;   // seconds after the minute - [0, 60] including leap second
    int tm_min;   // minutes after the hour - [0, 59]
    int tm_hour;  // hours since midnight - [0, 23]
    int tm_mday;  // day of the month - [1, 31]
    int tm_mon;   // months since January - [0, 11]
    int tm_year;  // years since 1900
    int tm_wday;  // days since Sunday - [0, 6]
    int tm_yday;  // days since January 1 - [0, 365]
    int tm_isdst; // daylight savings time flag
};
```



```c
// 初始化
dev_sntp_init();
struct tm time_buffer;

dev_sntp_get_time(&time_buffer);

```

#### 9 tcp_socket客户端

结构体:

```c
// socket连接状态
typedef enum
 {
    TCP_SOCKET_CREATE_ERROR = 0x1,
    TCP_SOCKET_CREATE_OK = 0x2,
    TCP_SOCKET_CONNECT_ERROR = 0x3,
    TCP_SOCKET_CONNECT_OK = 0x4,
    TCP_SOCKET_SENT_ERROR = 0x5,
    TCP_SOCKET_SENT_OK = 0x6,
    TCP_SOCKET_SOCKET_ERROR = 0x7,
    TCP_SOCKET_SOCKET_CLOSE = 0x8,
}TCP_SOCKET_STATE;
```

```c
typedef struct Dev_tcp_socket_info_TypeDef_TAG
{
    int socket;
    int error;
}Dev_tcp_socket_info_TypeDef;

```

```c
static void fun_tcp_socket_state(TCP_SOCKET_STATE tcp_socket_state)
{
}
static void tcp_socket_rec_callBack(char *buffer, int len)
{
}

int main()
{
  
     char *m_ip_addr = "192.168.1.100";
     int port = 3333;
     Dev_tcp_socket_info_TypeDef m_socket_info;
	// 注册状态和接收回调
     dev_regeist_tcp_socket_callBack(fun_tcp_socket_state);
     dev_regeist_tcp_socket_rec_callBack(tcp_socket_rec_callBack);
     while (1)
     {
        // 创建socket
         m_socket_info = dev_tcp_socket_create(m_ip_addr,port);
         if (m_socket_info.socket < 0)
         {
             break;
         }
         while (1)
         {
             // 发送消息
             int error = dev_tcp_socket_sent(m_socket_info, "hello kirito socket");
             vTaskDelay(1000 / portTICK_PERIOD_MS);
         }
    }
}
```

#### 10 mqtt

结构体:

```c
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
```

```c
static void messageArrived(MessageData *data)
{
    char *mdata =  (char *)data->message->payload);
}

void app_task(void *pvParameters)
{	
  // 整理ID
     char clientID[32] = {0};
    sprintf(clientID, "%s_%u", CONFIG_MQTT_CLIENT_ID, esp_random());
	// 连接服务器
    if(dev_mqtt_connect(CONFIG_MQTT_BROKER, CONFIG_MQTT_PORT, clientID, CONFIG_MQTT_USERNAME, 	CONFIG_MQTT_PASSWORD) == 0){
      
    }
  // 订阅消息
    dev_mqtt_sub_topic(CONFIG_MQTT_SUB_TOPIC, Mqtt_QS_1, messageArrived);
 	// 推送消息
    dev_mqtt_push_topic("connect", Mqtt_QS_1, "ok");
    while(1)
    {
        
        
    }
}
```



#### 11 http

m_http_config.h中 有http相关定义



```c
static void arg_state_fun(char state)
{

}
static void arg_msg_fun(char *data, int len)
{
  
}
 void app_task(void *pvParameters)
{
    char *get_url = "http://www.weather.com.cn/data/cityinfo/101270107.html";
   
    char *post_url = "http://192.168.1.101/home";
    char *post_data = "username=zero&password=kotlin";
	
   // 初始化
    http_init();
    http_regist_state_callback(arg_state_fun);
    http_regist_msg_callback(arg_msg_fun);
 
    while (1)
    {
        http_send_GET_quest(get_url);
        http_send_POST_quest(post_url, CONTENT_TYPE_A_FORM, post_data);
    }
 }
```

#### 12 https

```c
static void arg_state_fun(char state)
{

}
 static void arg_msg_fun(char *data, int len)
 {
 }
static void https_demo_task(void *pvParameters)
{
    dev_https_init();
    dev_https_regist_state_callback(arg_state_fun);
    dev_https_regist_msg_callback(arg_msg_fun);
    while(1){
         https_system_get_request(WEB_URL);
    }
}
```

#### 13 OTA

会发出http请求,服务器响应最新的固件, http下载固件, 重启更新

只有服务器的固件, 大于本地的固件, 才会更新

服务器json格式:

```json
{
        'lastVersion': 21,
        "file_name":"kesp_V21.bin",
        "download_path":"/download/kesp_V21.bin"
  }
```

本地只要修改 ota.c中的 服务器地址, 服务器请求,当前版本就可以了

```c
const int SDK_VERSION = 27;
// http请求最新版本地址
char *G_ota_get_url = "http://192.168.1.100/get_update";
// 下载服务器地址
#define EXAMPLE_SERVER_IP   "192.168.1.100"
#define EXAMPLE_SERVER_PORT "80"
```

调用ota初始化, 如果有升级就去升级, 没有升级就调用arg_fun

```c
void ota_init(void (*arg_fun)(void));
```



在user_main.c中, 

如果启用OTA模式, 就把注释去掉

```c
// 如果采用OTA模式
// #define OTA

// 系统准备就绪,查看是否有可升级的
void app_system_ready()
{   
    #ifdef OTA
    ota_init(app_normal_mode_run);
    #else
     app_normal_mode_run();
     #endif
}
```

