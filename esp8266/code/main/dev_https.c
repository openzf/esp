#include "dev_https.h"
#include "dev_https_parser.h"
#include "drv_com.h"


#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "cJSON.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"


#include "lwip/apps/sntp.h"
#include "esp_tls.h"


#define WEB_PORT 443
static const char *TAG = "example";
typedef struct HTTPS_callback_TypeDef_TAG
{
    void (*m_https_sentState_callback)(char data);
    void (*m_https_rec_callback)(char data);
}HTTPS_callback_TypeDef;

static void https_sent_packet(HTTPS_callback_TypeDef *https_callback, char *web_server, char *request_data);

extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

static HTTPS_callback_TypeDef m_https;
static char buf[1024];
static esp_tls_cfg_t cfg;

static void (*system_arg_state_fun)(char state);
static void (*system_arg_msg_fun)(char *data, int len);


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
static void https_get_host(char *URL,char *host) 
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
static void https_get_packet(char *packet_buffer,char *url,char *m_host)
{
    // 获得host
    https_get_host(url,m_host);
     sprintf(packet_buffer, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: esp-idf/1.0 esp32\r\n\r\n", url, m_host);
}

/**
* @ Function Name : https_post_packet
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
static void https_post_packet(char *packet_buffer,char *url,char *contentType,char *data,char *m_host)
{
    int m_len = strlen(data);
    // 获得host
    https_get_host(url,m_host);
    sprintf(packet_buffer, "POST %s HTTP/1.0\r\nHost: %s\r\nContent-Length: %d\r\nContent-Type: %s\r\nUser-Agent: esp-idf/1.0 esp32\r\n\r\n%s", url, m_host, m_len,contentType,data);
}


/**
* @ Function Name :https_sent_packet
* @ Author        : ygl
* @ Brief         : https发送包
* @ Date          : 2019.03.05
* @ Input         : HTTPS_callback_TypeDef *https_callback:  状态和消息回调
                    char *web_server: 服务器host
                    char *request_data :请求的数据
* @ Output		  : null
* @ Modify        : ...
**/
static void https_sent_packet(HTTPS_callback_TypeDef *https_callback,char *web_server,char *request_data)
{
    int ret, len;

    ESP_LOGI("SD", "CONNECT BEGIN:%s",web_server);
    // 连接服务器
    struct esp_tls *tls = esp_tls_conn_new(web_server, strlen(web_server), WEB_PORT, &cfg);
    // 连接成功
    if (tls != NULL)
    {   
        ESP_LOGI("SD", "CONNECT OK");
        https_callback->m_https_sentState_callback(HTTPS_ERROR_CODE_CONNECT_OK);
    }
    else
    {
        https_callback->m_https_sentState_callback(HTTPS_ERROR_CODE_CONNECT_ERROR);
        goto exit;
    }
     ESP_LOGI("SD", "CONNECT");
    size_t written_bytes = 0;
    do
    {
        ret = esp_tls_conn_write(tls,
                                    request_data + written_bytes,
                                    strlen(request_data) - written_bytes);
        if (ret >= 0)
        {
            written_bytes += ret;
        }
        else if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            https_callback->m_https_sentState_callback(HTTPS_ERROR_CODE_WRITE_ERROR);
            goto exit;
        }
    } while (written_bytes < strlen(request_data));

    https_callback->m_https_sentState_callback(HTTPS_ERROR_CODE_READ_RESPONSE_OK);
    do
    {
        len = sizeof(buf) - 1;
        bzero(buf, sizeof(buf));
        ret = esp_tls_conn_read(tls, (char *)buf, len);

        if (ret == MBEDTLS_ERR_SSL_WANT_WRITE || ret == MBEDTLS_ERR_SSL_WANT_READ)

            continue;

        if (ret < 0)
        {
            https_callback->m_https_sentState_callback(HTTPS_ERROR_CODE_READ_ERROR);
            break;
        }

        if (ret == 0)
        {
            https_callback->m_https_sentState_callback(HTTPS_ERROR_CODE_CONNECT_CLOSE);
            break;
        }
        len = ret;
        /* Print response directly to stdout as it is read */
        for (int i = 0; i < len; i++)
        {
            // drv_com0_printf("%c",buf[i]);
            https_callback->m_https_rec_callback(buf[i]);             
        }
         https_callback->m_https_sentState_callback(HTTPS_REC_END);
        
    } while (1);
exit:
    esp_tls_conn_delete(tls);
}


/**
* @ Function Name :https_sent_get_packet
* @ Author        : ygl
* @ Brief         : https发送包
* @ Date          : 2019.03.05
* @ Input         : char *get_url url地址
                    HTTPS_callback_TypeDef *https_callback :消息和状态回调
* @ Output		  : null
* @ Modify        : ...
**/
 static void https_sent_get_packet(char *get_url,HTTPS_callback_TypeDef *https_callback)
{
   char m_host[20]; // host
   char buffer[300]; // 最简单的数据200字节左右
   // get打包,request储存在buffer中
   https_get_packet(buffer,get_url,m_host);

   https_sent_packet(https_callback, m_host, buffer);
}

/**
* @ Function Name :https_sent_post_packet
* @ Author        : ygl
* @ Brief         : https发送包 post
* @ Date          : 2019.03.05
* @ Input         : char *get_url url地址
                    char *content_type :内容格式
                    char *post_data :数据
                    HTTPS_callback_TypeDef *https_callback :消息和状态回调
* @ Output		  : null
* @ Modify        : ...
**/
 static  void https_sent_post_packet(char *post_url,char *content_type,char *post_data,HTTPS_callback_TypeDef *https_callback)
{
   char m_host[20]; // host
   char buffer[300]; // 最简单的数据200字节左右
   // get打包,request储存在buffer中
   https_post_packet(buffer, post_url, content_type, post_data ,m_host);

   https_sent_packet(https_callback, m_host, buffer);
}


