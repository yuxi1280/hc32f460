#include "i2cmaster.h"
#include "io.h"
#include "system_hc32f460.h"
#include "lr.h"



void i2c_master_init(void)
{
    CM_PWC->FCG1 &= ~(1u << 4);
    
    bCM_I2C1->CR1_b.PE = 0;
    bCM_I2C1->CR1_b.SWRST = 1;
    bCM_I2C1->CR1_b.PE = 1;
    CM_I2C1->CCR = 0x000E2505u;
    CM_I2C1->FLTR = 0x00000003u;
    CM_I2C1->CR1 = 0x00000044u;
    bCM_I2C1->CR1_b.SWRST = 0;
}

static uint8_t i2c_wait_bus_idle(void)
{
    uint32_t timeout = 0x10000u;

    while (CM_I2C1->SR & (1u << 5)) 
    { 
        if (--timeout == 0) {
            return 1;
        }
    }
    return 0;
}


static uint8_t i2c_send_start(void)
{
    uint32_t timeout = 0x10000u;
    bCM_I2C1->CR1_b.START = 1;
    while ((CM_I2C1->SR & (1u << 0)) == 0) 
    { 
        if (--timeout == 0) {
            return 1;
        }
    }
    CM_I2C1->CLR = (1u << 0);
    return 0;
}


static void i2c_send_stop(void)
{
    bCM_I2C1->CR1_b.STOP = 1;
}

static uint8_t i2c_send_addr(uint8_t addr, uint8_t rw)
{
    uint32_t timeout = 0x10000u;
    CM_I2C1->DTR = (addr << 1) | rw;
    while ((CM_I2C1->SR & (1u << 3)) == 0) { 
        if (--timeout == 0) {
            return 1;
        }
    }
    CM_I2C1->CLR = (1u << 3);
    if (CM_I2C1->SR & (1u << 8)) {
        CM_I2C1->CLR = (1u << 8);
        return 1;
    }
    
    return 0;
}

static uint8_t i2c_send_data(uint8_t data)
{
    uint32_t timeout = 0x10000u;
    
    CM_I2C1->DTR = data;
    
    while ((CM_I2C1->SR & (1u << 3)) == 0) {
        if (--timeout == 0) {
            return 1;
        }
    }
    CM_I2C1->CLR = (1u << 3);
    if (CM_I2C1->SR & (1u << 8)) {
        CM_I2C1->CLR = (1u << 8);
        return 1;
    }
    return 0;
}

static uint8_t i2c_recv_data(uint8_t ack)
{
    uint32_t timeout = 0x10000u;
    uint8_t data;

    if (ack) {
        bCM_I2C1->CR1_b.ACK = 1;
    } else {
        bCM_I2C1->CR1_b.ACK = 0;
    }

    while ((CM_I2C1->SR & (1u << 6)) == 0) { 
        if (--timeout == 0) {
            return 0xFF;
        }
    }
    data = CM_I2C1->DRR;
    return data;
}

uint8_t i2c_master_send_cmd(uint8_t cmd)
{
    uint8_t ret = 0;
    
    if (i2c_wait_bus_idle()) {
        return 1;
    }
    if (i2c_send_start()) {
        ret = 1;
        goto exit;
    }
    if (i2c_send_addr(I2C_SLAVE_ADDR, 0)) {
        ret = 1;
        goto exit;
    }
    if (i2c_send_data(cmd)) {
        ret = 1;
        goto exit;
    }
exit:
    i2c_send_stop();
    sys_delay_us(100); 
    return ret;
}

uint8_t i2c_master_read_status(void)
{
    uint8_t status = 0xFF;
    
    if (i2c_wait_bus_idle()) {
        return 0xFF;
    }
    
    if (i2c_send_start()) {
        goto exit;
    }
    
    if (i2c_send_addr(I2C_SLAVE_ADDR, 1)) {
        goto exit;
    }
    
    status = i2c_recv_data(0);
    
exit:
    i2c_send_stop();
    sys_delay_us(100);
    
    return status;
}

uint8_t i2c_master_cmd_and_read(uint8_t cmd, uint8_t *status)
{
    if (i2c_master_send_cmd(cmd)) {
        return 1;
    }
    
    sys_delay_us(500); 
    
    *status = i2c_master_read_status();
    
    if (*status == 0xFF) {
        return 1;
    }
    
    return 0;
}


void i2c_test(void)
{
    
}