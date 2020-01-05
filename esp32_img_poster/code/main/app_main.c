
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "driver/gpio.h"
#include "camera.h"
#include "bitmap.h"


#define EXAMPLE_ESP_WIFI_MODE_AP CONFIG_ESP_WIFI_MODE_AP  // TRUE:AP FALSE:STA
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_MAX_STA_CONN CONFIG_MAX_STA_CONN
#define CAMERA_LED_GPIO 16

static void wifi_init_sta(void);




static const char* TAG = "camera_demo";

static const char* STREAM_CONTENT_TYPE =
        "multipart/x-mixed-replace; boundary=123456789000000000000987654321";

static const char* STREAM_BOUNDARY = "--123456789000000000000987654321";

static EventGroupHandle_t s_wifi_event_group;
const int CONNECTED_BIT = BIT0;
static ip4_addr_t s_ip_addr;
static camera_pixelformat_t s_pixel_format;

#define CAMERA_FRAME_SIZE  CAMERA_FS_VGA





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

#define POST_ITEM_LEN 30240
#define POST_ALL_ITEM_LEN POST_ITEM_LEN+512
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
        return 1;
    }
    // 连接到服务器socket
    if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
        close(s);
        freeaddrinfo(res);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        printf("skt1 error\r\n");
        return 1;
    }
    printf("connect ok \r\n");
    freeaddrinfo(res);
    // 写入数据,post还是get
    
    if (write(s, url_quest,len) < 0) {
        close(s);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        printf("write error\r\n");
        return 1;
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
       
         return 1;
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

void app_main()
{
    esp_log_level_set("wifi", ESP_LOG_WARN);
    esp_log_level_set("gpio", ESP_LOG_WARN);

    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ESP_ERROR_CHECK( nvs_flash_init() );
    }

    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    gpio_set_direction(CAMERA_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(CAMERA_LED_GPIO, 1);

    camera_config_t camera_config = {
        .ledc_channel = LEDC_CHANNEL_0,
        .ledc_timer = LEDC_TIMER_0,
        .pin_d0 = CONFIG_D0,
        .pin_d1 = CONFIG_D1,
        .pin_d2 = CONFIG_D2,
        .pin_d3 = CONFIG_D3,
        .pin_d4 = CONFIG_D4,
        .pin_d5 = CONFIG_D5,
        .pin_d6 = CONFIG_D6,
        .pin_d7 = CONFIG_D7,
        .pin_xclk = CONFIG_XCLK,
        .pin_pclk = CONFIG_PCLK,
        .pin_vsync = CONFIG_VSYNC,
        .pin_href = CONFIG_HREF,
        .pin_sscb_sda = CONFIG_SDA,
        .pin_sscb_scl = CONFIG_SCL,
        .pin_reset = CONFIG_RESET,
        .xclk_freq_hz = CONFIG_XCLK_FREQ,
    };

    camera_model_t camera_model;
    err = camera_probe(&camera_config, &camera_model);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera probe failed with error 0x%x", err);
        return;
    }

    if (camera_model == CAMERA_OV2640) {
        ESP_LOGI(TAG, "Detected OV2640 camera, using JPEG format");
        s_pixel_format = CAMERA_PF_JPEG;
        camera_config.frame_size = CAMERA_FRAME_SIZE;
        camera_config.jpeg_quality = 15;
    } else {
        ESP_LOGE(TAG, "Camera not supported");
        return;
    }

    camera_config.pixel_format = s_pixel_format;
    err = camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return;
    }


    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();


     HTTP_POST http_post;
    HTTP_POST_ITEM http_post_item1;

    char name[]="skt.jpg";
    char content[] = "kirito";
    char mss_data[] = "1234567890";

    http_post.post_item_len = 0;
    http_post_item1.file_type = jpeg;
    http_post_item1.is_file = 5;

    http_post_item1.name_len = strlen(name) +1;
    printf("len:%d\n",http_post_item1.name_len);
    memcpy(http_post_item1.name,name,http_post_item1.name_len);

    esp_err_t err1 = camera_run();
    if (err1 != ESP_OK) {
        ESP_LOGD(TAG, "Camera capture failed with error = %d", err1);
        return;
    }

    char* p = (char *)camera_get_fb();
    int x = camera_get_data_size();
    printf("camera:%d",x);

    http_post_item1.content_len = x;

    printf("len:%ld\n",http_post_item1.content_len);
    http_post_item1.content = p;
 
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

    ESP_LOGI(TAG, "Free heap: %u", xPortGetFreeHeapSize());
    ESP_LOGI(TAG, "Camera demo ready");

}






// /* FreeRTOS event group to signal when we are connected*/
// static EventGroupHandle_t s_wifi_event_group;

// /* The event group allows multiple bits for each event,
//    but we only care about one event - are we connected
//    to the AP with an IP? */
// const int WIFI_CONNECTED_BIT = BIT0;

static esp_err_t event_handler(void* ctx, system_event_t* event) 
{
  switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
      esp_wifi_connect();
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
      s_ip_addr = event->event_info.got_ip.ip_info.ip;
      xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      ESP_LOGI(TAG, "station:" MACSTR " join, AID=%d", MAC2STR(event->event_info.sta_connected.mac),
               event->event_info.sta_connected.aid);
#if EXAMPLE_ESP_WIFI_MODE_AP
      xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
#endif
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      ESP_LOGI(TAG, "station:" MACSTR "leave, AID=%d", MAC2STR(event->event_info.sta_disconnected.mac),
               event->event_info.sta_disconnected.aid);
#if EXAMPLE_ESP_WIFI_MODE_AP
      xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
#endif
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      esp_wifi_connect();
      xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
      break;
    default:
      break;
  }
  return ESP_OK;
}



static void wifi_init_sta() 
{
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {.ssid = EXAMPLE_ESP_WIFI_SSID, .password = EXAMPLE_ESP_WIFI_PASS},
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s", EXAMPLE_ESP_WIFI_SSID,
            EXAMPLE_ESP_WIFI_PASS);
    
    xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}