/**
* @ Function Name :https_state_callback
* @ Author        : ygl
* @ Brief         : https 状态回调
* @ Date          : 2019.03.05
* @ Input         : char data: 状态
* @ Output		  : null
* @ Modify        : ...
**/
static void https_state_callback(char data)
{
    switch (data)
    {
        case HTTPS_ERROR_CODE_READ_RESPONSE_OK:
            https_parser_init();
            break;
        case HTTPS_REC_END:
            https_parser_msg_stop();
            break;
        default:
            break;
    }
    if(system_arg_state_fun != 0){
        system_arg_state_fun(data);
    }
}


/**
* @ Function Name :https_rec_callback
* @ Author        : ygl
* @ Brief         : http接收到消息回调, 传递到解析器
* @ Date          : 2019.03.05
* @ Input         : char data: 数据
* @ Output		  : null
* @ Modify        : ...
**/
static void https_rec_callback(char data)
{
    https_paser_msg(data);
}



/**
* @ Function Name :https_parser_msg_byte_callback
* @ Author        : ygl
* @ Brief         : 解析器后的消息,字节
* @ Date          : 2019.03.05
* @ Input         : char data: 数据
* @ Output		  : null
* @ Modify        : ...
**/
static void https_parser_msg_byte_callback(char data)
{
    // drv_com0_printf("%c",data);
}

/**
* @ Function Name :https_parser_head_callback
* @ Author        : ygl
* @ Brief         : 解析器后的头
* @ Date          : 2019.03.05
* @ Input         : HTTP_RESPONSE_TYPEDEF *arg_http_response: 数据
* @ Output		  : null
* @ Modify        : ...
**/
static void https_parser_head_callback(HTTP_RESPONSE_TYPEDEF *arg_http_response)
{

}

/**
* @ Function Name :https_parser_msg_callback
* @ Author        : ygl
* @ Brief         : 解析器后的数据
* @ Date          : 2019.03.05
* @ Input         : char *data:数据指针
                      int len :数据长度
* @ Output		  : null
* @ Modify        : ...
**/
static void https_parser_msg_callback(char *data,int len)
{
    if(system_arg_msg_fun != 0){
         system_arg_msg_fun(data,len);
    }
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
static void https_regist_sentState_callback(void (*arg_https_sentState_callback)(char data))
    {
        m_https.m_https_sentState_callback = arg_https_sentState_callback;
}

    /**
* @ Function Name :https_regist_rec_callback
* @ Author        : ygl
* @ Brief         : 注册http发送消息回调函数
* @ Date          : 2019.03.05
* @ Input         : void (*arg_http_rec_callback)(char data) http发送消息回调函数指针
* @ Output		  : null
* @ Modify        : ...
**/
static void https_regist_rec_callback(void (*arg_http_rec_callback)(char data))
{
    m_https.m_https_rec_callback = arg_http_rec_callback;
}


// ------- 对外接口

    /**
* @ Function Name :dev_https_init
* @ Author        : ygl
* @ Brief         : https初始化
* @ Date          : 2019.03.05
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
  void dev_https_init()
  {  
    // 证书初始化
    cfg.cacert_pem_buf = server_root_cert_pem_start;
    cfg.cacert_pem_bytes = server_root_cert_pem_end - server_root_cert_pem_start;

    //https状态和接收
    https_regist_sentState_callback(https_state_callback);
    https_regist_rec_callback(https_rec_callback);

    // 解析器
    https_parser_regist_printCallback(drv_com0_printf);
    https_parser_regist_http_head_callback( https_parser_head_callback);
    https_parser_regist_msgbyte_callback(https_parser_msg_byte_callback);
    https_parser_regist_https_msg_callback(https_parser_msg_callback);
  }


    /**
* @ Function Name :dev_https_regist_state_callback
* @ Author        : ygl
* @ Brief         : https 注册状态回调
* @ Date          : 2019.03.05
* @ Input         : void (*arg_state_fun)(char state) :函数指针
* @ Output		  : null
* @ Modify        : ...
**/
  void dev_https_regist_state_callback(void (*arg_state_fun)(char state))
{
    system_arg_state_fun = arg_state_fun;
}

    /**
* @ Function Name :dev_https_regist_msg_callback
* @ Author        : ygl
* @ Brief         : https 注册消息回调
* @ Date          : 2019.03.05
* @ Input         : void (*arg_msg_fun)(char *data,int len):函数指针
* @ Output		  : null
* @ Modify        : ...
**/
void dev_https_regist_msg_callback(void (*arg_msg_fun)(char *data,int len))
{
    system_arg_msg_fun = arg_msg_fun;
}

    /**
* @ Function Name :https_system_get_request
* @ Author        : ygl
* @ Brief         : https发送get
* @ Date          : 2019.03.05
* @ Input         : char *url :url地址
* @ Output		  : null
* @ Modify        : ...
**/
void https_system_get_request(char *url)
{
    https_sent_get_packet(url, &m_https); 
}

    /**
* @ Function Name :https_system_post_request
* @ Author        : ygl
* @ Brief         : https发送post
* @ Date          : 2019.03.05
* @ Input         : char *url :url地址
                    char *content_type :数据类型
                    char *post_data :数据
* @ Output		  : null
* @ Modify        : ...
**/
void https_system_post_request(char *post_url,char *content_type,char *post_data)
{
    https_sent_post_packet(post_url, content_type, post_data, &m_https);
}
