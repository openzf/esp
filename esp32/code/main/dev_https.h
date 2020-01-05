#ifndef __DEV_HTTPS_H__
#define __DEV_HTTPS_H__

#ifdef __cplusplus
extern "C" {
#endif
   
    #include "m_http_config.h"

    #define HTTPS_ERROR_CODE_GET_TIME_ERROR 1
    #define HTTPS_ERROR_CODE_GET_TIME_OK 2
    #define HTTPS_ERROR_CODE_CONNECT_ERROR 3
    #define HTTPS_ERROR_CODE_CONNECT_OK 4
    #define HTTPS_ERROR_CODE_WRITE_ERROR 5
    #define HTTPS_ERROR_CODE_READ_RESPONSE_OK  6
    #define HTTPS_ERROR_CODE_READ_ERROR  7
    #define HTTPS_ERROR_CODE_CONNECT_CLOSE  8
    #define HTTPS_REC_END 9

    void dev_https_init();

    void dev_https_regist_state_callback(void (*arg_state_fun)(char state));
    void dev_https_regist_msg_callback(void (*arg_msg_fun)(char *data, int len));

    void https_system_get_request(char *url);
    void https_system_post_request(char *post_url, char *content_type, char *post_data);


#ifdef __cplusplus
}
#endif
#endif