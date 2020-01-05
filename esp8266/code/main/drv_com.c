#include "drv_com.h"

#include "stdarg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"



static const char *TAG = "uart_events";

// 缓冲区大小
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

// 接收事件
static QueueHandle_t uart0_queue;
static QueueHandle_t uart1_queue;


static Drv_com_rec_callback_TypeDef com_rec_callback;

// 接收回调函数
static void (*drv_com0_handle)(unsigned char data);
static void (*drv_com1_handle)(unsigned char data);

static void uart_event_task(void *pvParameters);





/**
* @ Function Name : drv_com1_write_bytes
* @ Author        : ygl
* @ Brief         : 端口控制器0写字节
* @ Date          : 2018.11.18
* @ Input         : char *data 写入的数据指针
*					int len 写入的长度
* @ Output		  : null
* @ Modify        : ...
**/
void drv_com0_write_bytes(char *data,int len)
{
    uart_write_bytes(UART_NUM_0, (const char *)data, len);
}

/**
* @ Function Name : drv_com1_write_bytes
* @ Author        : ygl
* @ Brief         : 端口控制器1写字节
* @ Date          : 2018.11.18
* @ Input         : char *data 写入的数据指针
*					int len 写入的长度
* @ Output		  : null
* @ Modify        : ...
**/
void drv_com1_write_bytes(char *data,int len)
{
    uart_write_bytes(UART_NUM_1, (const char *)data, len);
}


/**
* @ Function Name : drv_com0_printf
* @ Author        : ygl
* @ Brief         : 端口控制器0重定向
* @ Date          : 2018.11.18
* @ Input         : 函数可变参数 ,参考print
* @ Output		  : null
* @ Modify        : ...
**/
void drv_com0_printf(char *fmt, ...)
{
	char buffer[DRV_USART_BUFFER_LEN+1];  
	char *p = buffer;
	va_list arg_ptr;
	va_start(arg_ptr, fmt);  
	vsnprintf(buffer, DRV_USART_BUFFER_LEN+1, fmt, arg_ptr);
	while(*p != '\0'){
		/* 发送一个字节数据到串口 */	
         uart_write_bytes(UART_NUM_0, (const char *)p++, 1);
	}
	va_end(arg_ptr);
}

/**
* @ Function Name : drv_com1_printf
* @ Author        : ygl
* @ Brief         : 端口控制器1重定向
* @ Date          : 2018.11.18
* @ Input         : 函数可变参数 ,参考print
* @ Output		  : null
* @ Modify        : ...
**/
void drv_com1_printf(char *fmt, ...)
{
	char buffer[DRV_USART_BUFFER_LEN+1];  
	char *p = buffer;
	va_list arg_ptr;
	va_start(arg_ptr, fmt);  
	vsnprintf(buffer, DRV_USART_BUFFER_LEN+1, fmt, arg_ptr);
	while(*p != '\0'){
		/* 发送一个字节数据到串口 */	
         uart_write_bytes(UART_NUM_0, (const char *)p++, 1);
	}
	va_end(arg_ptr);
}


/**
* @ Function Name : uart_event_task
* @ Author        : ygl
* @ Brief         : 端口控制器X串口接收任务重
* @ Date          : 2018.11.18
* @ Input         : 函数可变参数 ,参考task
* @ Output		  : null
* @ Modify        : ...
**/
static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
	uart_port_t uart_x;
	QueueHandle_t *uartx_queue;
    uint8_t *dtmp = (uint8_t *) malloc(RD_BUF_SIZE);

	// 取出参数
	Drv_com_rec_callback_TypeDef *com_rec_callback;
	com_rec_callback =(Drv_com_rec_callback_TypeDef*)pvParameters;
	
	// 取出缓冲队列
	if (com_rec_callback->com_x == COM_0 ){
		uartx_queue = &uart0_queue;
		uart_x = UART_NUM_0;
	}else{
		uartx_queue = &uart1_queue;
		uart_x = UART_NUM_1;
	}

	
	for (;;) {
        // Waiting for UART event.
        if (xQueueReceive(*uartx_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
            bzero(dtmp, RD_BUF_SIZE);
            switch (event.type) {
                // Event of UART receving data
                // We'd better handler data event fast, there would be much more data events than
                // other types of events. If we take too much time on data event, the queue might be full.
                case UART_DATA:
                    uart_read_bytes(uart_x, dtmp, event.size, portMAX_DELAY);
					 char *p = (char *)dtmp;
					 int i = 0;
					 while (i < event.size)
					 {
						 i++;
						 com_rec_callback->drv_com_m_handle(*(p++));
						
					}
					
                    break;

                // Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(uart_x);
                    xQueueReset(uart0_queue);
                    break;

                // Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(uart_x);
                    xQueueReset(uart0_queue);
                    break;

                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;

                // Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;

                // Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}


/**
* @ Function Name : drv_com_init
* @ Author        : ygl
* @ Brief         : 端口控制器初始化
* @ Date          : 2018.11.18
* @ Input         : com_part_t com_x com号
*					int baud 波特率
*					void (*drv_com_x_handle)(unsigned char data) 串口接收回调函数指针
* @ Output		  : null
* @ Modify        : ...
**/
void drv_com_init(com_part_t com_x,int baud,void (*drv_com_x_handle)(unsigned char data))
{
	uart_port_t uart_x;
	
	// 串口配置
	uart_config_t uart_config = {
		.baud_rate = baud,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
	

	// 安装驱动
	if (com_x == COM_0){
		uart_x = UART_NUM_0;
		drv_com0_handle = drv_com_x_handle;
		uart_param_config(uart_x, &uart_config);
		uart_driver_install(uart_x, BUF_SIZE * 2, BUF_SIZE * 2, 100, &uart0_queue);
	}else{
		uart_x = UART_NUM_1;
		drv_com1_handle = drv_com_x_handle;
		uart_param_config(uart_x, &uart_config);
		uart_driver_install(uart_x, BUF_SIZE * 2, BUF_SIZE * 2, 100, &uart1_queue);
	}
  
 	// 接收的回调函数
	 com_rec_callback.com_x = com_x;
	 com_rec_callback.drv_com_m_handle = drv_com_x_handle;

	// 创建串口接收任务
	 xTaskCreate(uart_event_task, "uart_event_task", 4096, &com_rec_callback, 12, NULL);
}