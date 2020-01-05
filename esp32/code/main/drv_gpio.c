#include "drv_gpio.h"

#include "driver/gpio.h"

static void m_gpio_init(GPIO_Init_TypeDef *gpio_init_typedef);
/**
* @ Function Name : gpio_init
* @ Author        : ygl
* @ Brief         : gpio初始化
* @ Date          : 2018.11.18
* @ Input         : GPIO_Init_TypeDef *gpio_init_typedef 串口配置的结构体指针
* @ Output		  : null
* @ Modify        : ...
**/
static void m_gpio_init(GPIO_Init_TypeDef *gpio_init_typedef)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;

    // 输入输出模式
    if (gpio_init_typedef->mode == INPUT){
        // 选择上拉下拉模式
        if (gpio_init_typedef->pull_mode == PIN_PD){
            io_conf.pull_down_en = 1;
            io_conf.pull_up_en = 0;
        }else{
            io_conf.pull_down_en = 0;
            io_conf.pull_up_en = 1;
        }

        io_conf.mode = GPIO_MODE_INPUT;
    }else{
        io_conf.pull_down_en = 0;
        io_conf.pull_up_en = 0;
        io_conf.mode = GPIO_MODE_OUTPUT;
    }
    
    // 选择引脚
    io_conf.pin_bit_mask = (1ULL<<gpio_init_typedef->gpio_num);
    gpio_config(&io_conf);
}

/**
* @ Function Name : GPIO_SET_OUTPUT
* @ Author        : ygl
* @ Brief         : gpio设置输出
* @ Date          : 2018.11.18
* @ Input         : unsigned char gpio_num IO号
* @ Output		  : null
* @ Modify        : ...
**/
void GPIO_SET_OUTPUT(unsigned char gpio_num)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = (1ULL<<gpio_num);
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

/**
* @ Function Name : GPIO_Set_Level
* @ Author        : ygl
* @ Brief         : gpio设置输出电平
* @ Date          : 2018.11.18
* @ Input         : unsigned char gpio_num IO号
                    unsigned char level 电平高低
* @ Output		  : null
* @ Modify        : ...
**/
void GPIO_Set_Level(unsigned char gpio_num,unsigned char level)
{
    gpio_set_level(gpio_num, level);
}


/**
* @ Function Name : GPIO_SET_INPUT
* @ Author        : ygl
* @ Brief         : gpio设置输入
* @ Date          : 2018.11.18
* @ Input         : unsigned char gpio_num IO号
* @ Output		  : null
* @ Modify        : ...
**/
void GPIO_SET_INPUT(unsigned char gpio_num,GPIO_PULL_MODE gpio_mode)
{

    GPIO_Init_TypeDef gpio_init_typedef;

    gpio_init_typedef.gpio_num =gpio_num;
    gpio_init_typedef.mode = INPUT;
    gpio_init_typedef.pull_mode = gpio_mode;
    if (gpio_mode ==  PIN_PD){
    // 下拉
     gpio_pulldown_en(gpio_num);
    }else{
    // 上拉
     gpio_pullup_en(gpio_num);
    }
    m_gpio_init(&gpio_init_typedef);
}

int GPIO_Get_Level(unsigned char gpio_num)
{
    return gpio_get_level(gpio_num);
}


// void GPIO_Set_ISR(unsigned char gpio_num, GPIO_PULL_MODE gpio_mode, INTR_MODE intr_mode,void (*gpio_isr_handler(void *arg))
// {
//     GPIO_SET_INPUT(gpio_num,gpio_mode);
//      // 中断类型
//     gpio_set_intr_type(gpio_num, intr_mode);
//     // 安装中断
//     gpio_install_isr_service(0);
//     // 安装中断服务
//     gpio_isr_handler_add(gpio_num, gpio_isr_handler, (void *) gpio_num);
// }