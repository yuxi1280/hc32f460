#include "i2cmaster.h"
#include "io.h"
#include "system_hc32f460.h"
#include "lr.h"



#define I2C_MASTER_TIMEOUT         (0x10000u)
#define I2C1_CCR_400K_PCLK3_DIV64   (0x007EE5E5u)
#define I2C1_FLTR_DIG_4CLK           (0x00000013u)

void i2c_master_init(void)
{
    CM_PWC->FCG1 &= ~(1u << 4); 

    CM_I2C1->CR1 = 0x00000040u;
    bCM_I2C1->CR1_b.PE = 0;
    bCM_I2C1->CR1_b.SWRST = 1;
    bCM_I2C1->CR1_b.PE = 1;

    CM_I2C1->SLR0 = 0x00000000u;
    CM_I2C1->SLR1 = 0x00000000u;
    CM_I2C1->CCR = I2C1_CCR_400K_PCLK3_DIV64;
    CM_I2C1->FLTR = I2C1_FLTR_DIG_4CLK;

    bCM_I2C1->CR1_b.SWRST = 0;
    bCM_I2C1->CR1_b.PE = 1;
}

void i2c_master_start(void)
{
    uint32_t t = I2C_MASTER_TIMEOUT;

    bCM_I2C1->CR1_b.PE = 1;
    bCM_I2C1->CR1_b.START = 1;

    while ((CM_I2C1->SR & (1u << 0)) == 0) 
    {
        if (--t == 0) {
            break;
        }
    }

    CM_I2C1->CLR = (1u << 0);
}

void i2c_master_stop(void)
{
    uint32_t t = I2C_MASTER_TIMEOUT;

    bCM_I2C1->CR1_b.STOP = 1;

    while ((CM_I2C1->SR & (1u << 4)) == 0) 
    {
        if (--t == 0) {
            break;
        }
    }

    CM_I2C1->CLR = (1u << 4);
}

uint8_t i2c_master_send_addr(uint8_t addr, uint8_t rw)
{
    uint32_t t = I2C_MASTER_TIMEOUT;

    CM_I2C1->DTR = (uint8_t)((addr << 1) | (rw & 1u));

    while ((CM_I2C1->SR & (1u << 3)) == 0) 
    {
        if (--t == 0) {
            return 0;
        }
    }

    CM_I2C1->CLR = (1u << 3);

    if (CM_I2C1->SR & (1u << 12)) 
    {
        CM_I2C1->CLR = (1u << 12);
        return 0;
    }

    return 1;
}

uint8_t i2c_master_write_byte(uint8_t data)
{
    uint32_t t = I2C_MASTER_TIMEOUT;

    CM_I2C1->DTR = data;

    while ((CM_I2C1->SR & (1u << 3)) == 0)
    {
        if (--t == 0) {
            return 0;
        }
    }

    CM_I2C1->CLR = (1u << 3);

    if (CM_I2C1->SR & (1u << 12)) 
    {
        CM_I2C1->CLR = (1u << 12);
        return 0;
    }

    return 1;
}

uint8_t i2c_master_read_byte(uint8_t ack)
{
    uint32_t t = I2C_MASTER_TIMEOUT;
    uint8_t data;

    if (ack) 
    {
        bCM_I2C1->CR1_b.ACK = 0; /* 发 ACK */
    } 
    else {
        bCM_I2C1->CR1_b.ACK = 1; /* 发 NACK */
    }

    while ((CM_I2C1->SR & (1u << 6)) == 0) 
    {
        if (--t == 0) {
            return 0xFF;
        }
    }

    data = (uint8_t)CM_I2C1->DRR;
    return data;
}

uint8_t i2c_master_check_busy(void)
{
    return (CM_I2C1->SR & (1u << 17)) ? 1u : 0u;
}