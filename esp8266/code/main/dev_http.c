#include "dev_http.h"
#include "dev_http_parser.h"
#include "drv_com.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "nvs_flash.h"

#include <netdb.h>
#include <sys/socket.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static const char *TAG = "http";
static char request_buffer[300]; // 最简单的数据200字节左右
static char recv_buf[400];

// 接收回调函数
static void (*m_http_rec_callback)(char data);
// 发送回调函数
static void (*m_http_sentState_callback)(char data);

// 对外接口
static void (*system_http_state_fun_callback)(char state);
static void (*system_http_msg_fun_calback)(char *data, int len);


/**
* @ Function Name :http_get_host
* @ Author        : ygl
* @ Brief         : 解析url里面的host
* @ Date          : 2019.03.05
* @ Input         : char *URL 将要解析的url指针
                    char *host 存放的host指针
* @ Output		  : null
* @ Modify        : ...
**/
static void http_get_host(char *URL,char *host) 
{
	char *PA;
	char *PB;

	if (!(*URL)){
		printf("\r\n ----- URL return -----  \r\n");
		return;
	}
	PA = URL;
	if (!strncmp(PA, "http://", strlen("http://"))) {
		PA = URL + strlen("http://");
	} else if (!strncmp(PA, "https://", strlen("https://"))) {
		PA = URL + strlen("https://");
	}

	PB = strchr(PA, '/');
	if (PB) {
		memcpy(host, PA, strlen(PA) - strlen(PB));
		*(host+strlen(PA) - strlen(PB))= '\0';
	} 
}

/**
* @ Function Name : http_send_packet
* @ Author        : ygl
* @ Brief         : 发送http数据包
* @ Date          : 2019.03.05
* @ Input         : struct addrinfo *res  地址结构体指针
                    char *url_quest   发送的数据包
* @ Output		  : null
* @ Modify        : ...
**/
static int  http_send_packet(struct addrinfo *res,char *url_quest)
{
    int  s,r;

    // 检测域名和类型,协议类型,返回操作数
    s = socket(res->ai_family, res->ai_socktype, 0);
    // 如果不支持就释放空间
    if(s < 0) {
        freeaddrinfo(res);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        m_http_sentState_callback(ERROR_CODE_SOCKET);
        return ERROR_CODE_SOCKET;
    }
    // 连接到服务器socket
    if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
        close(s);
        freeaddrinfo(res);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
         m_http_sentState_callback(ERROR_CODE_CONNECT);
        return ERROR_CODE_CONNECT;
    }
    freeaddrinfo(res);
    // 写入数据,post还是get
    int len = strlen(url_quest);
    if (write(s, url_quest,len) < 0) {
        close(s);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
         m_http_sentState_callback(ERROR_CODE_SENT);
        return  ERROR_CODE_SENT;
    }


    struct timeval receiving_timeout;
    receiving_timeout.tv_sec = 5;
    receiving_timeout.tv_usec = 0;
    // 接收返回的信息,超时时间为x
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, sizeof(receiving_timeout)) < 0)
     {
        close(s);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
         m_http_sentState_callback(ERROR_CODE_REC);
         return ERROR_CODE_REC;
    }

     m_http_sentState_callback(OK_RECCODE);
      
    // 读取消息
    /* Read HTTP response */
    do {
        memset(recv_buf,0, sizeof(recv_buf));
        r = read(s, recv_buf, sizeof(recv_buf)-1);
        for(int i = 0; i < r; i++) {         
             m_http_rec_callback(recv_buf[i]);
        }
    } while(r > 0);
    m_http_sentState_callback(REC_END);

    // 关闭socket
    close(s);
    return 0;
}

