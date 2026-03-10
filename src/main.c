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

void _pid_update(void)
{
	static uint8_t step = 0;
    static uint64_t last_time = 0;
	voltage_overload();

}

int main(void)
{
	stimer_t st;

	stimer_init();
	io_init();
	ws2812_init();
	ability_init();
	voltage_init();
	WDT_init();	
	//ds18b20_init();
	//pwmled_init();
	lr_init();
	lr_receive_init();
	pwmfly_init();

	st.id = 1;
	st.statu = 0;
	st.period = 100;
	st.handle = _pid_update;
	stimer_register(&st);
	stimer_ctrl(1, true, STIMER_STATU_POS_ENABLE | STIMER_STATU_POS_PERIOD);

	// while (1)
	// {
	// 	lr_nec(0xAA, 0xFF);
	// 	sys_delay_ms(3000);
	// 	lr_nec(0xAA, 0xAA);
	// 	sys_delay_ms(3000);
	// 	lr_nec(0xAA, 0x11);
	// 	sys_delay_ms(3000);
	// 	lr_nec(0xAA, 0x12);
	// 	sys_delay_ms(3000);
	// 	lr_nec(0xAA, 0x13);
	// 	sys_delay_ms(3000);
	// }
	


	for (;;) {

		WDT_flash();
		stimer_poll();
		Relay();//充电站
		inquire();//询问
		//lr_robot();
	}
}
