#include "lr.h"
#include "system_hc32f460.h"
#include <stdio.h>
#include "ws2812.h"
#include "ability.h"
//lr_nec(0xAA, 0x11);  到位
//lr_nec(0xAA, 0x12);  校准
//lr_nec(0xAA, 0x13);  精确
//lr_nec(0xAA, 0xFF);  重定位
//lr_nec(0xAA, 0xAA);  完成 
//lr_nec(0xAA, 0x55);  询问
//lr_nec(0xAA, 0x56);  询问回复
//lr_nec(0xAA, 0x99);  测试



#define lr_nec_tall() do{ CM_TMRA_4->CMPAR4 =  82; CM_TMRA_4->PCONR4 = 0x1043u; } while(0)//低位
#define lr_nec_low() do{  CM_TMRA_4->PCONR4 = 0x1243u; } while(0)//高位

#define HEARTBEAT_TIMEOUT_TICK 15000 
static volatile uint64_t last_heartbeat_time = 0;//上次心跳时间

static volatile uint8_t lr_rx_state = 0;//0:等待起始 1:等待起始低电平结束 2:等待数据位低电平结束 3:等待数据位高电平结束，解码机状态
static volatile uint32_t lr_rx_code = 0;//接收的完整32位数据
static volatile uint8_t lr_rx_bit_cnt = 0;//已接收的数据位数
static volatile uint8_t lr_rx_ready = 0;//接收完成标志
static volatile uint64_t lr_rx_last_time = 0;//上次捕获时间，用于计算脉冲宽度
static volatile uint8_t lr_rx_level = 1;//当前电平状态，1表示高电平，0表示低电平
static volatile uint8_t state=0;//状态机
//0空闲1在充电2充电完成3重定位
static  volatile uint8_t robote_state = 0;
//0空闲1在充电2充电完成3重定位
static volatile uint8_t last_receive_time = 0;


lr_data_t lr_data;

void sys_delay_us(uint32_t us)
{
    uint64_t now = sys_get_tick();
    uint32_t ticks = (us + 50) / 100;
    
    for (;;) {
		if ((sys_get_tick() - now) >= ticks) {
			break;
		}
}
}

void lr_init(void)//红外发射初始化
{
    CM_PWC->FCG2 &= ~(1 << 5); 
    CM_TMRA_4->BCSTRL = 0x42u;
    CM_TMRA_4->BCSTRH = 0x00u;
    CM_TMRA_4->CNTER = 0u;
    CM_TMRA_4->PERAR = 164u;

    CM_TMRA_4->CMPAR4 = 0u;
    CM_TMRA_4->PCONR4 = 0x1343u;
	// CM_TMRA_4->CMPAR8 = 0u;
    // CM_TMRA_4->PCONR8 = 0x1043u;
    bCM_TMRA_4->BCSTRL_b.START = 1u;    
}

void lr_receive_init(void)//红外接收初始化
{
    CM_PWC->FCG2 &= ~(1 << 5);
	
	CM_TMRA_4->BCSTRL = 0x42u;  
	CM_TMRA_4->BCSTRH = 0x00u;
	CM_TMRA_4->CNTER = 0u;
	CM_TMRA_4->PERAR = 0xffffu;  

	CM_TMRA_4->CCONR3 = 0x4031u;  
	CM_TMRA_4->CMPAR3 = 0u;
	
	CM_TMRA_4->ICONR |= (1 << 2); 
	
	CM_INTC->SEL84 = INT_SRC_TMRA_4_CMP; 
	NVIC_SetPriority(INT084_IRQn, 1);
	NVIC_EnableIRQ(INT084_IRQn);
	
	bCM_TMRA_4->BCSTRL_b.START = 1u;
	
	lr_rx_state = 0;
	lr_rx_code = 0;
	lr_rx_bit_cnt = 0;
	lr_rx_ready = 0;
	lr_rx_last_time = 0;
	lr_rx_level = 0;
	state = 0;

	last_heartbeat_time = sys_get_tick();
    
}

//红外发射
void lr_nec( uint8_t addr, uint8_t cmd)
{
    uint32_t code;
    uint8_t i;
    code = (uint32_t)addr | ((uint32_t)(uint8_t)~addr << 8) | ((uint32_t)cmd << 16) | ((uint32_t)(uint8_t)~cmd << 24);
    //code = addr | ((~addr) << 8) | (cmd << 16) | ((~cmd) << 24);
    lr_nec_tall();
    sys_delay_us(9000);
    lr_nec_low();
    sys_delay_us(4000); 
    for (i = 0; i < 32; i++)
    {
        lr_nec_tall();
        sys_delay_us(560);
        lr_nec_low();
        if (code & (1u << i)) {
            sys_delay_us(1690);
        } else {
            sys_delay_us(560);
        }
        
    }
    
    lr_nec_tall();
    sys_delay_us(560);
    lr_nec_low();
    
}