/**
* @ Function Name : http_get_packet
* @ Author        : ygl
* @ Brief         : http的get封包
* @ Date          : 2019.03.05
* @ Input         : char *packet_buffer  将要存放的包指针,
                    char *url  发送数据包的url地址
                    char *m_host 将要存放的host地址
* @ Output		  : null
* @ Modify        : ...
**/
static void http_get_packet(char *packet_buffer,char *url,char *m_host)
{
    // 获得host
    http_get_host(url,m_host);
    sprintf(packet_buffer, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: esp-idf/1.0 esp32\r\n\r\n", url, m_host);
}

/**
* @ Function Name : http_post_packet
* @ Author        : ygl
* @ Brief         : http的post封包
* @ Date          : 2019.03.05
* @ Input         : char *packet_buffer  将要存放的包指针,
                    char *url  发送数据包的url地址
                    char *contentType 发送数据包的类型指针
                    char *data 发送数据包的数据指针
                    char *m_host 将要存放的host地址
* @ Output		  : null
* @ Modify        : ...
**/
static void http_post_packet(char *packet_buffer,char *url,char *contentType,char *data,char *m_host)
{
    int m_len = strlen(data);
    // 获得host
    http_get_host(url,m_host);
    //  sprintf(packet_buffer, "POST %s HTTP/1.0\r\nHost: %s\r\nContent-Length: %d\r\nContent-Type: application/x-www-form-urlencoded\r\nUser-Agent: esp-idf/1.0 esp32\r\n\r\n%s", url, m_host, m_len,contentType,data);
    sprintf(packet_buffer, "POST %s HTTP/1.0\r\nHost: %s\r\nContent-Length: %d\r\nContent-Type: %s\r\nUser-Agent: esp-idf/1.0 esp32\r\n\r\n%s", url, m_host, m_len,contentType,data);
}


/**
* @ Function Name : http_DNS_getAddr
* @ Author        : ygl
* @ Brief         : 根据most,请求DNS服务器,获得真实IP
* @ Date          : 2019.03.05
* @ Input         : char *m_host host指针
                    struct addrinfo **res 得到数据后,存放的真实数据指针
* @ Output		  : int  获得DNS成功或者失败,  0 成功, -1 失败
* @ Modify        : ...
**/
static int http_DNS_getAddr(char *m_host,struct addrinfo **res){
    const struct addrinfo hints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM,
        };
     // DNS查找主机,获得IP地址,存储在res中
     int err = getaddrinfo(m_host, "80", &hints, res);
     if (err != 0 || res == NULL)
     {  
           m_http_sentState_callback(ERROR_DNS);
         return -1;
    }
    return 0;
}

/**
* @ Function Name : http_DNS_getAddr
* @ Author        : ygl
* @ Brief         : 根据most,请求DNS服务器,获得真实IP
* @ Date          : 2019.03.05
* @ Input         : char *m_host host指针
                    struct addrinfo **res 得到数据后,存放的真实数据指针
                    char **strIP    得到数据后,存放的真实IP指针
* @ Output		  : int  获得DNS成功或者失败,  0 成功, -1 失败
* @ Modify        : ...
**/
static int http_getRealIP(char *m_host,struct addrinfo **res,char **strIP)
{
    const struct addrinfo hints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM,
        };
    struct in_addr *addr;
   
    // DNS查找主机,获得IP地址,存储在res中
    int err = getaddrinfo(m_host, "80", &hints, res);


    if (err != 0 || res == NULL)
    {
        m_http_sentState_callback(ERROR_DNS);
        return -1;
    }
            // 获得真实IP地址
    addr = &((struct sockaddr_in *)(*res)->ai_addr)->sin_addr;
    *strIP = inet_ntoa(*addr);
    return 0;
}



// ------------------ http任务 -------

/**
* @ Function Name :http_rec_callback
* @ Author        : ygl
* @ Brief         : http消息回调
* @ Date          : 2019.03.05
* @ Input         : char data: 数据
*                   
* @ Output		  : null
* @ Modify        : ...
**/
static void http_rec_callback(char data)
{
    // 采用解析器解析
     paser_msg(data);
}

/**
* @ Function Name :http_sentState_callback
* @ Author        : ygl
* @ Brief         : http状态回调
* @ Date          : 2019.03.05
* @ Input         : char data: 状态
*                   
* @ Output		  : null
* @ Modify        : ...
**/
static void http_sentState_callback(char data)
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
        case REC_END:
            parser_msg_stop();
            break;
    }

    if(system_http_state_fun_callback != NULL){
        system_http_state_fun_callback(data);
    }
}



/**
* @ Function Name :parser_http_head
* @ Author        : ygl
* @ Brief         : 解析器head回调
* @ Date          : 2019.03.05
* @ Input         : HTTP_RESPONSE_TYPEDEF http_response: 数据
*                   
* @ Output		  : null
* @ Modify        : ...
**/
static void parser_http_head(HTTP_RESPONSE_TYPEDEF http_response)
{
    // printf("system call:%s\r\n",http_response.http_response_line.msg_responseLine_protocol);
    // printf("system call head num:%d\r\n",http_response.http_response_head.head_num);

    // for (int i = 0; i < http_response.http_response_head.head_num;i++)
    // {
    //     printf("system call %d:head:%s--val:%s\r\n",i,http_response.http_response_head.head_list[i].key,http_response.http_response_head.head_list[i].val);
    // }
}

/**
* @ Function Name :parser_http_msg
* @ Author        : ygl
* @ Brief         : 解析器字节消息回调
* @ Date          : 2019.03.05
* @ Input         : char data: 数据
*                   
* @ Output		  : null
* @ Modify        : ...
**/
static void parser_http_msg(char data)
{
    // drv_com0_printf("%c",data);
}


