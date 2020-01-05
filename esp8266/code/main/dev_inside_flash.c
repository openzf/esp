#include "dev_inside_flash.h"
#include "drv_com.h"


#include <esp_spi_flash.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static void dev_create_file_table(Dev_file_partition_table_TypeDef *arg_m_dev_file_partition_talbe);
static int dev_read_file_table(Dev_file_partition_table_TypeDef *arg_m_dev_file_partition_talbe);
static int dev_easySql_findKey(char *key);
static void (*file_overlord_fun)(int state);

static Dev_file_partition_table_TypeDef dev_file_partition_talbe;


/**
* @ Function Name : drv_flash_read
* @ Author        : ygl
* @ Brief         : 读取flash扇区
* @ Date          : 2018.11.18
* @ Input         : int select 选中的扇区
                    int start_index 扇区偏移
                     uint8_t *write_data 写入的数据指针
                     int len 写入的长度
* @ Output		  : int 如果写入不是4的倍数 就返回-1 正常为0
* @ Modify        : ...
**/
int drv_flash_read(int select, int start_index, uint8_t *write_data, int len)
{
    // 如果写入的长度,不是4的倍数,就直接返回
    if (len%4 != 0)
    {
         drv_com0_printf( "not 4x\r\n");
    return -1;
    }
    // 每次读的4个字节的倍数
    spi_flash_read(select*4*1024+start_index, (uint32_t *)write_data, len); 
    return 0;
}

/**
* @ Function Name : drv_flash_write_withoutRead
* @ Author        : ygl
* @ Brief         : 直接写入扇区,不读取之前的数据 相当于是覆盖
* @ Date          : 2018.11.18
* @ Input         : int select 选中的扇区
                    int start_index 扇区偏移
                     uint8_t *write_data 写入的数据指针
                     int len 写入的长度
* @ Output		  : int 如果写入不是4的倍数 就返回-1 正常为0
* @ Modify        : ...
**/
int drv_flash_write_withoutRead(int select,int start_index, uint8_t *write_data, int len)
{
    char read_len = 0;

    // 如果写入的长度,不是4的倍数,就直接返回
    if (len%4 != 0)
    {
        drv_com0_printf( "not 4x\r\n");
        return -1;
    }
    // 如果是下一个扇区的,目前就认为写入失败
    if (start_index > MAX_WRITE_BUFFER)
    {
        return -1;
    }
     //写入数据
    spi_flash_erase_sector(select);
    spi_flash_write(select * 4 * 1024 + start_index, (uint32_t *)write_data, len); 
    return 0;
}

/**
* @ Function Name : drv_flash_write_withoutRead
* @ Author        : ygl
* @ Brief         : 会保存当前扇区的以前内容
* @ Date          : 2018.11.18
* @ Input         : int select 选中的扇区
                    int start_index 扇区偏移
                     uint8_t *write_data 写入的数据指针
                     int len 写入的长度
* @ Output		  : int 如果写入不是4的倍数,写入长度大于1024 就返回-1 正常为0
* @ Modify        : ...
**/
int drv_flash_write(int select,int start_index, uint8_t *write_data, int len)
{
    char read_buffer[MAX_WRITE_BUFFER]={0};
    char *p = &read_buffer;
    char read_len = 0;

    // 如果写入的长度,不是4的倍数,就直接返回
    if (len%4 != 0)
    {
        drv_com0_printf( "not 4x\r\n");
        return -1;
    }
    // 如果是下一个扇区的,目前就认为写入失败
    if (start_index > MAX_WRITE_BUFFER)
    {
        return -1;
    }

    if (start_index >0){
        // 先存储数据
        drv_flash_read(select, 0, (uint8_t *)p, start_index);
        // 复制到缓冲区
        memcpy(p + start_index, write_data, len);

        //写入数据
        spi_flash_erase_sector(select);
        // 每次写的4个字节的倍数,每次从扇区头开始写
        spi_flash_write(select * 4 * 1024, (uint32_t *)&read_buffer, start_index +len); 
    }else
    {   
        //写入数据
        spi_flash_erase_sector(select);
        spi_flash_write(select * 4 * 1024 + start_index, (uint32_t *)write_data, len); 
    }
    
    return 0;
}


