#include "ota.h"
#include "dev_http.h"
#include "cJSON.h"

/* OTA example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "drv_com.h"

const int SDK_VERSION = 27;

char *G_ota_get_url = "http://192.168.1.100/get_update";
#define EXAMPLE_SERVER_IP   "192.168.1.100"
#define EXAMPLE_SERVER_PORT "80"


// #define EXAMPLE_FILENAME "download/gpio.bin"
#define BUFFSIZE 1500
#define TEXT_BUFFSIZE 1024

typedef enum esp_ota_firm_state {
    ESP_OTA_INIT = 0,
    ESP_OTA_PREPARE,
    ESP_OTA_START,
    ESP_OTA_RECVED,
    ESP_OTA_FINISH,
} esp_ota_firm_state_t;

typedef struct esp_ota_firm {
    uint8_t             ota_num;
    uint8_t             update_ota_num;

    esp_ota_firm_state_t    state;

    size_t              content_len;

    size_t              read_bytes;
    size_t              write_bytes;

    size_t              ota_size;
    size_t              ota_offset;

    const char          *buf;
    size_t              bytes;
} esp_ota_firm_t;

static const char *TAG = "ota";
/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };
/*an packet receive buffer*/
static char text[BUFFSIZE + 1] = { 0 };
/* an image total length*/
static int binary_file_length = 0;
/*socket id*/
static int socket_id = -1;

static int m_file_len = 0;
static void (*m_normal_fun)(void);


#define OTA_CHECK_SYSTEM_OK 5
#define OTA_CHECK_SYSTEM_ERROR 6
#define OTA_CONNECT_SERVER_OK 7
#define OTA_CONNECT_SERVER_ERROR 8
#define OTA_ALLOCATE_MEM_ERROR 9
#define OTA_SENT_GET_OK 10
#define OTA_SENT_GET_ERROR 11
#define OTA_BEGIN_WRITE_PARTITION 12
#define OTA_BEGIN_OK 13
#define OTA_BEGIN_ERROR 14
#define OTA_HTTP_REC_DATA_ERROR 15
#define OTA_WRITE_DATA_ERROR 16
#define OTA_CONNECT_CLOSED_REC_OK 17
#define OTA_HTTP_UNEXPECTED_ERROR 18
#define OTA_END_ERROR 19
#define OTA_UPDATE_OK 20
#define REBOOT_NOW 21
#define OTA_SET_BOOT_ERROR 22
#define OTA_HTTP_HEADER_ERROR 23
#define OTA_HTTP_HEADER_CON_LEN_PERSER_ERROR 24

void ota_config_state(int ota_all_select_num,int target_select_num)
{
    ESP_LOGI(TAG, "Totoal OTA number %d update to %d part", ota_all_select_num, target_select_num);
}

void ota_bin_size_state(int len)
{
    ESP_LOGI(TAG, "OTA BIN SIZE %d", len);
}

void ota_download_process(int all_len,int now_len)
{
    int now_per = now_len * 100 / (float)all_len;
    ESP_LOGI(TAG, "ALL:%d P:%d  HW: %dkb", all_len,now_per,now_len/1024);
}