/**
* @ Function Name :parser_http_msg_all
* @ Author        : ygl
* @ Brief         : 解析器消息回调
* @ Date          : 2019.03.05
* @ Input         : char *data: 数据指针
*                   int len :数组长度
* @ Output		  : null
* @ Modify        : ...
**/
static void parser_http_msg_all(char *data, int len)
{
     if (system_http_msg_fun_calback != NULL){
         system_http_msg_fun_calback(data,len);
     }
}


/**
* @ Function Name : http_regist_rec_callback
* @ Author        : ygl
* @ Brief         : 注册接收回调函数
* @ Date          : 2019.03.05
* @ Input         : void (*arg_http_rec_callback)(char data)  http消息消息接收回调函数指针
* @ Output		  : null
* @ Modify        : ...
**/
static void http_regist_rec_callback(void (*arg_http_rec_callback)(char data))
{
    m_http_rec_callback = arg_http_rec_callback;
}


/**
* @ Function Name :http_regist_sentState_callback
* @ Author        : ygl
* @ Brief         : 注册http发送状态回调函数
* @ Date          : 2019.03.05
* @ Input         : void (*arg_http_sentState_callback)(char data) http发送状态回调函数指针
* @ Output		  : null
* @ Modify        : ...
**/
static void http_regist_sentState_callback(void (*arg_http_sentState_callback)(char data))
{
    m_http_sentState_callback = arg_http_sentState_callback;
}



// ---- 对外接口
/**
* @ Function Name :http_init
* @ Author        : ygl
* @ Brief         : http初始化
* @ Date          : 2019.03.05
* @ Input         : 
*                   
* @ Output		  : null
* @ Modify        : ...
**/
void http_init()
{

    // 注册解析器 接收回调函数和发送状态
    parser_regist_printCallback(drv_com0_printf);
    parser_regist_msg_callback(parser_http_msg);
    parser_regist_http_head_callback(parser_http_head);
    parser_regist_http_msg_callback(parser_http_msg_all);

    // http注册接收回调函数
    http_regist_rec_callback(http_rec_callback);
    http_regist_sentState_callback(http_sentState_callback);
}


/**
* @ Function Name : http_regist_state_callback
* @ Author        : ygl
* @ Brief         : 注册状态回调
* @ Date          : 2019.03.05
* @ Input         : arg_state_fun  函数指针
* @ Output		  : null
* @ Modify        : ...
**/
 void http_regist_state_callback(void (*arg_state_fun)(char state))
 {
     system_http_state_fun_callback = arg_state_fun;
 }


/**
* @ Function Name : http_regist_msg_callback
* @ Author        : ygl
* @ Brief         : 注册消息回调
* @ Date          : 2019.03.05
* @ Input         : arg_msg_fun  函数指针
* @ Output		  : null
* @ Modify        : ...
**/
 void http_regist_msg_callback(void (*arg_msg_fun)(char *data, int len))
 {
     system_http_msg_fun_calback = arg_msg_fun;
 }

/**
* @ Function Name : http_send_POST_quest
* @ Author        : ygl
* @ Brief         : 发送post请求
* @ Date          : 2019.03.05
* @ Input         : char *post_url  post地址指针
                    char *content_type 发送数据类型的指针
                    char *post_data  发送的数据
* @ Output		  : int  获得DNS成功或者失败,  0 成功, -1 失败
* @ Modify        : ...
**/
void http_send_POST_quest(char *post_url,char *content_type,char *post_data)
{
    // 地址信息
    struct addrinfo *res;
    int s, r;
    char recv_buf[64];
    char m_host[20]; // host
    char buffer[300]; // 最简单的数据200字节左右
    char *p_host = m_host;
    char *p_packet = buffer;

    // post
    http_post_packet(buffer, post_url, content_type, post_data ,m_host);
    int m_dns_code = http_DNS_getAddr(m_host,&res);
    if (m_dns_code != 0){
         ESP_LOGE(TAG, "DNS lookup failed err");
    }
    // 发送数据包
    int error_code = http_send_packet(res,p_packet);
}

/**
* @ Function Name : http_send_GET_quest
* @ Author        : ygl
* @ Brief         : 发送get请求
* @ Date          : 2019.03.05
* @ Input         : get_url  get地址指针
* @ Output		  : int  获得DNS成功或者失败,  0 成功, -1 失败
* @ Modify        : ...
**/
void http_send_GET_quest(char *get_url)
{
    // 地址信息
    struct addrinfo *res;
    int s, r;
    char m_host[20]; // host
    char *p_host = m_host;
    char *p_packet =request_buffer;
    struct in_addr *addr;
    // get
    http_get_packet(request_buffer,get_url,m_host);
    int m_dns_code = http_DNS_getAddr(m_host,&res);
    if (m_dns_code != 0){
         ESP_LOGE(TAG, "DNS lookup failed err");
    }
    
    // 发送数据包
    int error_code = http_send_packet(res,p_packet);
}



