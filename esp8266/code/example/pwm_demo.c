#include "drv_pwm.h"
#include "driver/pwm.h"
// ------------------pwm 测试 -------
static void pwm_task()
{
    int duty = 0;
    int flag = 0;
    drv_pwm_init(PWM_PIN_0, 500, 0);
    while(1){
        if (flag == 0){
            duty++;
        }
        else{
            duty--;
        }

        if (duty>100){
            flag = 1;
        }
        if (duty<0){
            flag = 0;
        }
        drv_pwm_set_duty(PWM_PIN_0,duty);
        vTaskDelay(10 /portTICK_RATE_MS);
    }
}