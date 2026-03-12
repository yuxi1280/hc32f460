#include "i2c.h"
#include "io.h"
#include "system_hc32f460.h"

void i2c_init(void)
{
    CM_PWC->FCG1 &= ~(1 << 4); //使能I2C1时钟

    bCM_I2C1 -> CR1_b.PE = 0; //禁用I2C1
    bCM_I2C1 -> CR1_b.SWRST = 1; //软件复位I2C1
    bCM_I2C1 -> CR1_b.SWRST = 0; //结束复位
    bCM_I2C1 -> CR1_b.PE = 1; //使能I2C1
    CM_I2C1 -> SLR0 = 0x00000000u; 
    CM_I2C1 -> SLR1 = 0x00000000u;
    CM_I2C1 -> CCR = 0x00000000u; 
    CM_I2C1 -> SR = 0x00000000u; 
}

void i2c_write(uint8_t addr, uint8_t *data, uint32_t len)
{

}



void i2c_read(uint8_t addr, uint8_t *data, uint32_t len)
{
    
}
