#ifndef __ABILITY_H_
#define __ABILITY_H_

#include "system_hc32f460.h"

void ability_init(void);
void voltage_init(void);
uint16_t voltage_get_adc(void);
void voltage_overload(void);
float voltage_get();
void pwmfly_set_pwm(uint16_t duty);
void ability_robote_init(void);

#endif