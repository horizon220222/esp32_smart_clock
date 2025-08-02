#ifndef _CK_SERVO_DISPLAY_H
#define _CK_SERVO_DISPLAY_H

#include "esp_err.h"

// 定义显示段基地址
#define HOUR_TENS_BASE    0
#define HOUR_ONES_BASE    8
#define MINUTE_TENS_BASE  16
#define MINUTE_ONES_BASE  24

void servo_display_init(float hz);
void servo_display_set(uint8_t hours, uint8_t minutes);
void set_servo_angle(uint8_t servo_num, uint8_t angle);
void display_digit_at_segment(uint8_t segment_base, uint8_t digit);

#endif