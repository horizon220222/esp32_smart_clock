#include "ck_pca9685.h"
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include <inttypes.h>
#include "rom/ets_sys.h"
#include "hal/gpio_types.h"

static const char *TAG = "ck_pca9685";

// 寄存器定义
#define PCA_Model             0x00
#define LED0_ON_L             0x06
#define LED0_ON_H             0x07
#define LED0_OFF_L            0x08
#define LED0_OFF_H            0x09
#define PCA_Pre               0xFE

// IO口操作
#define SDA_OUT()        gpio_set_direction(GPIO_SDA,GPIO_MODE_OUTPUT)
#define SDA_IN()         gpio_set_direction(GPIO_SDA,GPIO_MODE_INPUT)
#define SDA_GET()        gpio_get_level(GPIO_SDA)
#define SDA(x)           gpio_set_level(GPIO_SDA, (x?1:0))
#define SCL(x)           gpio_set_level(GPIO_SCL, (x?1:0))

void delay_ms(unsigned int ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void delay_us(unsigned int us) {
    ets_delay_us(us);
}

//------------------IIC相关函数------------------

void IIC_Start(void) {
    SDA_OUT();

    SDA(1);
    delay_us(5);
    SCL(1);
    delay_us(5);

    SDA(0);
    delay_us(5);
    SCL(0);
    delay_us(5);

}

void IIC_Stop(void) {
    SDA_OUT();
    SCL(0);
    SDA(0);

    SCL(1);
    delay_us(5);
    SDA(1);
    delay_us(5);

}

void IIC_Send_Ack(unsigned char ack) {
    SDA_OUT();
    SCL(0);
    SDA(0);
    delay_us(5);
    if (!ack) SDA(0);
    else
        SDA(1);
    SCL(1);
    delay_us(5);
    SCL(0);
    SDA(1);
}

unsigned char I2C_WaitAck(void) {

    char ack = 0;
    unsigned char ack_flag = 10;
    SCL(0);
    SDA(1);
    SDA_IN();
    delay_us(5);
    SCL(1);
    delay_us(5);

    while ((SDA_GET() == 1) && (ack_flag)) {
        ack_flag--;
        delay_us(5);
    }

    if (ack_flag <= 0) {
        IIC_Stop();
        return 1;
    } else {
        SCL(0);
        SDA_OUT();
    }
    return ack;
}

void Send_Byte(uint8_t dat) {
    int i = 0;
    SDA_OUT();
    SCL(0);//拉低时钟开始数据传输

    for (i = 0; i < 8; i++) {
        SDA((dat & 0x80) >> 7);
        delay_us(1);
        SCL(1);
        delay_us(5);
        SCL(0);
        delay_us(5);
        dat <<= 1;
    }
}

unsigned char Read_Byte(void) {
    unsigned char i, receive = 0;
    SDA_IN();//SDA设置为输入
    for (i = 0; i < 8; i++) {
        SCL(0);
        delay_us(5);
        SCL(1);
        delay_us(5);
        receive <<= 1;
        if (SDA_GET()) {
            receive |= 1;
        }
        delay_us(5);
    }
    SCL(0);
    return receive;
}

//------------------PCA9685底层函数------------------


void PCA9685_Write(uint8_t pca_addr, uint8_t addr, uint8_t data) {
    IIC_Start();

    Send_Byte(pca_addr);
    I2C_WaitAck();

    Send_Byte(addr);
    I2C_WaitAck();

    Send_Byte(data);
    I2C_WaitAck();

    IIC_Stop();
}

uint8_t PCA9685_Read(uint8_t pca_addr, uint8_t addr) {
    uint8_t data;

    IIC_Start();

    Send_Byte(pca_addr);
    I2C_WaitAck();

    Send_Byte(addr);
    I2C_WaitAck();

    IIC_Stop();

    delay_us(10);

    IIC_Start();

    Send_Byte(pca_addr | 0x01);
    I2C_WaitAck();

    data = Read_Byte();
    IIC_Send_Ack(1);
    IIC_Stop();

    return data;
}

/**
 * @brief 设置 PCA9685 芯片的 PWM 输出值
 *
 * @param pca_addr PCA9685 芯片的 I2C 地址
 * @param num 要设置的 PWM 输出通道，范围为 0-15
 * @param on 要设置的 PWM 输出值，范围为 0-4095
 * @param off 要设置的 PWM 输出值，范围为 0-4095
 */
void PCA9685_setPWM(uint8_t pca_addr, uint8_t num, uint32_t on, uint32_t off) {
    IIC_Start();

    Send_Byte(pca_addr);
    I2C_WaitAck();

    Send_Byte(LED0_ON_L + 4 * num);
    I2C_WaitAck();

    Send_Byte(on & 0xFF);
    I2C_WaitAck();

    Send_Byte(on >> 8);
    I2C_WaitAck();

    Send_Byte(off & 0xFF);
    I2C_WaitAck();

    Send_Byte(off >> 8);
    I2C_WaitAck();

    IIC_Stop();
}

//------------------PCA9685应用函数------------------

/**
 * @brief 设置 PCA9685 芯片的 PWM 输出角度
 *
 * @param pca_addr PCA9685 芯片的 I2C 地址
 * @param num 要设置的 PWM 输出通道，范围为 0-15
 * @param angle 要设置的角度，范围为 0-180 度
 */
void PCA9685_SetAngle(uint8_t pca_addr, uint8_t num, uint8_t angle) {
    uint32_t off = 0;
    off = (uint32_t) (103 + angle * 2.1);
    PCA9685_setPWM(pca_addr, num, 0, off);
}

/**
 * @brief 设置 PCA9685 芯片的 PWM 输出频率
 *
 * @param pca_addr PCA9685 芯片的 I2C 地址
 * @param freq PWM 输出频率，单位为赫兹
 */
void PCA9685_setFreq(uint8_t pca_addr, float freq) {
    uint8_t pre_scale, old_mode, new_mode;
    double pre_scale_val;
    pre_scale_val = 25000000;
    pre_scale_val /= 4096;
    pre_scale_val /= freq;
    pre_scale_val -= 1;
    pre_scale = floor(pre_scale_val + 0.5f);
    old_mode = PCA9685_Read(pca_addr, PCA_Model);
    new_mode = (old_mode & 0x7F) | 0x10;
    PCA9685_Write(pca_addr, PCA_Model, new_mode);
    PCA9685_Write(pca_addr, PCA_Pre, pre_scale);
    PCA9685_Write(pca_addr, PCA_Model, old_mode);
    delay_ms(5);
    PCA9685_Write(pca_addr, PCA_Model, old_mode | 0xa1);
}

/**
 * @brief 初始化 PCA9685 芯片
 *
 * @param pca_addr PCA9685 芯片的 I2C 地址
 * @param hz PWM 输出频率，单位为赫兹
 * @param angle 舵机角度，范围为 0-180 度
 */
void PCA9685_Init(uint8_t pca_addr, float hz, uint8_t angle) {

    //初始化引脚
    gpio_config_t OUT_config = {
            .pin_bit_mask = (1ULL << GPIO_SDA) | (1ULL << GPIO_SCL),
            .mode =GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&OUT_config);

    // 向 PCA9685 芯片的模式寄存器写入 0x00，将芯片设置为默认工作模式
    PCA9685_Write(pca_addr, PCA_Model, 0x00);

    // 设置 PCA9685 芯片的 PWM 输出频率
    PCA9685_setFreq(pca_addr, hz);

    // 定义一个无符号 32 位整数变量 off，用于存储 PWM 信号的关断时间
    uint32_t off = 0;
    off = (uint32_t) (103 + angle * 2.1);

    // 控制16个舵机输出off角度
    PCA9685_setPWM(pca_addr, 0, 0, off);
    PCA9685_setPWM(pca_addr, 1, 0, off);
    PCA9685_setPWM(pca_addr, 2, 0, off);
    PCA9685_setPWM(pca_addr, 3, 0, off);
    PCA9685_setPWM(pca_addr, 4, 0, off);
    PCA9685_setPWM(pca_addr, 5, 0, off);
    PCA9685_setPWM(pca_addr, 6, 0, off);
    PCA9685_setPWM(pca_addr, 7, 0, off);
    PCA9685_setPWM(pca_addr, 8, 0, off);
    PCA9685_setPWM(pca_addr, 9, 0, off);
    PCA9685_setPWM(pca_addr, 10, 0, off);
    PCA9685_setPWM(pca_addr, 11, 0, off);
    PCA9685_setPWM(pca_addr, 12, 0, off);
    PCA9685_setPWM(pca_addr, 13, 0, off);
    PCA9685_setPWM(pca_addr, 14, 0, off);
    PCA9685_setPWM(pca_addr, 15, 0, off);

    delay_ms(100);
}