void ota_state_monitor(char state)
{
switch (state)
{
    case OTA_CHECK_SYSTEM_OK:
        ESP_LOGE(TAG, "OTA_CHECK_SYSTEM_OK");
        break;

    case OTA_CHECK_SYSTEM_ERROR:
        ESP_LOGE(TAG, "OTA_CHECK_SYSTEM_ERROR");
        break;

    case OTA_CONNECT_SERVER_OK:
        ESP_LOGE(TAG, "OTA_CONNECT_SERVER_OK");
        break;

    case OTA_CONNECT_SERVER_ERROR:
        ESP_LOGE(TAG, "OTA_CONNECT_SERVER_ERROR");
        break;

    case OTA_ALLOCATE_MEM_ERROR:
        ESP_LOGE(TAG, "OTA_ALLOCATE_MEM_ERROR");
        break;

    case OTA_SENT_GET_OK:
        ESP_LOGE(TAG, "OTA_SENT_GET_OK");
        break;

    case OTA_SENT_GET_ERROR:
        ESP_LOGE(TAG, "OTA_SENT_GET_ERROR");
        break;

    case OTA_BEGIN_WRITE_PARTITION:
        ESP_LOGE(TAG, "OTA_BEGIN_WRITE_PARTITION");
        break;

    case OTA_BEGIN_OK:
        ESP_LOGE(TAG, "OTA_BEGIN_OK");
        break;

    case OTA_BEGIN_ERROR:
        ESP_LOGE(TAG, "OTA_BEGIN_ERROR");
        break;

    case OTA_HTTP_REC_DATA_ERROR:
        ESP_LOGE(TAG, "OTA_HTTP_REC_DATA_ERROR");
        break;

    case OTA_WRITE_DATA_ERROR:
        ESP_LOGE(TAG, "OTA_WRITE_DATA_ERROR");
        break;

    case OTA_CONNECT_CLOSED_REC_OK:
        ESP_LOGE(TAG, "OTA_CONNECT_CLOSED_REC_OK");
        break;

    case OTA_HTTP_UNEXPECTED_ERROR:
        ESP_LOGE(TAG, "OTA_HTTP_UNEXPECTED_ERROR");
        break;

    case OTA_END_ERROR:
        ESP_LOGE(TAG, "OTA_END_ERROR");
        break;
        
    case OTA_UPDATE_OK:
        ESP_LOGE(TAG, "OTA_UPDATE_OK");
        break;

    case REBOOT_NOW:
        ESP_LOGE(TAG, "REBOOT_NOW");
        break;

    case  OTA_SET_BOOT_ERROR:
        ESP_LOGE(TAG, " OTA_SET_BOOT_ERROR");
        break;

    case OTA_HTTP_HEADER_ERROR:
        ESP_LOGE(TAG, "OTA_HTTP_HEADER_ERROR");
        break;

    case  OTA_HTTP_HEADER_CON_LEN_PERSER_ERROR:
        ESP_LOGE(TAG, " OTA_HTTP_HEADER_CON_LEN_PERSER_ERROR");
        break;

    default:
        break;
}
}
/*read buffer by byte still delim ,return read bytes counts*/
static int read_until(const char *buffer, char delim, int len)
{
//  /*TODO: delim check,buffer check,further: do an buffer length limited*/
    int i = 0;
    while (buffer[i] != delim && i < len) {
        ++i;
    }
    return i + 1;
}

int get_version(){
    return SDK_VERSION;
}


// 链接到服务器
static bool connect_to_http_server()
{
    int  http_connect_flag = -1;
    struct sockaddr_in sock_info;

    socket_id = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_id == -1) {
        return false;
    }

    // set connect info
    memset(&sock_info, 0, sizeof(struct sockaddr_in));
    sock_info.sin_family = AF_INET;
    sock_info.sin_addr.s_addr = inet_addr(EXAMPLE_SERVER_IP);
    sock_info.sin_port = htons(atoi(EXAMPLE_SERVER_PORT));

    // connect to http server
    http_connect_flag = connect(socket_id, (struct sockaddr *)&sock_info, sizeof(sock_info));
    if (http_connect_flag == -1) {
        close(socket_id);
        return false;
    } else {
        return true;
    }
    return false;
}

// 发生错误,一直停留
static void __attribute__((noreturn)) task_fatal_error()
{
    ESP_LOGE(TAG, "Exiting task due to fatal error...");
    close(socket_id);
    (void)vTaskDelete(NULL);

    while (1) {
        ;
    }
}

