#include "dev_https_parser.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>





static int String_find(char *src,int index_begin,char c);
static void String_subStr(char *dest, char *src, int index_begin,int len);
static int String_find_sub(char *dest,char *src,int *index_now, char c);

static void https_paser_msg_responseLine(char data);
static void https_paser_msg_responseHead(char data);
static void https_paser_msg_responseBody(char data);

static HTTP_MSG_TYPEDEF m_http_msg;
static int msg_index = 0;
static int msg_len = 0;

// 全局变量
#define MAX_TEMP 1024
static char g_msg_responseLine_temp[MAX_TEMP];

static HTTP_RESPONSE_HEAD_TYPEDEF http_response_head;
static HTTP_RESPONSE_LINE_TYPEDEF http_response_line;

#define MAX_TEMP_RES 50
static char responseLine_temp[MAX_TEMP_RES];

static HTTP_RESPONSE_TYPEDEF http_response;
static void (*m_printf)(char *fmt, ...);
static void (*m_msg_fun)(char data);
static void(*m_http_head_fun)(HTTP_RESPONSE_TYPEDEF arg_http_response);
static void (*fun)(char data);
static void (*m_https_msg_callback)(char *data, int len);

static int content_type = 0;


//-------------本地字符串类---------
// 字符串类查找
static int String_find(char *src,int index_begin,char c)
{	
	char *des = src;
	des = des + index_begin;
	char *pa = des;
	char *pb = strchr(des,c);
	
	if (!pb){
		return 0;
	}else{
		return strlen(pa)-strlen(pb);
	}
	
}

// 字符串截取
static void String_subStr(char *dest, char *src, int index_begin,int len)
{	
	char *src_temp = src;
	src_temp = src_temp + index_begin;
	if (len > 0){
		memcpy(dest, src_temp, len);
		dest = dest + len;
		*dest = '\0';
	}
}

// 查找并截取
static int String_find_sub(char *dest,char *src,int *index_now, char c)
{
	int index_temp = String_find(src, *index_now, c);
	String_subStr(dest, src, *index_now, index_temp);
	return  *index_now += index_temp + 1;
}


/**
* @ Function Name : https_paser_msg_responseLine
* @ Author        : ygl
* @ Brief         : 解析报文响应行
* @ Date          : 2019.03.05
* @ Input         : char data  将要解析的数据
* @ Output		  : null
* @ Modify        : ...
**/
static void https_paser_msg_responseLine(char data)
{

	static int responseLine_index = 0;


	if (responseLine_index > MAX_TEMP_RES){
		m_printf(" response error decode:%c\r\n",data);
	}else{
		responseLine_temp[responseLine_index++] = data;
	}
	if (data == '\r')
	{
		responseLine_temp[responseLine_index-1] = '\0';
		// m_printf("%s!",responseLine_temp);
	}
	if (data == '\n')
	{
		int now_index = 0;

		now_index=  String_find_sub(http_response_line.msg_responseLine_protocol, responseLine_temp, &now_index, '/');
		// m_printf("--%s--\r\n", http_response_line.msg_responseLine_protocol);

		now_index=  String_find_sub(http_response_line.msg_responseLine_protocolV, responseLine_temp, &now_index, ' ');
		// m_printf("--%s--\r\n", http_response_line.msg_responseLine_protocolV);

		char stateCode_temp[8];
		now_index = String_find_sub(stateCode_temp, responseLine_temp, &now_index, ' ');
		http_response_line.msg_responseLine_stateCode = atof(stateCode_temp);
		// m_printf("--%d--\r\n",  http_response_line.msg_responseLine_stateCode);

		now_index = String_find_sub(http_response_line.msg_responseLine_stateMsg, responseLine_temp, &now_index, '\0');
		// m_printf("--%s--\r\n", http_response_line.msg_responseLine_stateMsg);

		// 复制头部行
		memcpy(&http_response.http_response_line, &http_response_line,sizeof(HTTP_RESPONSE_LINE_TYPEDEF));

		fun = https_paser_msg_responseHead;
		responseLine_index = 0;
	}
}


