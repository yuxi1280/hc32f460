#include "system_hc32f460.h"
#include "lr.h"
#include "lr_robot.h"

lr_data_t lr_data;
static  volatile uint8_t robote_state = 0;
//0空闲1准备充电2心跳进行

// CM_GPIO->PCRA1 = 0x0053u;//继电器1open+    0x0002
// CM_GPIO->PCRA2 = 0x0053u;//继电器1close+   0x0004
// CM_GPIO->PCRA3 = 0x0053u;//继电器2open-    0x0008
// CM_GPIO->PCRA4 = 0x0053u;//继电器2close-   0x0010
//lr_nec(0xAA, 0x11);  到位
//lr_nec(0xAA, 0x12);  校准
//lr_nec(0xAA, 0x13);  精确
//lr_nec(0xAA, 0xFF);  重定位
//lr_nec(0xAA, 0xAA);  完成 
//lr_nec(0xAA, 0x55);  询问
//lr_nec(0xAA, 0x56);  询问回复

void lr_robot_init(void)
{
    CM_GPIO->PORRA =
        0x0004u | 
        0x0010u | 
        0x0008u | 
        0x0002u;
}

void lr_robot(void)
{
	if (lr_get_data(&lr_data)) {
		if (lr_data.addr == 0xAA)
		{
			switch (lr_data.cmd)
			{
				case 0x12:
                CM_GPIO->POSRA = 0x0002 | 0x0008;
				lr_nec(0xAA, 0x13);
				robote_state = 1;
				break;
				case 0x55:
				lr_nec(0xAA, 0x56);
                robote_state = 2;
				break;
			}
		}
		
	}
}

void Relay_robot(void)
{
    if (robote_state == 1)
    {
        sys_delay_ms(10);
        CM_GPIO->PORRA = 0x0002 | 0x0008;
    }
    else if (robote_state == 0)
    {
        lr_robot_init();
    }
    else if (robote_state == 2)
    {
        
    }
}   