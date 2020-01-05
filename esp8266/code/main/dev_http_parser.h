#ifndef __DEV_HTTP_PARSER_H__
#define __DEV_HTTP_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "m_http_config.h"

void parser_init();
void paser_msg(char data);
void parser_msg_stop();
void parser_regist_printCallback(void (*arg_printf)(char *fmt, ...));
void parser_regist_msg_callback(void (*arg_msg_fun)(char data));
void parser_regist_http_msg_callback(void (*arg_msg_fun)(char *data, int len));
void parser_regist_http_head_callback(void (*arg_http_head_fun)(HTTP_RESPONSE_TYPEDEF arg_http_response));

#ifdef __cplusplus
}
#endif
#endif