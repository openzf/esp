#ifndef __DEV_TCP_SOCKET_CLIENT_H__
#define __DEV_TCP_SOCKET_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

// socket连接状态
typedef enum
 {
    TCP_SOCKET_CREATE_ERROR = 0x1,
    TCP_SOCKET_CREATE_OK = 0x2,
    TCP_SOCKET_CONNECT_ERROR = 0x3,
    TCP_SOCKET_CONNECT_OK = 0x4,
    TCP_SOCKET_SENT_ERROR = 0x5,
    TCP_SOCKET_SENT_OK = 0x6,
    TCP_SOCKET_SOCKET_ERROR = 0x7,
    TCP_SOCKET_SOCKET_CLOSE = 0x8,
}TCP_SOCKET_STATE;

typedef struct Dev_tcp_socket_info_TypeDef_TAG
{
    int socket;
    int error;
}Dev_tcp_socket_info_TypeDef;


// 注册状态回调函数
void dev_regeist_tcp_socket_callBack(void (*arg_dev_tcp_socket_state)(TCP_SOCKET_STATE tcp_socket_state));
// 注册接收回调函数
void dev_regeist_tcp_socket_rec_callBack(void (*arg_dev_tcp_socket_rec_callBack)(char *buffer, int len));

Dev_tcp_socket_info_TypeDef  dev_tcp_socket_create(char *ip_addr,int port);
int dev_tcp_socket_sent(Dev_tcp_socket_info_TypeDef arg_tcp_socket_info, char *msg);
void dev_tcp_socket_close(Dev_tcp_socket_info_TypeDef arg_tcp_socket_info);

#ifdef __cplusplus
}
#endif
#endif