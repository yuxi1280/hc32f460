#ifndef __LR_H_
#define __LR_H_

#include "system_hc32f460.h"

typedef struct {
	uint8_t addr;
	uint8_t cmd;
	uint8_t valid;
} lr_data_t;

void lr_init(void);
void lr_receive_init(void); 
void lr_nec( uint8_t addr, uint8_t cmd);//发射
uint8_t lr_nec_receive(uint8_t *addr, uint8_t *cmd);
uint8_t lr_get_data(lr_data_t *data);
void sys_delay_us(uint32_t us);
void Relay (void);
void inquire (void);
void lr_robot(void);
//lr_nec(0xAA, 0x11);  到位
//lr_nec(0xAA, 0x12);  校准
//lr_nec(0xAA, 0x13);  精确
//lr_nec(0xAA, 0xFF);  重定位
//lr_nec(0xAA, 0xAA);  完成

#endif