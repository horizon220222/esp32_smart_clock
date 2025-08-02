#include "ck_servo_display.h"
#include "esp_log.h"
#include "ck_lu9685.h"

const uint8_t digit_segments[11][7] = {
        {1, 1, 1, 1, 1, 1, 0}, // 0
        {0, 1, 1, 0, 0, 0, 0}, // 1
        {1, 1, 0, 1, 1, 0, 1}, // 2
        {1, 1, 1, 1, 0, 0, 1}, // 3
        {0, 1, 1, 0, 0, 1, 1}, // 4
        {1, 0, 1, 1, 0, 1, 1}, // 5
        {1, 0, 1, 1, 1, 1, 1}, // 6
        {1, 1, 1, 0, 0, 0, 0}, // 7
        {1, 1, 1, 1, 1, 1, 1}, // 8
        {1, 1, 1, 1, 0, 1, 1},  // 9
        {0, 0, 0, 0, 0, 0, 0}  // 空
};


void servo_display_init(float hz)
{
    LU9685_Init(PCA_ADDR_1, hz, 0);
    LU9685_Init(PCA_ADDR_2, hz, 0);
}

uint8_t get_pca_addr(uint8_t servo_num) {
    return servo_num < 16 ? PCA_ADDR_1 : PCA_ADDR_2;
}

uint8_t get_actual_servo_num(uint8_t servo_num) {
    return servo_num < 16 ? servo_num : servo_num - 16;
}

/**
 * @brief 设置舵机角度
 *
 * @param servo_num 舵机编号
 * @param angle 角度
 */
void set_servo_angle(uint8_t servo_num, uint8_t angle) {
    uint8_t pca_addr = get_pca_addr(servo_num);
    uint8_t actual_servo_num = get_actual_servo_num(servo_num);
    LU9685_SetSingleAngle(pca_addr, actual_servo_num, angle);
}

/**
 * @brief 显示数字在指定段
 *
 * @param segment_base 段基地址
 * @param digit 数字
 */
void display_digit_at_segment(uint8_t segment_base, uint8_t digit) {
    for (int i = 0; i < 7; i++) {
        uint8_t servo_num = segment_base + i;
        if (digit_segments[digit][i]) {
            set_servo_angle(servo_num, DISPLAY_ANGLE);
        } else {
            set_servo_angle(servo_num, HIDE_ANGLE);
        }
    }
}

/**
 * @brief 设置显示时间
 *
 * @param hours 小时
 * @param minutes 分钟
 */
void servo_display_set(uint8_t hours, uint8_t minutes)
{
    uint8_t hour_tens = hours / 10;
    uint8_t hour_ones = hours % 10;
    uint8_t minute_tens = minutes / 10;
    uint8_t minute_ones = minutes % 10;

    display_digit_at_segment(HOUR_TENS_BASE, hour_tens);
    display_digit_at_segment(HOUR_ONES_BASE, hour_ones);

    display_digit_at_segment(MINUTE_TENS_BASE, minute_tens);
    display_digit_at_segment(MINUTE_ONES_BASE, minute_ones);
}