//红外接收中断处理函数
void IRQ084_Handler(void)
{
	uint64_t now;
	uint32_t pulse_width;
	
	// 检查捕获中断标志
	if (CM_TMRA_4->STFLR & (1 << 2)) {
		// 清除中断标志
		CM_TMRA_4->STFLR &= ~(1 << 2);
		
		now = sys_get_tick();
		
		if (lr_rx_last_time > 0) {
			pulse_width = (uint32_t)(now - lr_rx_last_time);
		} else {
			pulse_width = 0;
		}
		lr_rx_last_time = now;
		
		if (pulse_width > 200) {
			lr_rx_state = 0;
			lr_rx_bit_cnt = 0;
			lr_rx_code = 0;
		}
		
		// 切换电平状态
		lr_rx_level = !lr_rx_level;
		
		// NEC协议解码状态机
		if (lr_rx_level == 0) {
			switch (lr_rx_state) {
				case 0://引导码9MS
					if (pulse_width >= 60 && pulse_width <= 120) {
						lr_rx_state = 1;
						lr_rx_bit_cnt = 0;
						lr_rx_code = 0;
					}
					break;
				case 2://引导码4.5MS
					if (pulse_width >= 3 && pulse_width <= 10) {
						lr_rx_state = 3;
					} else {
						lr_rx_state = 0;
					}
					break;
			}
		} else {
			switch (lr_rx_state) {
				case 1:
					if (pulse_width >= 25 && pulse_width <= 60) {
						lr_rx_state = 2;
					} else {
						lr_rx_state = 0;
					}
					break;
				case 3:
					if (pulse_width >= 3 && pulse_width <= 10) {
						lr_rx_code >>= 1;
					} else if (pulse_width >= 12 && pulse_width <= 25) {
						lr_rx_code = (lr_rx_code >> 1) | 0x80000000;
					} else {
						lr_rx_state = 0;
						break;
					}
					
					lr_rx_bit_cnt++;
					
					if (lr_rx_bit_cnt >= 32) {
						lr_rx_ready = 1;
						lr_rx_state = 0;
					} else {
						lr_rx_state = 2;
					}
					break;
			}
		}
	}
}

//红外接收
uint8_t lr_get_data(lr_data_t *data)
{
	uint8_t addr, addr_inv, cmd, cmd_inv;
	
	if (!lr_rx_ready) {
		return 0;
	}
	
	addr = lr_rx_code & 0xff;
	addr_inv = (lr_rx_code >> 8) & 0xff;
	cmd = (lr_rx_code >> 16) & 0xff;
	cmd_inv = (lr_rx_code >> 24) & 0xff;
	
	// 校验
	if ((addr + addr_inv == 0xff) && (cmd + cmd_inv == 0xff)) {
		data->addr = addr;
		data->cmd = cmd;
		data->valid = 1;
		lr_rx_ready = 0;
		return 1;
	}
	
	// 校验失败
	lr_rx_ready = 0;
	return 0;
}
//lr_nec(0xAA, 0x11);  到位
//lr_nec(0xAA, 0x12);  校准
//lr_nec(0xAA, 0x13);  精确
//lr_nec(0xAA, 0xFF);  重定位
//lr_nec(0xAA, 0xAA);  完成 
//lr_nec(0xAA, 0x55);  询问
//lr_nec(0xAA, 0x56);  询问回复
// CM_GPIO->PCRA1 = 0x0053u;//继电器1open+    0x0002
// CM_GPIO->PCRA2 = 0x0053u;//继电器1close+   0x0004
// CM_GPIO->PCRA3 = 0x0053u;//继电器2open-    0x0008
// CM_GPIO->PCRA4 = 0x0053u;//继电器2close-   0x0010
//CM_GPIO->PCRB6 = 0x0052u;//继电器open充电站- 0x0040
//CM_GPIO->PCRB5 = 0x0052u;//继电器close充电站-0x0020

//充电站端
void Relay (void)
{
    if (lr_get_data(&lr_data)) {
        if (lr_data.addr == 0xAA)
        {
            switch (lr_data.cmd)
            {
                case 0x11:
                    //到位
                    
					CM_GPIO->PORRB = 0x0060u;
					CM_GPIO->PORRA = 0x0018u;
					sys_delay_ms(10); 
					CM_GPIO->POSRB = 0X0040U;
					sys_delay_ms(10);
					CM_GPIO->POSRA = 0x0008u;
					sys_delay_ms(10);
					CM_GPIO->PORRB = 0x0060u;
					CM_GPIO->PORRA = 0x0018u;
					sys_delay_ms(100);
					lr_nec(0xAA, 0x12);
					last_heartbeat_time = sys_get_tick();
					state = 1;
                    break;


				case 0xFF:
					state = 3;
					break;
                case 0xAA:
                    //完成
                    ability_init();
					sys_delay_ms(100);
					state = 2;
                    break;
				case 0x56:
					if (state == 1)
					{
						last_heartbeat_time = sys_get_tick();
					}
					
					break;
            }
        }	
        
    } 
    
}

void inquire (void)
	{
		static uint64_t last_send_time = 0;
		uint64_t now = sys_get_tick();
		
		if (state == 1)
		{
			if (now - last_send_time >= 10000)
			{
				lr_nec(0xAA, 0x55);
				last_send_time = now;
			}
			if (now - last_heartbeat_time >= HEARTBEAT_TIMEOUT_TICK)
			{
				state = 3;
			}
			
		}
		else if (state == 2)
		{
			NVIC_SystemReset();
		}
		else if (state == 3)
		{
			lr_nec(0xAA, 0xFF);
			//CM_GPIO->POSRA = 0x0040;
			ability_init();
            sys_delay_ms(10);
            //CM_GPIO->PORRA = 0x0040;
			lr_init();
			lr_receive_init();
			state = 0;
		}
	}