/**
* @ Function Name : https_paser_msg_responseHead
* @ Author        : ygl
* @ Brief         : 解析报文解析头
* @ Date          : 2019.03.05
* @ Input         : char data  将要解析的数据
* @ Output		  : null
* @ Modify        : ...
**/
static void https_paser_msg_responseHead(char data)
{

	static int responseLine_index = 0;
	static int msg_head_packet_byte = 0;
	static int msg_head_num = 0;
	static int msg_head_end_index = 0;
	
	static int http_response_head_index = 0;
	char head_tmep[50];
	char head_val[50];

	if (responseLine_index > MAX_TEMP){
		m_printf("head error decode:%c\r\n",data);
	}else{
		g_msg_responseLine_temp[responseLine_index++] = data;
	}
	msg_head_packet_byte++;

	if (data == '\r')
	{
		g_msg_responseLine_temp[responseLine_index-1] = '\0';
	}
	if (data == '\n')
	{
		responseLine_index = 0;
		msg_head_num++;

		// 如果头部结尾 下一步就是消息体
		
		if ((msg_head_packet_byte - msg_head_end_index) == 2)
		{
			// head的数量
		 msg_head_num--;
		// m_printf("the head num is %d",msg_head_num);		
		 responseLine_index = 0;
		 msg_head_packet_byte = 0;
		 msg_head_num = 0;
	 	 msg_head_end_index = 0;

		// 复制头部行
		memcpy(&http_response.http_response_head, &http_response_head,sizeof(HTTP_RESPONSE_HEAD_TYPEDEF));
		// http头回调
		m_http_head_fun(http_response);

		http_response_head_index = 0;
		 http_response_head.head_num = 0;
         msg_index = 0;
         fun = https_paser_msg_responseBody;
		}else{
			// 查找head key
			int now_index = 0;
			now_index=  String_find_sub(head_tmep, g_msg_responseLine_temp, &now_index, ':');
			
			// 查找val
			int ss = String_find(g_msg_responseLine_temp,0,';');
			// 代表有多个数据,用;分割开
			if (ss>0){	    
			   	now_index=  String_find_sub(head_val, g_msg_responseLine_temp, &now_index, ';');
				// m_printf("--%s--\r\n", head_val);
			}else{
				// now_index=  String_find_sub(head_val, g_msg_responseLine_temp, &now_index, '\0');
				if (responseLine_index < 50){ 
					memcpy(head_val,g_msg_responseLine_temp+now_index, 50-responseLine_index);
				}
			}
			if (http_response_head_index <9){
				// 如果字符串长度大于X就不复制了
				int head_len = strlen(head_tmep);
				if (head_len<MAX_LIST_KEY){
					http_response_head.head_num++;
					http_response_head_index++;
					memcpy(http_response_head.head_list[http_response_head_index].key, head_tmep,head_len+1);
				}else{

				}
				int val_len = strlen(head_val);
				if (head_len< MAX_LIST_VAL){
					memcpy(http_response_head.head_list[http_response_head_index].val, head_val+1,val_len+1);
				}else{

				}
			}
            if (strcmp(head_tmep,"Content-Length") == 0){
              msg_len = atoi(http_response_head.head_list[http_response_head_index].val);
			  m_http_msg.len = msg_len;	
			  content_type = 1;	  
			}
		}
		msg_head_end_index = msg_head_packet_byte;
	}
}


/**
* @ Function Name : https_paser_msg_responseBody
* @ Author        : ygl
* @ Brief         : 解析消息体,如果消息太多久不好处理
* @ Date          : 2019.03.05
* @ Input         : char data  将要解析的数据
* @ Output		  : null
* @ Modify        : ...
**/
static void https_paser_msg_responseBody(char data)
{   
	// m_printf("%c", data);
	m_msg_fun(data);
	// 没有包含长度的
	if (content_type == 0){
		if(msg_index < MAX_TEMP)
		{
			m_http_msg.data[msg_index++] = data;
		}
	}else{
		if(msg_index < m_http_msg.len)
		{
			m_http_msg.data[msg_index++] = data;
		}
		if(msg_index == m_http_msg.len){
			m_https_msg_callback(m_http_msg.data,m_http_msg.len);
			msg_index = 0;
			content_type = 0;
		}
	}

}



// -- ---- 对外接口
/**
* @ Function Name : https_parser_init
* @ Author        : ygl
* @ Brief         : 解析器初始化
* @ Date          : 2019.03.05
* @ Input         : null
* @ Output		  : null
* @ Modify        : ...
**/
void https_parser_init(void)
{
	msg_index = 0;
	content_type = 0;
    fun = https_paser_msg_responseLine;
}


/**
* @ Function Name : https_paser_msg
* @ Author        : ygl
* @ Brief         : 解析消息,由上层调动
* @ Date          : 2019.03.05
* @ Input         : char data 将要解析的数据
* @ Output		  : null
* @ Modify        : ...
**/
void https_paser_msg(char data)
{
	fun(data);
}

// 解析器停止
void https_parser_msg_stop()
{	
	if(content_type == 0){
		m_https_msg_callback(m_http_msg.data,msg_index);
		msg_index = 0;
	}
}


/**
* @ Function Name : https_parser_regist_https_msg_callback
* @ Author        : ygl
* @ Brief         : 注册https消息接收函数
* @ Date          : 2019.03.05
* @ Input         : 注册回调
* @ Output		  : null
* @ Modify        : ...
**/
void https_parser_regist_https_msg_callback(void (*arg_msg_fun)(char *data, int len))
{
	m_https_msg_callback = arg_msg_fun;
}


/**
* @ Function Name : https_parser_regist_printCallback
* @ Author        : ygl
* @ Brief         : 注册打印函数
* @ Date          : 2019.03.05
* @ Input         : void (*arg_printf)(char *fmt, ...) 打印的回调函数指针
* @ Output		  : null
* @ Modify        : ...
**/
void https_parser_regist_printCallback(void (*arg_printf)(char *fmt, ...))
{
	m_printf = arg_printf;
}

/**
* @ Function Name : https_parser_regist_msg_callback
* @ Author        : ygl
* @ Brief         : 注册消息接收
* @ Date          : 2019.03.05
* @ Input         : void (*arg_msg_fun)(char data) 消息的回调函数指针
* @ Output		  : null
* @ Modify        : ...
**/
void https_parser_regist_msgbyte_callback(void (*arg_msg_fun)(char data))
{
	m_msg_fun = arg_msg_fun;
}

/**
* @ Function Name : https_parser_regist_http_head_callback
* @ Author        : ygl
* @ Brief         : 注册http头部接收
* @ Date          : 2019.03.05
* @ Input         : void(*arg_http_head_fun)(HTTP_RESPONSE_TYPEDEF arg_http_response) http头接收回调函数指针
* @ Output		  : null
* @ Modify        : ...
**/
void https_parser_regist_http_head_callback(void(*arg_http_head_fun)(HTTP_RESPONSE_TYPEDEF arg_http_response))
{
	m_http_head_fun = arg_http_head_fun;
}
