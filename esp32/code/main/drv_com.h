#ifndef __DRV_COM_H__
#define __DRV_COM_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DRV_USART_BUFFER_LEN 40
#define BUF_SIZE (1024)

/**
 * @brief UART peripheral number
 */
typedef enum
 {
    COM_0 = 0x0,
    COM_1 = 0x1,
}com_part_t;

typedef struct Drv_com_rec_callback_TypeDef_TAG
{
	com_part_t com_x;
	void (*drv_com_m_handle)(unsigned char data);
}Drv_com_rec_callback_TypeDef;

void drv_com_init(com_part_t com_x, int baud, void (*drv_com_x_handle)(unsigned char data));
void drv_com0_write_bytes(char *data, int len);
void drv_com1_write_bytes(char *data, int len);
void drv_com0_printf(char *fmt, ...);
void drv_com1_printf(char *fmt, ...);

#ifdef __cplusplus
}
#endif
#endif