// 解析http
bool _esp_ota_firm_parse_http(esp_ota_firm_t *ota_firm, const char *text, size_t total_len, size_t *parse_len)
{
    /* i means current position */
    int i = 0, i_read_len = 0;
    char *ptr = NULL, *ptr2 = NULL;
    char length_str[32];

    while (text[i] != 0 && i < total_len) {
        if (ota_firm->content_len == 0 && (ptr = (char *)strstr(text, "Content-Length")) != NULL) {
            ptr += 16;
            ptr2 = (char *)strstr(ptr, "\r\n");
            memset(length_str, 0, sizeof(length_str));
            memcpy(length_str, ptr, ptr2 - ptr);
            ota_firm->content_len = atoi(length_str);
#if defined(CONFIG_ESPTOOLPY_FLASHSIZE_1MB) && !defined(CONFIG_ESP8266_BOOT_COPY_APP)
            ota_firm->ota_size = ota_firm->content_len / ota_firm->ota_num;
            ota_firm->ota_offset = ota_firm->ota_size * ota_firm->update_ota_num;
#else
            ota_firm->ota_size = ota_firm->content_len;
            ota_firm->ota_offset = 0;
#endif

        ota_bin_size_state(ota_firm->content_len);
        m_file_len = ota_firm->ota_size;
        }

        i_read_len = read_until(&text[i], '\n', total_len - i);

        if (i_read_len > total_len - i) {
             ota_state_monitor(OTA_HTTP_HEADER_ERROR);
            task_fatal_error();
        }

        // if resolve \r\n line, http header is finished
        if (i_read_len == 2) {
            if (ota_firm->content_len == 0) {
                ota_state_monitor(OTA_HTTP_HEADER_CON_LEN_PERSER_ERROR);
                task_fatal_error();
            }
           
            *parse_len = i + 2;

            return true;
        }

        i += i_read_len;
    }

    return false;
}

static size_t esp_ota_firm_do_parse_msg(esp_ota_firm_t *ota_firm, const char *in_buf, size_t in_len)
{
    size_t tmp;
    size_t parsed_bytes = in_len; 

    switch (ota_firm->state) {
        case ESP_OTA_INIT:
            if (_esp_ota_firm_parse_http(ota_firm, in_buf, in_len, &tmp)) {
                ota_firm->state = ESP_OTA_PREPARE;
                // ESP_LOGD(TAG, "Http parse %d bytes", tmp);
                parsed_bytes = tmp;
            }
            break;
        case ESP_OTA_PREPARE:
            ota_firm->read_bytes += in_len;

            if (ota_firm->read_bytes >= ota_firm->ota_offset) {
                ota_firm->buf = &in_buf[in_len - (ota_firm->read_bytes - ota_firm->ota_offset)];
                ota_firm->bytes = ota_firm->read_bytes - ota_firm->ota_offset;
                ota_firm->write_bytes += ota_firm->read_bytes - ota_firm->ota_offset;
                ota_firm->state = ESP_OTA_START;
                // ESP_LOGD(TAG, "Receive %d bytes and start to update", ota_firm->read_bytes);
                // ESP_LOGD(TAG, "Write %d total %d", ota_firm->bytes, ota_firm->write_bytes);
            }

            break;
        case ESP_OTA_START:
            if (ota_firm->write_bytes + in_len > ota_firm->ota_size) {
                ota_firm->bytes = ota_firm->ota_size - ota_firm->write_bytes;
                ota_firm->state = ESP_OTA_RECVED;
            } else
                ota_firm->bytes = in_len;

            ota_firm->buf = in_buf;

            ota_firm->write_bytes += ota_firm->bytes;

            // ESP_LOGD(TAG, "Write %d total %d", ota_firm->bytes, ota_firm->write_bytes);

            break;
        case ESP_OTA_RECVED:
            parsed_bytes = 0;
            ota_firm->state = ESP_OTA_FINISH;
            break;
        default:
            parsed_bytes = 0;
            // ESP_LOGD(TAG, "State is %d", ota_firm->state);
            break;
    }

    return parsed_bytes;
}

static void esp_ota_firm_parse_msg(esp_ota_firm_t *ota_firm, const char *in_buf, size_t in_len)
{
    size_t parse_bytes = 0;

   //  ESP_LOGD(TAG, "Input %d bytes", in_len);

    do {
        size_t bytes = esp_ota_firm_do_parse_msg(ota_firm, in_buf + parse_bytes, in_len - parse_bytes);
        // ESP_LOGD(TAG, "Parse %d bytes", bytes);
        if (bytes)
            parse_bytes += bytes;
    } while (parse_bytes != in_len);
}