/**
* @ Function Name : dev_easySql_write_num
* @ Author        : ygl
* @ Brief         : 写入字典
* @ Date          : 2018.11.18
* @ Input         : char *key  写入的key
                    int num 写入的数据
* @ Output		  : int 写入长度大于1024,之前有key存在,文件表已经满了 就返回-1 正常为0
* @ Modify        : ...
**/
int dev_easySql_write_num(char *key, int num)
{
    int name_len = strlen(key);
    if (name_len > NAME_MAX_LEN)
    {
        drv_com0_printf( "name to long\r\n");
        return -1;
    }

    // 查找key
    if (dev_easySql_findKey(key) != -1)
    {
        return -1;
    }

    // 如果在引导内
    if (dev_file_partition_talbe.file_now_index < FILE_MAX_STO_LEN)
    {
        int now_file_index = dev_file_partition_talbe.file_now_index++;
        // 复制key_name信息
        memcpy(dev_file_partition_talbe.file_info[now_file_index].name, key, name_len);
        // 存储的扇区
        dev_file_partition_talbe.file_info[now_file_index].select = dev_file_partition_talbe.file_storage_select_addr;
        // 储存的偏移地址
        dev_file_partition_talbe.file_info[now_file_index].offset = dev_file_partition_talbe.file_storage_offset_addr;
        // 存储的类型
        dev_file_partition_talbe.file_info[now_file_index].type = FILE_STORAGE_TYPE_STRING;
        // 存储的长度
        dev_file_partition_talbe.file_info[now_file_index].len = sizeof(num);

        // 下一个文件的要储存的偏移地址
        dev_file_partition_talbe.file_storage_offset_addr += dev_file_partition_talbe.file_info[now_file_index].len;
        // 这里应该加上扇区偏移

        // 写入信息
        drv_flash_write(dev_file_partition_talbe.file_info[now_file_index].select, dev_file_partition_talbe.file_info[now_file_index].offset, (uint8_t *)&num, dev_file_partition_talbe.file_info[now_file_index].len);
            // 每次都要把分区表写一下
        dev_create_file_table(&dev_file_partition_talbe);
        return 0;
    }else{
        file_overlord_fun(1);
        drv_com0_printf( "file has filled\r\n");
        return -1;
    }
 
    
}


/**
* @ Function Name : dev_dev_easySql_read_num
* @ Author        : ygl
* @ Brief         : 写入字典
* @ Date          : 2018.11.18
* @ Input         : char *key  写入的key
                    int num 写入的数据
* @ Output		  : int 如果没有找到key 就返回-1 正常为0
* @ Modify        : ...
**/
int dev_dev_easySql_read_num(char *key, int *val)
{
    int file_index = dev_easySql_findKey(key);
    // 查找key
    if (file_index != -1){
        int temp_data = 0;
        drv_flash_read(dev_file_partition_talbe.file_info[file_index].select, dev_file_partition_talbe.file_info[file_index].offset, (uint8_t *)&temp_data, dev_file_partition_talbe.file_info[file_index].len);
        *val = temp_data;
        return 0;
    }else
    {
        
         return -1;
    }
}



