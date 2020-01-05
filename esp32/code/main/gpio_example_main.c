
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "drv_com.h"
#include "system_module.h"
#include "http_app_demo.h"
#include "https_app_demo.h"
#include "dev_spiffs.h"







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
char SERVER_ADDR[] = "192.168.1.102";
#define SERVER_PORT 5000
#define SERVER_URL      "test.inteink.com"
#define SERVER_PATH     "/upload"

#define HTTP_HEAD       "POST %s HTTP/1.1\r\n"\
                                        "Host: %s\r\n"\
                                        "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:59.0) Gecko/20100101 Firefox/59.0\r\n"\
                                        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"\
                                        "Accept-Language: en-US,en;q=0.5\r\n"\
                                        "Accept-Encoding: gzip, deflate\r\n"\
                                        "Content-Type: multipart/form-data; boundary=%s\r\n"\
                                        "Content-Length: %ld\r\n"\
                                        "Connection: close\r\n"\
                                        "Upgrade-Insecure-Requests: 1\r\n"\
                                        "DNT: 1\r\n\r\n"\

#define POST_FILE_REQUEST  "--%s\r\n"\
                                                "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"\
                                                "Content-Type: %s\r\n\r\n"
 
#define POST_KEY_VALUE_REQUEST "--%s\r\n"\
                                                "Content-Disposition: form-data; name=\"%s\"\r\n\r\n"\
                                                "%s\r\n"

                               // 文件类型
typedef enum File_Type{
	jpeg = 1,
	png,
	bmp,
	bin
}File_Type;

typedef struct HTTP_POST_ITEM_TAG
{
	File_Type file_type;
	char is_file;
	char name[20];
	char name_len;
	char *content;
	long content_len;
	long request_len;
}HTTP_POST_ITEM;


#define MAX_POST_ITEM_LEN 5
typedef struct HTTP_POST_TAG
{
	HTTP_POST_ITEM http_post_item[MAX_POST_ITEM_LEN];
	unsigned char post_item_len;
}HTTP_POST;



HTTP_POST m_http_post;

// 添加到post中
int http_post_add_item(HTTP_POST *http_post,HTTP_POST_ITEM *http_post_item)
{
	if (http_post->post_item_len > MAX_POST_ITEM_LEN){
		return -1;
	}
	http_post->http_post_item[http_post->post_item_len++] = *http_post_item;
	return 0;
}


#define POST_HEAD_LEN 512
#define END_TEMP_LEN 128

#define POST_ITEM_LEN 1024
#define POST_ALL_ITEM_LEN 1024
#define POST_ALL_LEN  POST_ALL_ITEM_LEN + END_TEMP_LEN +128

char http_post_item_buffer[POST_ITEM_LEN];
// post_buff = item_buffer *len + end_temp

char http_post_buffer[POST_ALL_ITEM_LEN]={0};
// POST的总长度

char request[POST_ALL_LEN]={0};
int g_request_len = 0;
char data_temp[20] = "\r\n";

char end_temp[END_TEMP_LEN];


void test(char *IP, const unsigned int port,char *URL,HTTP_POST *http_post)
{

    unsigned long temp_len = 0;
	unsigned long request_len = 0;
	unsigned long request_upload_len = 0;
	unsigned long totalsize = 0;

	for(int i = 0; i < http_post->post_item_len; i++){
		temp_len += http_post->http_post_item[i].name_len + http_post->http_post_item[i].content_len;
	}

	for(int i = 0; i < http_post->post_item_len; i++)
	{	
	
		if(http_post->http_post_item[i].is_file == 5){
			unsigned long request_item_len = snprintf(http_post_item_buffer,POST_ITEM_LEN,POST_FILE_REQUEST,"---------------------------1234",http_post->http_post_item[i].name,"image/png"); //请求信息1
			printf("request_item_len:%ld\n", request_item_len);
			// 复制到复制内容过去
			memcpy(http_post_item_buffer + request_item_len,http_post->http_post_item[i].content,http_post->http_post_item[i].content_len);
	
			// 末尾添加回车
			memcpy(http_post_item_buffer + request_item_len + http_post->http_post_item[i].content_len,data_temp,2);
			
			request_upload_len = request_item_len + http_post->http_post_item[i].content_len + 2;
			// 复制到总的缓冲区
			memcpy(http_post_buffer + request_len,http_post_item_buffer,request_upload_len);
			request_len += request_upload_len;
            printf("step 1\r\n");
        }else{
            printf("step 2\r\n");
			unsigned long request_item_len = snprintf(http_post_item_buffer,POST_ITEM_LEN,POST_KEY_VALUE_REQUEST,"---------------------------1234",http_post->http_post_item[i].name,http_post->http_post_item[i].content); //请求信息1
			request_upload_len = request_item_len;
			memcpy(http_post_buffer + request_len,http_post_item_buffer,request_upload_len);
			request_len += request_upload_len;
		}
		printf("filesize:%ld\n", request_upload_len);
	}
     printf("step 3\r\n");
	// 尾巴数据
	unsigned long end_len = snprintf(end_temp,END_TEMP_LEN,"--%s--\r\n","---------------------------1234"); //结束信息
	memcpy(http_post_buffer + request_len,end_temp,end_len);
	request_len += end_len;
	printf("end_len:%ld\n", end_len);
	
 	// HTTP头
    unsigned long head_len = snprintf(request,1024,HTTP_HEAD,SERVER_PATH,URL,"---------------------------1234",request_len); //头信息
    totalsize = head_len + request_len;
    printf("head_len:%ld\n", head_len);
    printf("totalsize:%ld\n", totalsize);
    
    /******* 拼接http字节流信息 *********/ 
     //http头信息
    memcpy(request + head_len,http_post_buffer,request_len);
    g_request_len = totalsize;
    for (int i = 0; i < totalsize; i++)
    {
        printf("%c", request[i]);
    }
}

