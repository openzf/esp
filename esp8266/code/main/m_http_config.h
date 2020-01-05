#ifndef __M_HTTP_CONFIG_H__
#define __M_HTTP_CONFIG_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// GET 请求不存在请求实体部分，键值对参数放置在 URL 尾部，浏览器把form数据转换成一个字串
/*
POST 请求
Http Header里的Content-Type一般有这三种：
1.application/x-www-form-urlencoded：数据被编码为名称/值对。这是标准的编码格式。默认行为。
会将表单内的数据转换拼接成 key-value 对（非 ASCII 码进行编码）
2.multipart/form-data(一般用来上传文件)： 数据被编码为一条消息，页上的每个控件对应消息中的一个部分
3.text/plain： 数据以纯文本形式(text/json/xml/html)进行编码，其中不含任何控件或格式字符。postman软件里标的是RAW。（中文不进行编码）
*/

#define CONTENT_TYPE_HTML "text/html"
#define CONTENT_TYPE_TEXT "text/plain"
#define CONTENT_TYPE_XML  "text/xml"

#define CONTENT_TYPE_GIF  "image/gif"
#define CONTENT_TYPE_JPEG "image/jpeg"
#define CONTENT_TYPE_PNG  "image/png"


#define CONTENT_TYPE_A_XML  "application/xml"
#define CONTENT_TYPE_A_JSON "application/json"
#define CONTENT_TYPE_A_PDF  "application/pdf"
#define CONTENT_TYPE_A_WORD "application/msword"
#define CONTENT_TYPE_A_OCS  "application/octet-stream" // 二进制流
#define CONTENT_TYPE_A_FORM "application/x-www-form-urlencoded" // 提交的表单数据

#define CONTENT_TYPE_FORMDATA "multipart/form-data"  // 表单上传文件
#define CONTENT_TYPE_MP3 "audio/mp3"
#define CONTENT_TYPE_MP4 "video/mpeg4"


#define MAX_TEMP 500
// http消息
typedef struct HTTP_MSG_TAG
{
	char data[MAX_TEMP];
	int len;
}HTTP_MSG_TYPEDEF;

// 报文响应行
typedef struct HTTP_RESPONSE_LINETAG
{
	char msg_responseLine_protocol[10]; // 报文响应行协议
	char msg_responseLine_protocolV[5]; // 报文响应行协议
	int  msg_responseLine_stateCode; 	// 报文响应行状态代码
	char msg_responseLine_stateMsg[10]; // 报文响应行状态消息
}HTTP_RESPONSE_LINE_TYPEDEF;


#define MAX_LIST_KEY 20
#define MAX_LIST_VAL 30
// 实现了列表
typedef struct LIST_TAG
{
	char type; // 数据类型
	char key[MAX_LIST_KEY];
	char val[MAX_LIST_VAL];
	int int_val;
}LIST_TYPEDEF;

// http头部
typedef struct HTTP_RESPONSE_HEAD_TAG
{
	LIST_TYPEDEF head_list[10];
	int head_num;
}HTTP_RESPONSE_HEAD_TYPEDEF;

// http返回信息
typedef struct HTTP_RESPONSE_TAG
{
	HTTP_RESPONSE_LINE_TYPEDEF http_response_line;
	HTTP_RESPONSE_HEAD_TYPEDEF http_response_head;
}HTTP_RESPONSE_TYPEDEF;


#ifdef __cplusplus
}
#endif
#endif