#include "system_hc32f460.h"
#include "io.h"
#include "rs485.h"
#include "encoder.h"
#include "ws2812.h"
#include "pwmled.h"
#include "lr.h"
#include "moto.h"
#include "pid_pos.h"
#include "soft_timer.h"
#include "ability.h"
#include "hot.h"
#include "i2c_slave.h"
#include "i2cmaster.h"


void _pid_update(void)
{
	static uint8_t step = 0;
    static uint64_t last_time = 0;

	voltage_overload();
	temperature_overload();



}

int main(void)
{
	stimer_t st;

	stimer_init();
	io_init();

	lr_init();
	lr_receive_init();
	////充电站
	ability_init();
	voltage_init();

	
	
	// //机器人
	// ability_robote_init();

	
	WDT_init();	

	st.id = 1;
	st.statu = 0;
	st.period = 100;
	st.handle = _pid_update;
	stimer_register(&st);
	stimer_ctrl(1, true, STIMER_STATU_POS_ENABLE | STIMER_STATU_POS_PERIOD);

	while (1)
	{
		// lr_nec(0xAA, 0xFF);
		// sys_delay_ms(3000);
		// lr_nec(0xAA, 0xAA);
		// sys_delay_ms(3000);
		// lr_nec(0xAA, 0x11);
		// sys_delay_ms(3000);
		// lr_nec(0xAA, 0x12);
		// sys_delay_ms(3000);
		// lr_nec(0xAA, 0x13);
		// sys_delay_ms(3000);


		
		lr_nec(0xAA, 0x99);
		sys_delay_ms(3000);
	}
	// CM_GPIO->PCRA1 = 0x0052u;//继电器1open机器人+0x0002
	// CM_GPIO->PCRA2 = 0x0052u;//继电器1close机器人+0x0004

	// CM_GPIO->PCRA3 = 0x0052u;//继电器2open机器人-充电站+0x0008
	// CM_GPIO->PCRA4 = 0x0052u;//继电器2close机器人-充电站+ 0x0010

	// CM_GPIO->PCRB6 = 0x0052u;//继电器open充电站-0x0040
	// CM_GPIO->PCRB5 = 0x0052u;//继电器close充电站-0x0020
	// while (1)
	// {
	// 	CM_GPIO->PORRB = 0x0060u;
	// 	CM_GPIO->PORRA = 0x0018u;
	// 	sys_delay_ms(500);
	// 	CM_GPIO->POSRB = 0x0040u;
	// 	CM_GPIO->POSRA = 0x0008u;
	// 	sys_delay_ms(50);
	// 	CM_GPIO->PORRB = 0x0040u;
	// 	CM_GPIO->PORRA = 0x0008u;
	// 	sys_delay_ms(500);
	// 	CM_GPIO->POSRB = 0x0020u;
	// 	CM_GPIO->POSRA = 0x0010u;
	// 	sys_delay_ms(50);
	// }
	
	


	for (;;) {

		WDT_flash();
		stimer_poll();
		//充电站
		// Relay();
		// inquire();
		/*机器人
		lr_robot();
		*/
	}
}
