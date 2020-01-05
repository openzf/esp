#ifndef __DEV_HTTPS_PARSER_H__
#define __DEV_HTTPS_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "m_http_config.h"

void https_parser_init();
void https_paser_msg(char data);
void https_parser_msg_stop();
void https_parser_regist_https_msg_callback(void (*arg_msg_fun)(char *data, int len));
void https_parser_regist_printCallback(void (*arg_printf)(char *fmt, ...));
void https_parser_regist_msgbyte_callback(void (*arg_msg_fun)(char data));
void https_parser_regist_http_head_callback(void (*arg_http_head_fun)(HTTP_RESPONSE_TYPEDEF arg_http_response));

#ifdef __cplusplus
}
#endif
#endif