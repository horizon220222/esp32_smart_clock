#include "ck_lu9685.h"
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include <inttypes.h>
#include "rom/ets_sys.h"
#include "hal/gpio_types.h"

// 寄存器定义
#define LU_RESET              0xFB
#define LU_HZ                 0xFC
#define LU_ALL_CONTROL        0xFD

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

//------------------LU9685底层函数------------------

// 软复位函数
void LU9685_Reset(uint8_t lu_addr) {
    IIC_Start();
    Send_Byte(lu_addr);
    I2C_WaitAck();
    Send_Byte(LU_RESET);
    I2C_WaitAck();
    Send_Byte(LU_RESET); // 补充发送一个 0xFB
    I2C_WaitAck();
    IIC_Stop();
}

// 设置 PWM 频率函数
void LU9685_setFreq(uint8_t lu_addr, float freq) {
    IIC_Start();
    Send_Byte(lu_addr);
    I2C_WaitAck();
    Send_Byte(LU_HZ);
    I2C_WaitAck();

    if (freq >= 30 && freq <= 400) {
        uint8_t freq_byte = (uint8_t)freq;
        Send_Byte(freq_byte);
        I2C_WaitAck();
    } else if (freq >= 20 && freq <= 300) {
        uint16_t freq_val = (uint16_t)freq;
        uint8_t high_byte = (freq_val >> 8) & 0xFF;
        uint8_t low_byte = freq_val & 0xFF;
        Send_Byte(high_byte);
        I2C_WaitAck();
        Send_Byte(low_byte);
        I2C_WaitAck();
    } else {
        // 默认频率 50Hz
        Send_Byte(50);
        I2C_WaitAck();
    }
    IIC_Stop();
}

// 单个舵机控制函数
/**
 * @brief 设置单个舵机的角度
 *
 * @param lu_addr LU9685 芯片的 I2C 地址
 * @param servo_num 舵机编号，范围 0 - 19
 * @param angle 舵机角度，范围 0 - 180 度
 */
void LU9685_SetSingleAngle(uint8_t lu_addr, uint8_t servo_num, uint8_t angle)
{
    IIC_Start();
    Send_Byte(lu_addr);
    I2C_WaitAck();
    Send_Byte(servo_num);
    I2C_WaitAck();
    Send_Byte(angle);
    I2C_WaitAck();
    IIC_Stop();
}

// 20 路舵机同时控制函数
void LU9685_SetAllAngles(uint8_t lu_addr, uint8_t angles[20]) {
    IIC_Start();
    Send_Byte(lu_addr);
    I2C_WaitAck();
    Send_Byte(LU_ALL_CONTROL);
    I2C_WaitAck();
    for (int i = 0; i < 20; i++) {
        Send_Byte(angles[i]);
        I2C_WaitAck();
    }
    IIC_Stop();
}

/**
 * @brief 初始化 LU9685 芯片
 *
 * @param lu_addr LU9685 芯片的 I2C 地址
 * @param hz PWM 输出频率，单位为赫兹
 * @param angle 舵机角度，范围为 0-180 度
 */
void LU9685_Init(uint8_t lu_addr, float hz, uint8_t angle) {
    // 初始化引脚
    gpio_config_t OUT_config = {
        .pin_bit_mask = (1ULL << GPIO_SDA) | (1ULL << GPIO_SCL),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&OUT_config);

    // 软复位芯片
    LU9685_Reset(lu_addr);

    // 设置 LU9685 芯片的 PWM 输出频率
    LU9685_setFreq(lu_addr, hz);

    // 20 路舵机同时设置角度
    uint8_t angles[20];
    for (int i = 0; i < 20; i++) {
        angles[i] = angle;
    }
    LU9685_SetAllAngles(lu_addr, angles);

    delay_ms(100);
}