#include "i2c_slave.h"
#include "io.h"
#include "system_hc32f460.h"
#include "ability.h"
#include "lr.h"


/*
标准模式最大 100Kbps，快速模式最大 400Kbps。
pclk3 50MHZ
CM_I2C1 -> CCR = //b31~b23 读出时为“0”,写入时写“0”
                 //b22~b19 读出时为“1”,写入时写“1”
                 //b18-b16 0 0 0：I2C基准时钟频率=PCLK3/1
                    0 0 1：I2C基准时钟频率=PCLK3/2
                    0 1 0：I2C基准时钟频率=PCLK3/4
                    0 1 1：I2C基准时钟频率=PCLK3/8
                    1 0 0：I2C基准时钟频率=PCLK3/16
                    1 0 1：I2C基准时钟频率=PCLK3/32
                    1 1 0：I2C基准时钟频率=PCLK3/64
                    1 1 1：I2C基准时钟频率=PCLK3/128
                //b15~b13 读出时为“1”,写入时写“1”
                //b12~b8 SHIGHW[4:0] 设定SCL高电平宽度位 设定SCL时钟的高电平宽度
                //b7~b5 读出时为“1”,写入时写“1”
                //b4~b0 SLOWW[4:0] 设定SCL低电平宽度位 设定SCL时钟的低电平宽度
*/

static volatile uint8_t slave_status = I2C_STATUS_IDLE;
static volatile uint8_t slave_cmd = 0xFF;
static volatile uint8_t slave_cmd_ready = 0;

static volatile uint8_t ir_comm_state = 0;  
static volatile uint64_t last_ir_heartbeat = 0;  
#define IR_HEARTBEAT_TIMEOUT 30000  



void i2c_slave_init(void)
{
    CM_PWC->FCG1 &= ~(1u << 4);

    bCM_I2C1->CR1_b.PE = 0;
    bCM_I2C1->CR1_b.SWRST = 1;
    bCM_I2C1->CR1_b.PE = 1;

    CM_I2C1->CLR = 0x00F050DFu;

    CM_I2C1->SLR0 = 0x10A0u;
    CM_I2C1->SLR1 = 0x00000000u;
    CM_I2C1->CCR = 0x000E2505u;
    CM_I2C1->FLTR = 0x00000003u;
    bCM_I2C1->CR1_b.ACK = 1;
    bCM_I2C1->CR1_b.SWRST = 0;
    CM_I2C1->CLR = 0x00F050DFu;
    bCM_I2C1->CR1_b.PE = 1;
    
    slave_status = I2C_STATUS_IDLE;
    slave_cmd = 0xFF;
    slave_cmd_ready = 0;
    ir_comm_state = 0;
    last_ir_heartbeat = sys_get_tick();

}



static uint8_t i2c_check_addr_match(void)
{
    if (CM_I2C1->SR & (1u << 1)) { // SLADDR0F位
        CM_I2C1->CLR = (1u << 1);
        return 1;
    }
    return 0;
}


static uint8_t i2c_check_stop_cond(void)
{
    if (CM_I2C1->SR & (1u << 4)) { // STOPF位
        CM_I2C1->CLR = (1u << 4);
        return 1;
    }
    return 0;
}

static uint8_t i2c_is_tx_mode(void)
{
    return (CM_I2C1->SR & (1u << 18)) ? 1 : 0; // TRA位
}

static uint8_t i2c_slave_recv_byte(void)
{
    uint32_t timeout = 0x10000u;
    
    while ((CM_I2C1->SR & (1u << 6)) == 0) { // RFULLF位
        if (--timeout == 0) {
            return 0xFF;
        }
    }
    
    return CM_I2C1->DRR;
}

static void i2c_slave_send_byte(uint8_t data)
{
    uint32_t timeout = 0x10000u;
    
    CM_I2C1->DTR = data;
    
    while ((CM_I2C1->SR & (1u << 3)) == 0) { // TENDF位
        if (--timeout == 0) {
            break;
        }
    }
    
    CM_I2C1->CLR = (1u << 3);
    (void)CM_I2C1->DRR; 
}

static volatile uint8_t status_changed = 0;

void i2c_slave_poll(void)
{
    // 检查地址匹配
     if (CM_I2C1->SR & (1u << 0)) { 
        CM_I2C1->CLR = (1u << 0);  
        
    }

     if (CM_I2C1->SR & (1u << 12)) {
        CM_I2C1->CLR = (1u << 12);
    }


    if (i2c_check_addr_match()) {
        // 判断主机是读还是写
        if (i2c_is_tx_mode()) {
            // 主机读：发送当前状态
            i2c_slave_send_byte(slave_status);
        } else {
            // 主机写：接收命令
            uint8_t cmd = i2c_slave_recv_byte();
            if (cmd != 0xFF) {
                slave_cmd = cmd;
                slave_cmd_ready = 1;
            }
        }
    }
    
    // 检查停止条件
    if (i2c_check_stop_cond()) {
        //I2C通讯结束，命令已接收 
        

    }
}

