#include "pwmled.h"

static int16_t pwm_duty = 0;//当前值
static int8_t pwm_step = 0;//步

void pwmled_init(void)
{
     // CM_PWC->FCG2 &= ~(1 << 7);
    CM_TMRA_6->BCSTRL = 0x42u;
    CM_TMRA_6->BCSTRH = 0x00u;
    CM_TMRA_6->CNTER = 0u;
    // CM_TMRA_6->PERAR = 284u;

    CM_TMRA_6->CMPAR3 = 0u;
    CM_TMRA_6->PCONR3 = 0x1043u;
    bCM_TMRA_6->BCSTRL_b.START = 1u;    
    pwm_duty = 0;
    pwm_step = 1;
}

void pwmled_set_test(void)
{
        pwm_duty = pwm_duty + pwm_step;
        if (pwm_duty >= 284) {
            pwm_step = -1;
            pwm_duty = 284;
        } else if (pwm_duty <= 0) {
            pwm_step = 1;
            pwm_duty = 0;
        }
        pwmled_set_pwm(pwm_duty);
}

void pwmled_set_stop(void)
{
    pwmled_set_pwm(0);
}


void pwmled_set_pwm(uint16_t duty)
{
    if (duty <= 0){
        CM_TMRA_6->PCONR3 = 0x1243;
    } else if (duty >= 284){
        CM_TMRA_6->PCONR3 = 0x1343;
    } else {
        CM_TMRA_6->CMPAR3 = duty;
        CM_TMRA_6->PCONR3 = 0x1043;
    }
    
}