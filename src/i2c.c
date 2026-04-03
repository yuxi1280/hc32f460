#include "i2c.h"
#include "io.h"
#include "system_hc32f460.h"
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



void i2c_init(void) 
{
    CM_PWC->FCG1 &= ~(1u << 4); 
    
    CM_I2C1 -> CR1 = 0x00000040u;
    bCM_I2C1 -> CR1_b.PE = 0;
    bCM_I2C1 -> CR1_b.SWRST = 1;
    bCM_I2C1 -> CR1_b.PE = 1;
    
    CM_I2C1 -> SLR0 = 0x000000A1u; // 从机地址0x50, 7位地址, 使能
    CM_I2C1 -> SLR1 = 0x00000000u;
    CM_I2C1 -> CCR = 0x000E2505u;  // 时钟配置约400kHz
    CM_I2C1 -> FLTR = 0x00000003u; // 数字滤波器
    CM_I2C1 -> CR1 = 0x00000044u;  // 使能I2C

    bCM_I2C1 -> CR1_b.SWRST = 0;
}

uint8_t i2c_read_byte(void) // 读取一个字节
{
    uint32_t timeout = 0x10000u;
    
    while ((CM_I2C1->SR & (1u << 6)) == 0) { // 等待接收满
        if (--timeout == 0) return 0xFF;
    }
    
    return CM_I2C1 -> DRR;
}

void i2c_write_byte(uint8_t data) // 写入一个字节
{
    uint32_t timeout = 0x10000u;
    
    CM_I2C1 -> DTR = data;
    
    while ((CM_I2C1 -> SR & (1u << 3)) == 0) { 
        if (--timeout == 0) break;
    }
    
    CM_I2C1->CLR = (1u << 3); 
}
    
uint8_t i2c_check_addr(void) // 检查地址匹配
{
    if (CM_I2C1 -> SR & (1u << 1)) { 
        CM_I2C1 -> CLR = (1u << 1); 
        return 1;
    }
    return 0;
}

uint8_t i2c_check_stop(void) // 检查停止条件
{   
    if (CM_I2C1->SR & (1u << 4)) { 
        CM_I2C1->CLR = (1u << 4); 
        return 1;
    }
    return 0;
}

uint8_t i2c_is_tx_mode(void) // 检查是否为发送模式
{
    return (CM_I2C1->SR & (1u << 18)) ? 1 : 0;

}

void i2c_enable(void) 
{
    bCM_I2C1->CR1_b.PE = 1;
}

void i2c_disable(void) 
{
    bCM_I2C1->CR1_b.PE = 0;
}


void i2c_waite(void)
{
    static volatile uint32_t hel = 0;
    static volatile uint16_t low = 0 ;
    

}


