static int  http_send_packet(struct addrinfo *res,char *url_quest, int len)
{
    int  s,r;

    // 检测域名和类型,协议类型,返回操作数
    s = socket(res->ai_family, res->ai_socktype, 0);
    // 如果不支持就释放空间
    if(s < 0) {
        freeaddrinfo(res);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("skt1\r\n");
        return ERROR_CODE_SOCKET;
    }
    // 连接到服务器socket
    if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
        close(s);
        freeaddrinfo(res);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        printf("skt1 error\r\n");
        return ERROR_CODE_CONNECT;
    }
    printf("connect ok \r\n");
    freeaddrinfo(res);
    // 写入数据,post还是get
    
    if (write(s, url_quest,len) < 0) {
        close(s);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        printf("write error\r\n");
        return ERROR_CODE_SENT;
    }
    printf("write ok\r\n");

    struct timeval receiving_timeout;
    receiving_timeout.tv_sec = 5;
    receiving_timeout.tv_usec = 0;
    // 接收返回的信息,超时时间为x
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, sizeof(receiving_timeout)) < 0)
     {
        close(s);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
       
         return ERROR_CODE_REC;
    }

    char recv_buf[20];
    // 读取消息
    /* Read HTTP response */
    do {
        memset(recv_buf,0, sizeof(recv_buf));
        r = read(s, recv_buf, sizeof(recv_buf)-1);
        for(int i = 0; i < r; i++) {         
             
        }
    } while(r > 0);
 

    // 关闭socket
    close(s);
    return 0;
}

static void drv_com_m_handle(unsigned char data)
{

}


static void led_task()
{
    while (1)
    {

         vTaskDelay(1000 /portTICK_RATE_MS);
    }
     
}


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
static int http_DNS_getAddr(char *m_host,struct addrinfo **res){
    const struct addrinfo hints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM,
        };
     // DNS查找主机,获得IP地址,存储在res中
     int err = getaddrinfo(m_host, "5000", &hints, res);
     if (err != 0 || res == NULL)
     {  
          
         return -1;
    }
    return 0;
}

void app_normal_mode_run()
{   

    HTTP_POST http_post;
    HTTP_POST_ITEM http_post_item1;

    char name[]="name34";
    char content[] = "kirito";
    char mss_data[] = "1234567890";

    http_post.post_item_len = 0;
    http_post_item1.file_type = jpeg;
    http_post_item1.is_file = 5;

    http_post_item1.name_len = strlen(name) +1;
    printf("len:%d\n",http_post_item1.name_len);
    memcpy(http_post_item1.name,name,http_post_item1.name_len);

    http_post_item1.content_len = strlen(content);
    printf("len:%ld\n",http_post_item1.content_len);
    http_post_item1.content = content;
    http_post_add_item(&http_post,&http_post_item1);

    printf("post_item_len:%d",http_post.post_item_len);
    test(SERVER_ADDR, SERVER_PORT, SERVER_URL, &http_post);


        // 地址信息
    struct addrinfo *res;
    char m_host[20]; // host
    char URL[] = "http://192.168.1.102/home";
    // post
    http_get_host(URL, m_host);
    int m_dns_code = http_DNS_getAddr(m_host, &res);
    if (m_dns_code != 0){
         ESP_LOGE(TAG, "DNS lookup failed err");
    }
    // 发送数据包
    int error_code = http_send_packet(res,request,g_request_len);

    //
    // https_demo_init();
    // dev_spiffs_init();
    drv_com1_printf("hello skt\r\n");

    // http_app_init();
    // xTaskCreate(led_task, "sntp", 1024, NULL, 10, NULL);
}
// 系统准备就绪,查看是否有可升级的
void app_system_ready()
{   
     app_normal_mode_run();
}


void app_main()
{   
    drv_com_init(COM_0, 115200, drv_com_m_handle);
   
    system_module_init(app_system_ready);
}

