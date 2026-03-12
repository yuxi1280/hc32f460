#include "ability.h"
#include "lr.h"
#include "system_hc32f460.h"


// //红外发射
// 	CM_GPIO->PFSRB9 = 4u; //TIMA_4_PWM4
// 	CM_GPIO->PCRB9 = 0x0050u;
// 	//红外接收
// 	CM_GPIO->PFSRB8 = 4u; //TIMA_4_PWM3
// 	CM_GPIO->PCRB8 = 0x0040u;
// CM_GPIO->PCRA1 = 0x0053u;//继电器1open+    0x0002
// CM_GPIO->PCRA2 = 0x0053u;//继电器1close+   0x0004
// CM_GPIO->PCRA3 = 0x0053u;//继电器2open-    0x0008
// CM_GPIO->PCRA4 = 0x0053u;//继电器2close-   0x0010
// 	//散热风扇
// 	CM_GPIO->PCRC13 = 0x0050u;
// 	CM_GPIO->PFSRC13 = 4u; //TIMA_4_PWM8
// 	//蜂鸣器
// 	CM_GPIO->PCRA6 = 0x0052u;
// 	//温度采集
// 	CM_GPIO->PCRB6 = 0x0040u;
// 	CM_GPIO->PFSRB6 = 

// 	//电压采样
// 	CM_GPIO->PCRA0 = 0x0040u;

void ability_init(void)
{
    CM_GPIO->PORRA =
        0x0004u | 
        0x0010u | 
        0x0008u | 
        0x0040u |
        0x0002u;  
}


void voltage_init(void)//电压采样
{
    CM_PWC->FCG3 &= ~(1 << 0);
    CM_ADC1->STR = 0x00u;
    sys_delay_ms(10);
    CM_ADC1->CR0 = 0x04c0u;
    CM_ADC1->CR1 = 0x0000u;
    CM_ADC1->TRGSR = 0x0000u;
    CM_ADC1->CHSELRA = 0x00000001u;
    CM_ADC1->AVCHSELR = 0x00000001u;
    CM_ADC1->SSTR0 = 0xffu;
    CM_ADC1->PGACR = 0x0000u;
}

uint16_t voltage_get_adc(void)
{
    CM_ADC1->STR = 0x01u;
    while ((CM_ADC1->ISR & 0x01u) == 0);
    uint16_t res = (uint16_t)CM_ADC1->DR0;
    CM_ADC1->ISR = 0x01u;
    return res;
}

float voltage_get()
{
    uint16_t adc_value = voltage_get_adc();
    return adc_value * 3.3f / 4095.0f / 0.1282; 
}

void voltage_overload(void)
{
    float voltage =voltage_get();
    if (voltage > 420.0f)//过压约26V
    {
        CM_GPIO->PORRA = 0x0004 | 0x0010 | 0x0008 | 0x0002;
        CM_GPIO->POSRA = 0x0040;
    }
    else if (voltage < 377.0f)//欠压19V
    {
        CM_GPIO->PORRA = 0x0004 | 0x0010 | 0x0008 | 0x0002;
        CM_GPIO->POSRA = 0x0040;
    }
    else if (voltage >= 377.0f && voltage <= 420.0f)
    {
        CM_GPIO->PORRA = 0x0040;
    }
}

