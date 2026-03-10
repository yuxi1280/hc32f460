#ifndef __PWMLED_H_
#define __PWMLED_H_

#include "system_hc32f460.h"

void pwmled_init(void);//初始化
void pwmled_set_pwm(uint16_t duty);//占空比
void pwmled_set_test(void);//循环
void pwmled_start(void);
void pwmled_stop(void);

#endif
