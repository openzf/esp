#include "drv_gpio.h"
#include "driver/gpio.h"

void gpio_set_output()
{
	// 设置2号引脚为输出
	GPIO_SET_OUTPUT(2);
	// 设置电平为1
    GPIO_Set_Level(2, 1);
    	// 设置电平为1
    GPIO_Set_Level(2, 0);
}

void gpio_set_input()
{
	GPIO_Init_TypeDef *gpio_init_typedef;
	gpio_init_typedef.gpio_num = 2;
	gpio_init_typedef.mode = INPUT;
	gpio_init_typedef.pull_mode = PIN_PD; // PIN_PU 
	gpio_init(gpio_init_typedef);
}
