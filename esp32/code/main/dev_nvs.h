#ifndef __DEV_NVS_H__
#define __DEV_NVS_H__

#ifdef __cplusplus
extern "C" {
#endif


void dev_nvs_init();
int dev_nvs_write_i32(char *nameSpace, char *key, int data);
int dev_nvs_read_i32(char *nameSpace, char *key, int *out_data);
int dev_nvs_write_string(char *nameSpace, char *key, char *value);
int dev_nvs_read_string(char *nameSpace, char *key, char *out_data, int *len);
int dev_nvs_write_blob(char *nameSpace, char *key, void *value, int len);
int dev_nvs_read_blob(char *nameSpace,char *key, void *value,int *len);

#ifdef __cplusplus
}
#endif
#endif