/**
* @ Function Name : dev_easySql_write_num
* @ Author        : ygl
* @ Brief         : 写入字典
* @ Date          : 2018.11.18
* @ Input         : char *key  写入的key
                    char *val 写入的数据
* @ Output		  : int 写入长度大于1024,之前有key存在,文件表已经满了 就返回-1 正常为0
* @ Modify        : ...
**/
int dev_easySql_write_string(char *key, char *val)
{
    
    int name_len = strlen(key);
    if (name_len > NAME_MAX_LEN)
    {
        return -1;
    }

    // 查找key,如果找到了key就返回
    if (dev_easySql_findKey(key) != -1)
    {
        return -1;
    }

    int val_len = strlen(val);
    if (val_len > MAX_WRITE_BUFFER)
    {
        return -1;
    }

    // 如果在引导内
    if (dev_file_partition_talbe.file_now_index < FILE_MAX_STO_LEN)
    {

    int now_file_index = dev_file_partition_talbe.file_now_index++;
    // 复制key_name信息
    memcpy(dev_file_partition_talbe.file_info[now_file_index].name, key, name_len);

    // 存储的扇区
    dev_file_partition_talbe.file_info[now_file_index].select = dev_file_partition_talbe.file_storage_select_addr;
    // 储存的偏移地址
    dev_file_partition_talbe.file_info[now_file_index].offset = dev_file_partition_talbe.file_storage_offset_addr;
    // 存储的类型
    dev_file_partition_talbe.file_info[now_file_index].type = FILE_STORAGE_TYPE_STRING;
    // 存储的长度
    dev_file_partition_talbe.file_info[now_file_index].len = val_len;

    // 一定要4K对齐
    int val_len_offset = val_len % 4;
    int write_len = val_len + 4 - val_len_offset;
    
    // 下一个文件的要储存的偏移地址
    dev_file_partition_talbe.file_storage_offset_addr += write_len;
    // 这里应该加上扇区偏移

    // 写入信息,一定4K对齐
    drv_flash_write(dev_file_partition_talbe.file_info[now_file_index].select, dev_file_partition_talbe.file_info[now_file_index].offset, (uint8_t *)val,  write_len);

    // 每次都要把分区表写一下
    dev_create_file_table(&dev_file_partition_talbe);
    return 0;
    }else{
        file_overlord_fun(1);
        drv_com0_printf("file has filled\r\n");
        return -1;
    }
}

/**
* @ Function Name : dev_easySql_findKey
* @ Author        : ygl
* @ Brief         : 查询key是否存在
* @ Date          : 2018.11.18
* @ Input         : char *key  key
* @ Output		  : int 没有找到key 就返回-1 正常为0
* @ Modify        : ...
**/
static int dev_easySql_findKey(char *key)
{
   int find_file_state = 0;
    int file_index = 0;
    // 查找分区表
    for (int i = 0; i < sizeof(dev_file_partition_talbe.file_info); i++)
    {
        
        if (strcmp(dev_file_partition_talbe.file_info[i].name,key) == 0)
        {
            find_file_state = 1;
            file_index = i;
            break;
        }
    }
    // 如果找到了文件
    if (find_file_state)
    {
         drv_com0_printf( "find key\r\n");  
        return file_index;
    }else{
         drv_com0_printf( "key not find\r\n");  
        return -1;
    }

}


/**
* @ Function Name : dev_easySql_write_num
* @ Author        : ygl
* @ Brief         : 写入字典
* @ Date          : 2018.11.18
* @ Input         : char *key  写入的key
                    char *val 存放的数据指针
* @ Output		  : int 没有找到key返回-1 正常为0
* @ Modify        : ...
**/
int dev_easySql_read_string(char *key,char *val)
{   
    int file_index = dev_easySql_findKey(key);
    // 查找key
    if (file_index != -1){
        // 一定要4K对齐来读取
        int val_len_offset = dev_file_partition_talbe.file_info[file_index].len % 4;
        int read_len = dev_file_partition_talbe.file_info[file_index].len + 4 - val_len_offset;
        // 读取数据
        drv_flash_read(dev_file_partition_talbe.file_info[file_index].select, dev_file_partition_talbe.file_info[file_index].offset, (uint8_t *)val, read_len);
        // 数据末尾加上'\0'
        val[dev_file_partition_talbe.file_info[file_index].len] = '\0';
        return 0;
    }else
    {
        return -1;
    }
}



