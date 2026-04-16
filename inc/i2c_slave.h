#ifndef __I2C_SLAVE_H_
#define __I2C_SLAVE_H_

#include "system_hc32f460.h"


#define I2C_SLAVE_ADDR  0x50

#define I2C_CMD_CHARGE_START    0x01
#define I2C_CMD_CHARGE_COMPLETE 0x02
#define I2C_CMD_IDLE            0x00
#define I2C_CMD_TEST            0x03  // 测试命令
#define I2C_CMD_RESET           0x04  // 复位命令

#define I2C_STATUS_NORMAL       0x10
#define I2C_STATUS_INTERRUPT    0x11
#define I2C_STATUS_IDLE         0x00
#define I2C_STATUS_TEST         0x03  // 测试状态
#define I2C_STATUS_TEST_TRUE    0x55  //测试正常

void i2c_slave_init(void);
void i2c_slave_poll(void);
void i2c_slave_set_status(uint8_t status);
uint8_t i2c_slave_get_status(void);
uint8_t i2c_slave_get_cmd(void);
void i2c_slave_ir_process(void);
uint8_t i2c_slave_get_status_changed(void);

#endif