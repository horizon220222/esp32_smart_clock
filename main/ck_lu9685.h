#ifndef _CK_PCA9685_H_
#define _CK_PCA9685_H_

#include "esp_log.h"

#define GPIO_SDA    21
#define GPIO_SCL    22

#define PCA_ADDR_1            0x00        // 第一块板子 I2C 地址
#define PCA_ADDR_2            0x02        // 第二块板子 I2C 地址

void LU9685_Init(uint8_t pca_addr, float hz, uint8_t angle);
void LU9685_setFreq(uint8_t pca_addr, float freq);
void LU9685_SetSingleAngle(uint8_t lu_addr, uint8_t servo_num, uint8_t angle);
void LU9685_SetAllAngles(uint8_t lu_addr, uint8_t angles[20]);

#endif