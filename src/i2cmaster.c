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
    CM_I2C1->CLR = 0x00F050DFu; 
    CM_I2C1->CCR = 0x000E2505u;
    CM_I2C1->FLTR = 0x00000003u;
    bCM_I2C1->CR1_b.SWRST = 0;
    bCM_I2C1->CR1_b.PE = 1;
}

static uint8_t i2c_wait_bus_idle(void)
{
    uint32_t timeout = 0x10000u;

    while (CM_I2C1->SR & (1u << 17)) 
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


static uint8_t  i2c_send_stop(void)
{
    uint32_t timeout = 0x10000u;

    bCM_I2C1->CR1_b.STOP = 1;
     while ((CM_I2C1->SR & (1u << 4)) == 0) {
        if (--timeout == 0) {
            return 1;  
        }
    }
    CM_I2C1->CLR = (1u << 4);

     return 0;

}

static uint8_t i2c_send_addr(uint8_t addr, uint8_t rw)
{
    uint32_t timeout = 0x10000u;
    CM_I2C1->DTR = (addr << 1) | rw;
    while (((CM_I2C1->SR & ((1u << 3) | (1u << 12))) == 0)) { 
        if (--timeout == 0) {
            return 1;
        }
    }
    if (CM_I2C1->SR & (1u << 12)) {
        CM_I2C1->CLR = (1u << 12);
        return 1;
    }
    CM_I2C1->CLR = (1u << 3);
    return 0;
}

static uint8_t i2c_send_data(uint8_t data)
{
    uint32_t timeout = 0x10000u;
    
    CM_I2C1->DTR = data;
    
    while ((CM_I2C1->SR & (1u << 3 | (1u << 12))) == 0) {
        if (--timeout == 0) {
            return 1;
        }
    }
    if (CM_I2C1->SR & (1u << 12)) {
        CM_I2C1->CLR = (1u << 12);
        return 1;
    }
    CM_I2C1->CLR = (1u << 3);
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

    CM_I2C1->CLR = 0x00F050DFu; 
    
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

    CM_I2C1->CLR = 0x00F050DFu;
    
    
    if (i2c_wait_bus_idle()) {     
        goto exit;
    }

    if (i2c_send_start()) {
        goto exit;
    }

    bCM_I2C1->CR1_b.ACK = 0;
    
    if (i2c_send_addr(I2C_SLAVE_ADDR, 1)) {
        goto exit;
    }
    
    status = i2c_recv_data(0);
    
exit:
    i2c_send_stop();
    bCM_I2C1->CR1_b.ACK = 1;
    CM_I2C1->CLR = 0x00F050DFu;
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


void test(void)
{
    CM_GPIO->PORRA = 0x001E;
    CM_GPIO->POSRA = 0x0002;
        sys_delay_ms(10);
        CM_GPIO->PORRA = 0x001E;
        sys_delay_ms(100);
        CM_GPIO->POSRA = 0x0004;
        sys_delay_ms(10);
        CM_GPIO->PORRA = 0x001E;

        
}

void i2c_master_test(void)
{
    i2c_master_send_cmd(I2C_CMD_TEST);
    sys_delay_ms(2000);
    i2c_master_send_cmd(I2C_CMD_RESET);
    sys_delay_ms(2000);



//     uint8_t status;
//     uint8_t retry;
//     CM_I2C1->CLR = 0x00F050DFu;
//      sys_delay_ms(100);

//     // 发送测试命令
//     for (retry = 0; retry < 3; retry++) {
//     if (i2c_master_send_cmd(I2C_CMD_TEST) == 0) {
//         CM_GPIO->PORRA = 0x001E;
//         CM_GPIO->POSRA = 0x0002;
//         sys_delay_ms(10);
//         CM_GPIO->PORRA = 0x001E;
//         sys_delay_ms(300);
//         CM_GPIO->POSRA = 0x0004;
//         sys_delay_ms(10);
//         CM_GPIO->PORRA = 0x001E;
//     }
//      CM_I2C1->CLR = 0x00F050DFu;
//     sys_delay_ms(100);
// }

//     sys_delay_ms(2000);

//     // 读取从机状态
//     status = 0xFF;
//     for (retry = 0; retry < 3; retry++) {
//         status = i2c_master_read_status();
//         if (status != 0xFF) {
//             break;
//         }
//         CM_I2C1->CLR = 0x00F050DFu;
//         sys_delay_ms(100);
//     }

//     if (status == I2C_STATUS_TEST_TRUE) {
//         CM_GPIO->PORRA = 0x001E;
//         CM_GPIO->POSRA = 0x0008;
//         sys_delay_ms(10);
//         CM_GPIO->PORRA = 0x001E;
//         sys_delay_ms(300);
//         CM_GPIO->POSRA = 0x0010;
//         sys_delay_ms(10);
//         CM_GPIO->PORRA = 0x001E;
//     }

//     // 每次循环结束清所有标志，防止残留状态
//     CM_I2C1->CLR = 0x00F050DFu;
//     bCM_I2C1->CR1_b.ACK = 1;

//     sys_delay_ms(3000);

    // uint8_t addr_result;
    // uint8_t status;

    // // ===== 第1步：发送测试命令 =====
    // if (i2c_master_send_cmd(I2C_CMD_TEST) == 0) {
    //     // 发送成功
    //     test();
    // } else {
    //     // 发送失败 → 0x0002闪2次
    //     test();
    //     sys_delay_ms(100);
    //     test();
    //     return;
    // }

    // sys_delay_ms(2000);

    // // ===== 第2步：读取从机状态（分步检查） =====
    // CM_I2C1->CLR = 0x00F050DFu;

    // if (i2c_wait_bus_idle()) {
    //     // 总线忙 → 0x0008闪1次
    //     CM_GPIO->POSRA = 0x0008;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     sys_delay_ms(1000);
    //     CM_GPIO->POSRA = 0x0010;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     sys_delay_ms(3000);
    //     return;
    // }

    // if (i2c_send_start()) {
    //     // START失败 → 0x0008闪2次
    //     CM_GPIO->POSRA = 0x0008;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     sys_delay_ms(300);
    //     CM_GPIO->POSRA = 0x0010;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     sys_delay_ms(300);
    //     CM_GPIO->POSRA = 0x0008;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     sys_delay_ms(300);
    //     CM_GPIO->POSRA = 0x0010;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     sys_delay_ms(3000);
    //     return;
    // }

    // bCM_I2C1->CR1_b.ACK = 0;
    // addr_result = i2c_send_addr(I2C_SLAVE_ADDR, 1);

    // if (addr_result) {
    //     // ★ 读地址NACK → 从机没响应读请求
    //     // → 0x0004闪2次
    //     test();
    //     CM_GPIO->POSRA = 0x0008;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     sys_delay_ms(300);
    //     CM_GPIO->POSRA = 0x0010;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     i2c_send_stop();
    //     bCM_I2C1->CR1_b.ACK = 1;
    //     sys_delay_ms(3000);
    //     return;
    // }

    // // 地址ACK了，读取数据
    // status = i2c_recv_data(0);

    // i2c_send_stop();
    // bCM_I2C1->CR1_b.ACK = 1;
    // sys_delay_us(100);

    // // ===== 第3步：判断读回结果 =====
    // if (status == I2C_STATUS_TEST_TRUE) {
    //     // 读到0x55 → 成功！
    //     CM_GPIO->PORRA = 0x001E;
    //     CM_GPIO->POSRA = 0x0008;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     test();
    // } else if (status == 0xFF) {
    //     // ★ 超时没收到数据 → 0x0010闪2次
    //     test();
    //     CM_GPIO->POSRA = 0x0010;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    // } else {
    //     // ★ 读到了但值不是0x55 → 0x0010闪3次
    //     test();
    //     CM_GPIO->PORRA = 0x001E;
    //     CM_GPIO->POSRA = 0x0008;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     sys_delay_ms(100);
    //     CM_GPIO->POSRA = 0x0010;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     test();
    // }

    // sys_delay_ms(3000);


    //  uint8_t status;

    //  i2c_master_send_cmd(I2C_CMD_TEST);
    //  sys_delay_ms(500);

    // // 第1步：发送测试命令
    // if (i2c_master_send_cmd(I2C_CMD_TEST) == 0) {
    //     // 发送成功 → 0x0002闪 → 0x0004闪
    //     CM_GPIO->PORRA = 0x001E;
    //     CM_GPIO->POSRA = 0x0002;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     sys_delay_ms(100);
    //     CM_GPIO->POSRA = 0x0004;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    // }

    // // 等2秒让从机处理
    // sys_delay_ms(1000);

    // // 第2步：读取从机状态
    // status = i2c_master_read_status();

    // if (status == I2C_STATUS_TEST_TRUE) {
    //     // 读取成功 → 0x0008闪 → 0x0010闪
    //     CM_GPIO->PORRA = 0x001E;
    //     CM_GPIO->POSRA = 0x0008;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     sys_delay_ms(1000);
    //     CM_GPIO->POSRA = 0x0010;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    // }
    // else{
    //     //test();
    // }

    // sys_delay_ms(2000);  


    // uint8_t addr_result;
    // uint8_t status;

    // // ===== 第1步：发送测试命令给从机 =====

    // CM_I2C1->CLR = 0x00F050DFu;

    // // 等总线空闲
    // if (i2c_wait_bus_idle()) {
    //     test();
    //     sys_delay_ms(1000);
    //     return;
    // }

    // // 发START
    // if (i2c_send_start()) {
    //     // START失败：0x0004长闪
    //     test();
    //     sys_delay_ms(1000);
    //     return;
    // }

    // // 发地址+写
    // addr_result = i2c_send_addr(I2C_SLAVE_ADDR, 0);

    // if (addr_result) {
    //     // NACK：0x0002闪2次短 → 从机没响应
    //     CM_GPIO->POSRA = 0x0008;
    //     sys_delay_ms(100);
    //     CM_GPIO->PORRA = 0x001E;
    //     sys_delay_ms(300);
    //     CM_GPIO->POSRA = 0x0010;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     i2c_send_stop();
    //     sys_delay_ms(2000);
    //     return;
    // }

    // // ACK！地址匹配成功 → 发送测试命令0x03
    // if (i2c_send_data(I2C_CMD_TEST)) {
    //     // 数据发送失败：0x0008长闪
    //     CM_GPIO->POSRA = 0x0008;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     sys_delay_ms(1000);
    //     i2c_send_stop();
    //     return;
    // }

    // // 发送命令成功 → 继电器0x0002闪1次
    // CM_GPIO->POSRA = 0x0002;
    // sys_delay_ms(10);
    // CM_GPIO->PORRA = 0x001E;

    // i2c_send_stop();

    // // 等2秒让从机处理命令
    // sys_delay_ms(2000);

    // // ===== 第2步：读取从机状态 =====

    // CM_I2C1->CLR = 0x00F050DFu;

    // if (i2c_wait_bus_idle()) {
    //     return;
    // }

    // if (i2c_send_start()) {
    //     return;
    // }

    // bCM_I2C1->CR1_b.ACK = 0;

    // addr_result = i2c_send_addr(I2C_SLAVE_ADDR, 1);

    // if (addr_result) {
    //     // 读地址NACK：0x0004闪2次短
    //     CM_GPIO->POSRA = 0x0008;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     sys_delay_ms(300);
    //     CM_GPIO->POSRA = 0x0010;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     i2c_send_stop();
    //     bCM_I2C1->CR1_b.ACK = 1;
    //     sys_delay_ms(2000);
    //     return;
    // }

    // status = i2c_recv_data(0);

    // i2c_send_stop();
    // bCM_I2C1->CR1_b.ACK = 1;
    // sys_delay_us(100);

    // // ===== 第3步：判断读回的状态 =====
    // if (status == I2C_STATUS_TEST_TRUE) {
    //     // 读取成功！继电器0x0004闪1次
    //     CM_GPIO->POSRA = 0x0004;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     // CM_GPIO->POSRA = 0x0008;
    //     // sys_delay_ms(10);
    //     // CM_GPIO->PORRA = 0x001E;
    //     // sys_delay_ms(300);
    //     // CM_GPIO->POSRA = 0x0010;
    //     // sys_delay_ms(10);
    //     // CM_GPIO->PORRA = 0x001E;
    // } else {
    //     // 读到的值不对：0x0008闪2次短
    //     CM_GPIO->POSRA = 0x0008;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    //     sys_delay_ms(300);
    //     CM_GPIO->POSRA = 0x0010;
    //     sys_delay_ms(10);
    //     CM_GPIO->PORRA = 0x001E;
    // }

    // sys_delay_ms(3000);  // 3秒后重新测试


    }
    