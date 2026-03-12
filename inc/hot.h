#ifndef __HOT_H_
#define __HOT_H_

#include "system_hc32f460.h"

void temperature_into(void);
void pwmfly_init(void);
void pwmfly_set_pwm(uint16_t duty);
uint16_t temperature_get(void);
void temperature_overload(void);


#endif