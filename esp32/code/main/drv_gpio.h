#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
 {
    INPUT = 0x0,
    OUTPUT = 0x1,
}GPIO_MODE;
typedef enum
 {
    PIN_PD = 0x0,
    PIN_PU = 0x1,
}GPIO_PULL_MODE;

typedef struct GPIO_Init_TypeDef_TAG
{
    char gpio_num;
    GPIO_MODE mode;
    GPIO_PULL_MODE pull_mode;
}GPIO_Init_TypeDef;


void GPIO_SET_OUTPUT(unsigned char gpio_num);
void GPIO_Set_Level(unsigned char gpio_num, unsigned char level);

void GPIO_SET_INPUT(unsigned char gpio_num, GPIO_PULL_MODE gpio_mode);
int GPIO_Get_Level(unsigned char gpio_num);

#ifdef __cplusplus
}
#endif
#endif