#ifndef __I2C_MASTER_H_
#define __I2C_MASTER_H_

#include "system_hc32f460.h"

// I2C从机地址（7位地址）
#define I2C_SLAVE_ADDR  0x50

// 主机发送命令定义
#define I2C_CMD_CHARGE_START    0x01  // 发送充电红外信号
#define I2C_CMD_CHARGE_COMPLETE 0x02  // 发送充电完成红外信号
#define I2C_CMD_IDLE            0x00  // 无命令
#define I2C_CMD_TEST            0x03  // 测试命令
#define I2C_CMD_RESET           0x04  // 复位命令

// 从机状态定义
#define I2C_STATUS_NORMAL       0x10  // 红外通讯正常
#define I2C_STATUS_INTERRUPT    0x11  // 通讯中断或重定位
#define I2C_STATUS_IDLE         0x00  // 未进行通讯
#define I2C_STATUS_TEST         0x03  // 测试状态
#define I2C_STATUS_TEST_TRUE    0x55  //测试正常

void i2c_master_init(void);
uint8_t i2c_master_send_cmd(uint8_t cmd);
uint8_t i2c_master_read_status(void);
uint8_t i2c_master_cmd_and_read(uint8_t cmd, uint8_t *status);
void i2c_master_test(void);

#endif