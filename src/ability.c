#include "ability.h"
#include "lr.h"
#include "system_hc32f460.h"

	// CM_GPIO->PCRA1 = 0x0052u;//继电器1open机器人+0x0002
	// CM_GPIO->PCRA2 = 0x0052u;//继电器1close机器人+0x0004

	// CM_GPIO->PCRA3 = 0x0052u;//继电器2open机器人-充电站+0x0008
	// CM_GPIO->PCRA4 = 0x0052u;//继电器2close机器人-充电站+ 0x0010

	// CM_GPIO->PCRB6 = 0x0052u;//继电器open充电站-0x0040
	// CM_GPIO->PCRB5 = 0x0052u;//继电器close充电站-0x0020
void ability_init(void)
{
    CM_GPIO->PORRA = 0x0058u;
    CM_GPIO->PORRB = 0x0060u;
    sys_delay_ms(10);
    CM_GPIO->POSRB = 0x0020u;
    CM_GPIO->POSRA = 0x0010u;
    sys_delay_ms(10);
    CM_GPIO->PORRB = 0x0020u;
    CM_GPIO->PORRA = 0x0010u;
}
void ability_robote_init(void)
{
    CM_GPIO->PORRA = 0x001Eu; 
    sys_delay_ms(10); 
    CM_GPIO->POSRA = 0x0004 | 0x0010;
    sys_delay_ms(10);
    CM_GPIO->PORRA = 0x001Eu; 
}

void voltage_init(void)//电压采样
{
    CM_PWC->FCG3 &= ~(1 << 0);
    CM_ADC1->STR = 0x00u;
    sys_delay_ms(10);
    CM_ADC1->CR0 = 0x04c0u;
    CM_ADC1->CR1 = 0x0000u;
    CM_ADC1->TRGSR = 0x0000u;
    CM_ADC1->CHSELRA = 0x00000120u;
    CM_ADC1->AVCHSELR = 0x00000120u;
    CM_ADC1->SSTR5 = 0xffu; 
    CM_ADC1->SSTR8 = 0xffu;
    CM_ADC1->PGACR = 0x0000u;
}

uint16_t voltage_get_adc(void)
{
    CM_ADC1->STR = 0x01u;
    while ((CM_ADC1->ISR & 0x01u) == 0);
    uint16_t res = (uint16_t)CM_ADC1->DR8;
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
    if (voltage > 420.0f)//过压
    {
        CM_GPIO->PORRA = 0x0004 | 0x0002;
        CM_GPIO->PORRB = 0x0020 | 0x0040u;
        CM_GPIO->POSRA = 0x0040;
    }
    else if (voltage < 400.0f)//欠压
    {
        CM_GPIO->PORRA = 0x0004 | 0x0002;
        CM_GPIO->PORRB = 0x0020 | 0x0040u;
        CM_GPIO->POSRA = 0x0040;
    }
    else if (voltage >= 400.0f && voltage <= 420.0f)
    {
        CM_GPIO->PORRA = 0x0040;
    }
}

