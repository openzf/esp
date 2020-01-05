#include "dev_nvs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include "nvs.h"
#include "esp_partition.h"
#include "esp_log.h"


static const char* TAG = "test_nvs";


/**
* @ Function Name :  dev_nvs_init
* @ Author        : ygl
* @ Brief         : 初始化存储
* @ Date          : 2019.03.05
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
void dev_nvs_init()
{
   nvs_handle handle_1;
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_LOGW(TAG, "nvs_flash_init failed (0x%x), erasing partition and retrying", err);
        const esp_partition_t* nvs_partition = esp_partition_find_first(
                ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
        assert(nvs_partition && "partition table must have an NVS partition");
        ESP_ERROR_CHECK( esp_partition_erase_range(nvs_partition, 0, nvs_partition->size) );
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
}


/**
* @ Function Name :  dev_nvs_write_i32
* @ Author        : ygl
* @ Brief         : 存储数字
* @ Date          : 2019.03.05
* @ Input         : char *nameSpace 命名空间
                    char *key 键
                    int data 值
* @ Output		  : int 写入成功返回0 失败-1
* @ Modify        : ...
**/
int dev_nvs_write_i32(char *nameSpace,char *key,int data)
{
    nvs_handle handle;
    esp_err_t err1 = nvs_open(nameSpace, NVS_READWRITE, &handle);
    esp_err_t err2 = nvs_set_i32(handle, key, data);
    nvs_commit(handle);
    nvs_close(handle);
    if ( err2 != ESP_OK){
        return -1;
    }else{
        return 0;
    }
}

/**
* @ Function Name :  dev_nvs_read_i32
* @ Author        : ygl
* @ Brief         : 读取存储的数字
* @ Date          : 2019.03.05
* @ Input         : char *nameSpace 命名空间
                    char *key 键
                    int *out_data 将要存储的数字空间
* @ Output		  : int 写入成功返回0 失败-1
* @ Modify        : ...
**/
int dev_nvs_read_i32(char *nameSpace,char *key,int *out_data)
{
    nvs_handle handle;
    esp_err_t err1= nvs_open(nameSpace, NVS_READWRITE, &handle);
    esp_err_t err2= nvs_get_i32(handle, key, out_data);
    nvs_close(handle);
        if ( err2 != ESP_OK){
        return -1;
    }else{
        return 0;
    }
}


/**
* @ Function Name :  dev_nvs_write_string
* @ Author        : ygl
* @ Brief         : 存储字符串
* @ Date          : 2019.03.05
* @ Input         : char *nameSpace 命名空间
                    char *key 键
                    char *value 将要写入的值
* @ Output		  : int 写入成功返回0 失败-1
* @ Modify        : ...
**/
int dev_nvs_write_string(char *nameSpace,char *key, char *value)
{
    nvs_handle handle;
    // esp_err_t err1 = nvs_open(nameSpace, NVS_READWRITE, &handle);
    // esp_err_t err2 = nvs_set_str(handle, key, value);
    // nvs_commit(handle);
    // nvs_close(handle);
    // if ( err2 != ESP_OK){
    //     return -1;
    // }else{
    //     return 0;
    // }

    ESP_ERROR_CHECK( nvs_open( nameSpace, NVS_READWRITE, &handle) );
    ESP_ERROR_CHECK( nvs_set_str(handle, key, value) );
    ESP_ERROR_CHECK( nvs_commit(handle) );
    nvs_close(handle);
    return 0;
}

/**
* @ Function Name :  dev_nvs_read_string
* @ Author        : ygl
* @ Brief         : 读取存储的字符串
* @ Date          : 2019.03.05
* @ Input         : char *nameSpace 命名空间
                    char *key 键
                     char *out_data 存储的空间地址
                     int len 存储空间的大小
* @ Output		  : int 写入成功返回0 失败-1
* @ Modify        : ...
**/
int dev_nvs_read_string(char *nameSpace,char *key, char *out_data,int *len)
{
    nvs_handle handle;
    esp_err_t err1 = nvs_open(nameSpace, NVS_READWRITE, &handle);
    esp_err_t err2 = nvs_get_str(handle, key, out_data,(uint32_t *)len);
    nvs_close(handle);
    if ( err2 != ESP_OK){
        return -1;
    }else{
        return 0;
    }
}


/**
* @ Function Name :  dev_nvs_write_blob
* @ Author        : ygl
* @ Brief         : 存储任何类型
* @ Date          : 2019.03.05
* @ Input         : char *nameSpace 命名空间
                    char *key 键
                     void *value 任意指针
                     int len 数据大小
* @ Output		  : int 写入成功返回0 失败-1
* @ Modify        : ...
**/
int dev_nvs_write_blob(char *nameSpace,char *key, void *value,int len)
{
    nvs_handle handle;
    esp_err_t err1 = nvs_open(nameSpace, NVS_READWRITE, &handle);
    esp_err_t err2 = nvs_set_blob(handle, key, value,len);
    nvs_commit(handle);
    nvs_close(handle);
    if ( err2 != ESP_OK){
        return -1;
    }else{
        return 0;
    }
}

/**
* @ Function Name :  dev_nvs_write_blob
* @ Author        : ygl
* @ Brief         : 存储任何类型
* @ Date          : 2019.03.05
* @ Input         : char *nameSpace 命名空间
                    char *key 键
                     void *value 存储任意指针
                     int len 数据大小
* @ Output		  : int 写入成功返回0 失败-1
* @ Modify        : ...
**/
int dev_nvs_read_blob(char *nameSpace,char *key, void *value,int *len)
{
    nvs_handle handle;
    esp_err_t err1 = nvs_open(nameSpace, NVS_READWRITE, &handle);
    esp_err_t err2 = nvs_get_blob(handle, key, value,(uint32_t *)len);
    nvs_close(handle);
    if ( err2 != ESP_OK){
        return -1;
    }else{
        return 0;
    }
}


