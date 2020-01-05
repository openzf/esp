#ifndef __DEV_INSIDE_FLASH_H__
#define __DEV_INSIDE_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif


// 文件分区表存储扇区
#define TABLE_SELECT 120
// 数据表存储扇区
#define DATE_SELECT 130

#define NAME_MAX_LEN 15
#define FILE_MAX_STO_LEN 30
#define MAX_WRITE_BUFFER 1024

#define TALBE_KEY 784
typedef enum
 {
    FILE_STORAGE_TYPE_NUM = 0x0,
    FILE_STORAGE_TYPE_STRING = 0x1,
}FILE_STORAGE_TYPE;

// 文件储存的时候分区表信息,存储在第一个扇区
typedef struct File_storage_info_TypeDef_TAG
{
    char name[NAME_MAX_LEN];
    int select;
    int offset;
    FILE_STORAGE_TYPE type;
    int len;
}File_storage_info_TypeDef;


// 文件储存的时候分区表信息,存储在第一个扇区
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

    void dev_easySql_init(void (*arg_fun)(int state));
    void dev_restore_file_talbe();

    int dev_easySql_write_num(char *key, int num);
    int dev_dev_easySql_read_num(char *key, int *val);

    int dev_easySql_write_string(char *key, char *val);
    int dev_easySql_read_string(char *key, char *val);

#ifdef __cplusplus
}
#endif
#endif