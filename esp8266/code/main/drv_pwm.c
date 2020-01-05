#include "drv_pwm.h"

#include "driver/pwm.h"


int16_t phase[1] = {
    0, 
};

uint32_t  G_0_period = 0;
uint32_t  G_1_period = 0;
uint32_t  G_2_period = 0;
uint32_t  G_3_period = 0;

/**
* @ Function Name :  drv_pwm_init
* @ Author        : ygl
* @ Brief         : PWM初始化
* @ Date          : 2018.11.18
* @ Input         : PWM_PIN pwm_pin pwm的引脚号
                    uint32_t  period 周期
                    uint32_t  duty 占空比
* @ Output		  : null
* @ Modify        : ...
**/
void drv_pwm_init(PWM_PIN pwm_pin,uint32_t  period,uint32_t  duty)
{
    uint32_t  temp_duty = 0;
    // 周期单位是1us 
    if (duty > 100){
        duty = 100;
    }
    if (duty == 0){
        temp_duty = 0;
    }else{
        temp_duty = period*duty/100.0;
    }
   
     switch (pwm_pin)
    {
    case PWM_PIN_0:
        G_0_period = period;
        break;
    case PWM_PIN_1:
       G_1_period = period;
        break;
    case PWM_PIN_2:
        G_2_period = period;
        break;
    case PWM_PIN_3:
        G_3_period = period;
        break;
    default:
        return;
        break;
    }
    pwm_init(period,&temp_duty , 1, &pwm_pin);
    pwm_set_channel_invert(0x1 << 0);
    pwm_set_phases(phase);
    pwm_start();
}

/**
* @ Function Name : drv_pwm_set_duty
* @ Author        : ygl
* @ Brief         : PWM设置占空比
* @ Date          : 2018.11.18
* @ Input         : PWM_PIN pwm_pin pwm的引脚号
                    uint32_t  duty 占空比
* @ Output		  : null
* @ Modify        : ...
**/
void drv_pwm_set_duty(PWM_PIN pwm_pin,uint32_t  duty)
{
    int pwm_temp_pin = 0;
    int temp_period = 0;
    uint32_t  temp_duty = 0;

    switch (pwm_pin)
    {
    case PWM_PIN_0:
        pwm_temp_pin = 0;
        temp_period = G_0_period;
        break;
    case PWM_PIN_1:
        pwm_temp_pin = 1;
        temp_period = G_1_period;
        break;
    case PWM_PIN_2:
        pwm_temp_pin = 2;
        temp_period = G_2_period;
        break;
    case PWM_PIN_3:
        pwm_temp_pin = 3;
        temp_period = G_3_period;
        break;
    default:
        return;
        break;
    }

        // 周期单位是1us 
    if (duty > 100){
        duty = 100;
    }
    if (duty == 0){
        temp_duty = 0;
    }else{
        temp_duty = temp_period*duty/100.0;
    }
   
    pwm_set_duty(pwm_temp_pin, temp_duty);
    pwm_start();
}






