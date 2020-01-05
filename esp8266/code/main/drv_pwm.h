#ifndef __DRV_PWM_H__
#define __DRV_PWM_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
 {
    PWM_PIN_0 = 12,
    PWM_PIN_1 = 13,
    PWM_PIN_2 = 14,
    PWM_PIN_3 = 15,
}PWM_PIN;

void drv_pwm_init(PWM_PIN pwm_pin,unsigned int period,unsigned int duty);
void drv_pwm_set_duty(PWM_PIN pwm_pin, unsigned int duty);

#ifdef __cplusplus
}
#endif
#endif