static inline int esp_ota_firm_is_finished(esp_ota_firm_t *ota_firm)
{
    return (ota_firm->state == ESP_OTA_FINISH || ota_firm->state == ESP_OTA_RECVED);
}

static inline int esp_ota_firm_can_write(esp_ota_firm_t *ota_firm)
{
    return (ota_firm->state == ESP_OTA_START || ota_firm->state == ESP_OTA_RECVED);
}

static inline const char* esp_ota_firm_get_write_buf(esp_ota_firm_t *ota_firm)
{
    return ota_firm->buf;
}

static inline size_t esp_ota_firm_get_write_bytes(esp_ota_firm_t *ota_firm)
{
    return ota_firm->bytes;
}

static void esp_ota_firm_init(esp_ota_firm_t *ota_firm, const esp_partition_t *update_partition)
{
    memset(ota_firm, 0, sizeof(esp_ota_firm_t));
    ota_firm->state = ESP_OTA_INIT;
    ota_firm->ota_num = get_ota_partition_count();
    ota_firm->update_ota_num = update_partition->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_0;

    // 打印, 将更新固件到X
    ota_config_state(ota_firm->ota_num, ota_firm->update_ota_num);
}

void ota_begin(char *file_name,char *server_ip,char *port)
{
 esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = NULL;

    // 链接服务器
    /*connect to http server*/
    if (connect_to_http_server()) {
        ota_state_monitor(OTA_CONNECT_SERVER_OK);
    } else {
        ota_state_monitor(OTA_CONNECT_SERVER_ERROR);
        task_fatal_error();
    }
    // 发送请求
    const char *GET_FORMAT =
        "GET %s HTTP/1.0\r\n"
        "Host: %s:%s\r\n"
        "User-Agent: esp-idf/1.0 esp32\r\n\r\n";
    char *http_request = NULL;
    int get_len = asprintf(&http_request, GET_FORMAT,file_name,server_ip,port);
    if (get_len < 0) {
        ota_state_monitor(OTA_ALLOCATE_MEM_ERROR);    
        task_fatal_error();
    }
    int res = send(socket_id, http_request, get_len, 0);
    free(http_request);

    if (res < 0) {
        ota_state_monitor(OTA_SENT_GET_ERROR);  
        task_fatal_error();
    } else {
        ota_state_monitor(OTA_SENT_GET_OK); 
    }
    // 更新分区表信息
    update_partition = esp_ota_get_next_update_partition(NULL);
    ota_state_monitor(OTA_BEGIN_WRITE_PARTITION); 
    
    // ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
    //          update_partition->subtype, update_partition->address);
             
    assert(update_partition != NULL);
    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ota_state_monitor(OTA_BEGIN_ERROR); 
        task_fatal_error();
    }

    ota_state_monitor(OTA_BEGIN_OK); 
    bool flag = true;
    esp_ota_firm_t ota_firm;

    esp_ota_firm_init(&ota_firm, update_partition);

    // 处理接收包
    while (flag) {
        memset(text, 0, TEXT_BUFFSIZE);
        memset(ota_write_data, 0, BUFFSIZE);
        int buff_len = recv(socket_id, text, TEXT_BUFFSIZE, 0);
        // 处理错误
        if (buff_len < 0) { /*receive error*/
            ota_state_monitor(OTA_HTTP_REC_DATA_ERROR); 
            // ESP_LOGE(TAG, "rec data error=%d", errno);
            task_fatal_error();
        } else if (buff_len > 0) { /*deal with response body*/  // 处理消息
            esp_ota_firm_parse_msg(&ota_firm, text, buff_len);

            if (!esp_ota_firm_can_write(&ota_firm))
                continue;
            memcpy(ota_write_data, esp_ota_firm_get_write_buf(&ota_firm), esp_ota_firm_get_write_bytes(&ota_firm));
            buff_len = esp_ota_firm_get_write_bytes(&ota_firm);

            // 写入数据到 内部
            err = esp_ota_write( update_handle, (const void *)ota_write_data, buff_len);
            if (err != ESP_OK) {
                 ota_state_monitor(OTA_WRITE_DATA_ERROR); 
                task_fatal_error();
            }

            binary_file_length += buff_len;

            ota_download_process(m_file_len,binary_file_length);

        } else if (buff_len == 0) {  /*packet over*/ // 包处理完成
            flag = false;
            ota_state_monitor(OTA_CONNECT_CLOSED_REC_OK); 
            close(socket_id);
        } else {
            ota_state_monitor(OTA_HTTP_UNEXPECTED_ERROR); 
        }
        if (esp_ota_firm_is_finished(&ota_firm))
            break;
    }


    if (esp_ota_end(update_handle) != ESP_OK) {
        ota_state_monitor( OTA_END_ERROR); 
        task_fatal_error();
    }

    // 设置分区表
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ota_state_monitor(OTA_SET_BOOT_ERROR); 
        task_fatal_error();
    }
    esp_restart();
    return ;
}



