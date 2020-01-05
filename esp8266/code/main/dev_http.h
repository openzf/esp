#ifndef __DRV_HTTP_H__
#define __DRV_HTTP_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ERROR_CODE_SOCKET 1
#define ERROR_CODE_CONNECT 2
#define ERROR_CODE_SENT 3
#define ERROR_CODE_REC 4
#define ERROR_DNS    5
#define OK_RECCODE  6
#define REC_END 7

#include "m_http_config.h"


    // 设置接收回调和发送失败回调
    void http_init();

    void http_regist_state_callback(void (*arg_state_fun)(char state));
    void http_regist_msg_callback(void (*arg_msg_fun)(char *data, int len));


    void http_send_POST_quest(char *post_url, char *content_type, char *post_data);
    void http_send_GET_quest(char *get_url);


#ifdef __cplusplus
}
#endif
#endif