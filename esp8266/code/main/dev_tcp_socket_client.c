#include "dev_tcp_socket_client.h"

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>



#define CONFIG_EXAMPLE_IPV4
#ifdef CONFIG_EXAMPLE_IPV4
#else
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
#endif


const int IPV4_GOTIP_BIT = BIT0;
#ifdef CONFIG_EXAMPLE_IPV6
const int IPV6_GOTIP_BIT = BIT1;
#endif

static const char *TAG = "example";
static const char *payload = "Message from ESP32 ";

// 全局socket信息
static Dev_tcp_socket_info_TypeDef m_tcp_socket_info;


static void dev_tcp_socket_rec_task(void *pvParameters);
// socket状态回调函数
static void (*dev_tcp_socket_state)(TCP_SOCKET_STATE tcp_socket_state);
// 接收回调函数
static void (*dev_tcp_socket_rec_callBack)(char *buffer,int len);



/**
* @ Function Name : dev_tcp_socket_rec_task
* @ Author        : ygl
* @ Brief         : 接收socket任务
* @ Date          : 2019.03.05
* @ Input         : void *pvParameters 传入的空指针
* @ Output		  : null
* @ Modify        : ...
**/
static void dev_tcp_socket_rec_task(void *pvParameters)
{
    char rx_buffer[128];
    // 取出参数
    Dev_tcp_socket_info_TypeDef *recvarg_tcp_socket_info;
    recvarg_tcp_socket_info =(Dev_tcp_socket_info_TypeDef*)pvParameters;

    while(1){
        // 接收消息
        int len = recv(recvarg_tcp_socket_info->socket, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            break;
        }
        else {
            rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
            dev_tcp_socket_rec_callBack(rx_buffer, len);
          
        }
    }
    // 关闭suocket
    dev_tcp_socket_state(TCP_SOCKET_SOCKET_CLOSE);
    dev_tcp_socket_close(*recvarg_tcp_socket_info);
    vTaskDelete(NULL);
    
}


/**
* @ Function Name : dev_regeist_tcp_socket_callBack
* @ Author        : ygl
* @ Brief         : 注册状态回调函数
* @ Date          : 2019.03.05
* @ Input         : void (*arg_dev_tcp_socket_state)(TCP_SOCKET_STATE tcp_socket_state) 状态回调函数指针
* @ Output		  : null
* @ Modify        : ...
**/
void dev_regeist_tcp_socket_callBack(void (*arg_dev_tcp_socket_state)(TCP_SOCKET_STATE tcp_socket_state)){
    dev_tcp_socket_state = arg_dev_tcp_socket_state;
}


/**
* @ Function Name : dev_regeist_tcp_socket_rec_callBack
* @ Author        : ygl
* @ Brief         : 注册接收回调函数
* @ Date          : 2019.03.05
* @ Input         : void (*arg_dev_tcp_socket_rec_callBack)(char *buffer,int len) 接收回调函数指针
* @ Output		  : null
* @ Modify        : ...
**/
void dev_regeist_tcp_socket_rec_callBack(void (*arg_dev_tcp_socket_rec_callBack)(char *buffer,int len))
{
    dev_tcp_socket_rec_callBack = arg_dev_tcp_socket_rec_callBack;
}


/**
* @ Function Name : dev_tcp_socket_create
* @ Author        : ygl
* @ Brief         : 创建socket
* @ Date          : 2019.03.05
* @ Input         : null
* @ Output		  : Dev_tcp_socket_info_TypeDef socket信息结构体,创建失败就为null
* @ Modify        : ...
**/
Dev_tcp_socket_info_TypeDef dev_tcp_socket_create(char *ip_addr,int port)
{
    char addr_str[128];
    int addr_family;
    int ip_protocol;

// 初始化IP
#ifdef CONFIG_EXAMPLE_IPV4
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = inet_addr(ip_addr);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(port);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);
#else // IPV6
        struct sockaddr_in6 destAddr;
        inet6_aton(ip_addr, &destAddr.sin6_addr);
        destAddr.sin6_family = AF_INET6;
        destAddr.sin6_port = htons(port);
        addr_family = AF_INET6;
        ip_protocol = IPPROTO_IPV6;
        inet6_ntoa_r(destAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
#endif

        while(1){

      // 创建socket
        int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0) { ESP_LOGE(TAG, "Unable to create socket: errno");
            dev_tcp_socket_state(TCP_SOCKET_CREATE_ERROR);
           break;
        }
        dev_tcp_socket_state(TCP_SOCKET_CREATE_OK);

        // 连接到socket服务器
        int err = connect(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err != 0) {
           
            dev_tcp_socket_state(TCP_SOCKET_CONNECT_ERROR);
           break;
        }
        dev_tcp_socket_state(TCP_SOCKET_CONNECT_OK);

        m_tcp_socket_info.socket = sock;
        m_tcp_socket_info.error = err;

    
        // 创建接收任务
        xTaskCreate(dev_tcp_socket_rec_task, "tcp_client_rec", 1024, &m_tcp_socket_info, 5, NULL);
        break;
        }
       
        return m_tcp_socket_info;
}


/**
* @ Function Name : dev_tcp_socket_sent
* @ Author        : ygl
* @ Brief         : 往socket发送信息
* @ Date          : 2019.03.05
* @ Input         :Dev_tcp_socket_info_TypeDef arg_tcp_socket_info socket信息结构体
*                   char *msg 将要发送的消息指针
* @ Output		  : int  发送成功为0 失败为-1
* @ Modify        : ...
**/
int dev_tcp_socket_sent(Dev_tcp_socket_info_TypeDef arg_tcp_socket_info,char *msg)
{
    // 如果创建socket的时候出错,那么就不发
    if (arg_tcp_socket_info.error != 0) {
            dev_tcp_socket_state(TCP_SOCKET_SOCKET_ERROR);
            return -1;
    }
    
    // 发送信息
    int err = send(arg_tcp_socket_info.socket, msg, strlen(msg), 0);
    if (err < 0) {
        dev_tcp_socket_state(TCP_SOCKET_SENT_ERROR);
         return -1;
    }
    dev_tcp_socket_state(TCP_SOCKET_SENT_OK);
    return 0;
}


/**
* @ Function Name :  dev_tcp_socket_close
* @ Author        : ygl
* @ Brief         : 关闭socket
* @ Date          : 2019.03.05
* @ Input         : Dev_tcp_socket_info_TypeDef arg_tcp_socket_info socket结构体
* @ Output		  : null
* @ Modify        : ...
**/
void dev_tcp_socket_close(Dev_tcp_socket_info_TypeDef arg_tcp_socket_info)
{
    shutdown(arg_tcp_socket_info.socket, 0);
    close(arg_tcp_socket_info.socket);
}