static void arg_state_fun(char state)
{

}

static void arg_msg_fun(char *data, int len)
{
    cJSON *pJsonRoot = cJSON_Parse(data);
    //如果是否json格式数据
    if (pJsonRoot !=NULL) {
        ESP_LOGI(TAG, "is json");
        cJSON *version = cJSON_GetObjectItem(pJsonRoot, "lastVersion");
        cJSON *file_name = cJSON_GetObjectItem(pJsonRoot, "file_name");
        cJSON *download_path = cJSON_GetObjectItem(pJsonRoot, "download_path");
        int now_version = get_version();
        if (version)
        {
            
            if (file_name)
            {
                    if (download_path) 
                    {
                       
                    int server_v = version->valueint;
                    ESP_LOGI(TAG, "file_name: %s", file_name->valuestring);
                    drv_com0_printf("version: %d", server_v);
                    ESP_LOGI(TAG, "download_path: %s", download_path->valuestring);
                    drv_com0_printf("nowV: %d",now_version);

                    if (server_v > now_version)
                    {
                        ESP_LOGI(TAG, "ENTER OTA MODE!!!!");
                        ota_begin(download_path->valuestring, EXAMPLE_SERVER_IP, EXAMPLE_SERVER_PORT);
                                        }else{
                                            ESP_LOGI(TAG, "ENTER NORMAL RUN MODE!!!!"); 
                                            if (m_normal_fun!= NULL){
                                                // 调用运行模式, 把当前请求模式删除
                                                m_normal_fun();
                                                (void)vTaskDelete(NULL);
                                            }
                                        }
                                    } 
                            } 
                        }
                        free(version);
                        free(file_name);
                        free(download_path);
                    }
                    free(pJsonRoot);
}

static void ota_example_task(void *pvParameter)
{

    http_init();
    http_regist_state_callback(arg_state_fun);
    http_regist_msg_callback(arg_msg_fun);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    http_send_GET_quest(G_ota_get_url);

    while(1){
        /* code */
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
     // 
}


int  system_check()
{
    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    // 判断配置是否是OTA模式
    if (configured != running) {
    ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 configured->address, running->address);
    ESP_LOGW(TAG, "this is not OTA mode!");
        return -1;
    }
    // 现在从哪个分区表运行, 分区地址
    ESP_LOGI(TAG, "Running partition type: %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);
    return 0;
}


void ota_init(void (*arg_fun)(void))
{
    m_normal_fun = arg_fun;
    ESP_LOGI(TAG, "SDK_VERSION:%d",get_version());
    if ( system_check() == 0){
        ota_state_monitor(OTA_CHECK_SYSTEM_OK);
        xTaskCreate(&ota_example_task, "ota_example_task", 8192, NULL, 5, NULL);
    }else{
        ota_state_monitor(OTA_CHECK_SYSTEM_ERROR);
    }
}