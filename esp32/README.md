[TOC]

## esp32_easy_sdk说明

>modify:kirito 2019-11-10
>
>create:kirito 2019.5.16

### 前言

因为ESP32 RTOS比较难开发, 所以我想做成一个能简单实用的库, 方便调用,主要再次封装了RTOS_SDK的库, 变得和arduino一样好实用

Kirito

----------2019.8.16

### 1 功能简介

1 htttp, 包括 content-length和chunk --- 见 http_app_demo

2 https, 采用的mbedtls  -- 见https_app_demo

3 nvs存储系统

4 smartconfig, 智能配置模式, 必须采用esp_touch

5 sntp 在线获得时间

6 wifi, 简单的连接wfii

7 gpio

8 pwm

9 com



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

#### 4 NVS

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

#### 5 wifi

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

#### 6 smartconfig智能配置模式

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

#### 7 sntp 获得网络时间

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



#### 8 http

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

#### 9 https

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