uint8_t i2c_slave_get_status_changed(void)
{
    if (status_changed) {
        status_changed = 0;
        return 1;
    }
    return 0;
}


// #define I2C_CMD_CHARGE_START    0x01    //到位
// #define I2C_CMD_CHARGE_COMPLETE 0x02    //充电完成
// #define I2C_CMD_IDLE            0x00    //空闲
// #define I2C_CMD_TEST            0x03  // 测试命令
// #define I2C_CMD_RESET           0x04  // 复位命令

// #define I2C_STATUS_NORMAL       0x10  // 红外通讯正常
// #define I2C_STATUS_INTERRUPT    0x11  // 通讯中断或重定位
// #define I2C_STATUS_IDLE         0x00  // 未进行通讯
// #define I2C_STATUS_TEST         0x03  // 测试状态
// #define I2C_STATUS_TEST_TRUE    0x55  //测试正常




void i2c_slave_ir_process(void)
{
    static uint64_t last_send_time = 0;
    lr_data_t ir_data;
    
    // 处理接收到的I2C命令
    if (slave_cmd_ready) {
        switch (slave_cmd) {
            case I2C_CMD_CHARGE_START:
                // 主机要求开始充电，发送"到位"信号给充电桩
                lr_nec(0xAA, 0x11);  
                ir_comm_state = 6;  
                CM_GPIO->PORRA = 0x001E;	
                CM_GPIO->POSRA = 0x000A;
                sys_delay_ms(10);
				CM_GPIO->PORRA = 0x001E;
                //slave_status = I2C_STATUS_IDLE; 
                break;

            case I2C_CMD_CHARGE_COMPLETE:
                // 主机要求发送充电完成信号
                lr_nec(0xAA, 0xAA);  
                ir_comm_state = 2;  
                ability_robote_init(); 
                //slave_status = I2C_STATUS_NORMAL;
                break;
                
            case I2C_CMD_IDLE:
                // 空闲命令
                ir_comm_state = 0;
                //slave_status = I2C_STATUS_IDLE;
                break;

            // case I2C_CMD_TEST:
            //     CM_GPIO->PORRA = 0x001E;
            //     CM_GPIO->POSRA = 0x0008;
            //     sys_delay_ms(10);
            //     CM_GPIO->PORRA = 0x001E;
            //     CM_GPIO->POSRA = 0x0010;
            //     sys_delay_ms(10);
            //     CM_GPIO->PORRA = 0x001E;
            //     slave_status = I2C_STATUS_TEST_TRUE;
            //     break;
                
            default:
                break;
        }
        
        slave_cmd_ready = 0;
    }
    
    if (lr_get_data(&ir_data)) {
        if (ir_data.addr == 0xAA) 
        {
            switch (ir_data.cmd) {
                case 0x13:  
                    // CM_GPIO->PORRA = 0x001E;	
                    // CM_GPIO->POSRA = 0x000A;
					// sys_delay_ms(10);
					// CM_GPIO->PORRA = 0x001E;
                    // last_ir_heartbeat = sys_get_tick();
                    ir_comm_state = 1;  // 充电中
                    //slave_status = I2C_STATUS_NORMAL;  // 通讯正常
                    break;  
                    
                case 0xFF: 
                    ir_comm_state = 3;  // 重定位
                    slave_status = I2C_STATUS_INTERRUPT;  // 通讯中断
                    break;
                    
                case 0x55:  // 充电桩询问
                    sys_delay_ms(100);
                    lr_nec(0xAA, 0x56);  // 回复
                    last_ir_heartbeat = sys_get_tick();
                    // if (ir_comm_state == 1) {
                    //     slave_status = I2C_STATUS_NORMAL;
                    // }
                    break;
                    
                default:
                    break;
            }
        }
    }
    uint64_t now = sys_get_tick();
    if(ir_comm_state == 1) {
        // 检查心跳超时
        if (now - last_ir_heartbeat >= IR_HEARTBEAT_TIMEOUT)
        {
            ir_comm_state = 3;
            slave_status = I2C_STATUS_INTERRUPT;
        }
    }
    
    // 重定位状态处理
    if (ir_comm_state == 3) {
        ability_robote_init(); 
        ir_comm_state = 0;
        slave_status = I2C_STATUS_IDLE; 
    }
}

void i2c_slave_set_status(uint8_t status)
{
    slave_status = status;
}

uint8_t i2c_slave_get_status(void)
{
    return slave_status;
}






