/**
* @ Function Name : dev_easySql_init
* @ Author        : ygl
* @ Brief         : 写入字典
* @ Date          : 2018.11.18
* @ Input         : void (*arg_fun)(int state)  状态返回回调函数
* @ Output		  : null
* @ Modify        : ...
**/
void dev_easySql_init(void (*arg_fun)(int state))
{

    // 注册满了回调函数
    file_overlord_fun = arg_fun;

    Dev_file_partition_table_TypeDef m_dev_file_partition_talbe;
    memset(&m_dev_file_partition_talbe, 0, sizeof(Dev_file_partition_table_TypeDef));

    drv_com0_printf( "Reading Flash FileTable\r\n");  
    int error = dev_read_file_table(&m_dev_file_partition_talbe);
    // 创建分区表
    if (error != 0)
    {
        // 重新刷新缓冲区
         memset(&m_dev_file_partition_talbe, 0, sizeof(Dev_file_partition_table_TypeDef));
         // 数据存储地址
         m_dev_file_partition_talbe.file_storage_select_addr = DATE_SELECT;
         m_dev_file_partition_talbe.file_now_index = 0;
         m_dev_file_partition_talbe.key = TALBE_KEY;

         // 创建分区
        dev_create_file_table(&m_dev_file_partition_talbe);
        dev_file_partition_talbe = m_dev_file_partition_talbe;
        drv_com0_printf( "Read Table Error...Create New Table!!!\r\n");  
    }else
    {
        dev_file_partition_talbe = m_dev_file_partition_talbe;
        drv_com0_printf( "Read Tanle OK...\r\n");  
    }

}

/**
* @ Function Name : dev_restore_file_talbe
* @ Author        : ygl
* @ Brief         : 重建分区表
* @ Date          : 2018.11.18
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
void dev_restore_file_talbe()
{
    Dev_file_partition_table_TypeDef m_dev_file_partition_talbe;

// 重新刷新缓冲区
    memset(&m_dev_file_partition_talbe, 0, sizeof(Dev_file_partition_table_TypeDef));
    // 数据存储地址
    m_dev_file_partition_talbe.file_storage_select_addr = DATE_SELECT;
    m_dev_file_partition_talbe.file_now_index = 0;
    m_dev_file_partition_talbe.key = TALBE_KEY;

        // 创建分区
    dev_create_file_table(&m_dev_file_partition_talbe);
    dev_file_partition_talbe = m_dev_file_partition_talbe;
    drv_com0_printf( "Restore...Create New Table!!!\r\n");  
}

/**
* @ Function Name : dev_restore_file_talbe
* @ Author        : ygl
* @ Brief         : 创建分区表
* @ Date          : 2018.11.18
* @ Input         : Dev_file_partition_table_TypeDef *arg_m_dev_file_partition_talbe 将要创建的分区表指针
* @ Output		  : null
* @ Modify        : ...
**/
static void dev_create_file_table(Dev_file_partition_table_TypeDef *arg_m_dev_file_partition_talbe)
{

    int talbe_len = sizeof(Dev_file_partition_table_TypeDef);
    int write_len = talbe_len;

    // 直接写入数据
    drv_flash_write_withoutRead(TABLE_SELECT, 0, (uint8_t *)arg_m_dev_file_partition_talbe, write_len);
}


/**
* @ Function Name : dev_restore_file_talbe
* @ Author        : ygl
* @ Brief         : 创建分区表
* @ Date          : 2018.11.18
* @ Input         : Dev_file_partition_table_TypeDef *arg_m_dev_file_partition_talbe 将要创建存放的分区表指针
* @ Output		  : int 如果读取到就返回0  没有读取到就返回-1
* @ Modify        : ...
**/
static int dev_read_file_table(Dev_file_partition_table_TypeDef *arg_m_dev_file_partition_talbe)
{
    // 初始化
    memset(arg_m_dev_file_partition_talbe, 0, sizeof(Dev_file_partition_table_TypeDef));

    // 分区大小
   int talbe_len = sizeof(Dev_file_partition_table_TypeDef);
    int read_len = talbe_len;

    // 读取数据
    drv_flash_read(TABLE_SELECT, 0, (uint8_t *)arg_m_dev_file_partition_talbe, read_len);
     drv_com0_printf( "read table key:%d\r\n",arg_m_dev_file_partition_talbe->key);  
      drv_com0_printf( "read table index:%d\r\n",arg_m_dev_file_partition_talbe->file_now_index);  
     if (arg_m_dev_file_partition_talbe->key == TALBE_KEY)
    {
        return 0;
    }else
    {
        return -1;
    }

}





