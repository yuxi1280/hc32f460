#include "hot.h"
#include "io.h"
#include "system_hc32f460.h"


void pwmfly_init(void)//风扇
{
    CM_PWC->FCG2 &= ~(1 << 5); 
    CM_TMRA_4->BCSTRL = 0x52u;
    CM_TMRA_4->BCSTRH = 0x00u;
    CM_TMRA_4->CNTER = 0u;
    CM_TMRA_4->PERAR = 125u;

    CM_TMRA_4->CMPAR4 = 0u;
    CM_TMRA_4->PCONR4 = 0x1043u;
    bCM_TMRA_4->BCSTRL_b.START = 1u;    
}

void pwmfly_set_pwm(uint16_t duty)
{
    if (duty <= 0){
        CM_TMRA_4->PCONR8 = 0x1243u;
    } else if (duty >= 164){
        CM_TMRA_4->PCONR8 = 0x1343u;
    } else {
        CM_TMRA_4->CMPAR8 = duty;
        CM_TMRA_4->PCONR8 = 0x1043u;
    }
    
}

uint16_t temperature_get(void)//温度
{
    CM_ADC1->STR = 0x01u;
    while ((CM_ADC1->ISR & 0x01u) == 0);
    uint16_t res = (uint16_t)CM_ADC1->DR5;
    CM_ADC1->ISR = 0x01u;
    return res;
}

//float temperature_get()

void temperature_overload(void)
{
    float hot = temperature_get();
    if ( hot < 726.0 )//约60°
    {
        pwmfly_set_pwm(164);
    }
    else if (hot < 1369.0 && hot >= 726.0)//约40°-60
    {
        pwmfly_set_pwm(100);
    }
    else
    {
        pwmfly_set_pwm(60);
    }
    
}