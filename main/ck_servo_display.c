#include "ck_servo_display.h"
#include "esp_log.h"
#include "ck_lu9685.h"
#include "freertos/FreeRTOS.h"


const uint8_t digit_segments[13][7] = {
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
        {0, 0, 0, 0, 0, 0, 0},  // 空
        {1, 1, 0, 0, 0, 1, 1},  // 上0
        {0, 0, 1, 1, 1, 0, 1},  // 下0
};

// 定义不同组的隐藏角度数组
const uint8_t HIDE_ANGLES_GROUP_1_3[7] = {180, 180, 0, 0, 180, 0, 180};
const uint8_t HIDE_ANGLES_GROUP_2_4[7] = {0, 180, 0, 180, 180, 0, 0};

// 全局数组，用于缓存每个舵机的当前角度
static uint8_t servo_angle_cache[32] = {0};

void servo_display_init(float hz)
{
    LU9685_Init(PCA_ADDR_1, hz, 90);
    LU9685_Init(PCA_ADDR_2, hz, 90);
    // 初始化缓存数组
    for (int i = 0; i < 32; i++) {
        servo_angle_cache[i] = 90;
    }
}

uint8_t get_pca_addr(uint8_t servo_num) {
    return servo_num < 16 ? PCA_ADDR_1 : PCA_ADDR_2;
}

uint8_t get_actual_servo_num(uint8_t servo_num) {
    return servo_num < 16 ? servo_num : servo_num - 16;
}

/**
 * @brief 设置舵机角度，若角度未变化则不发送消息
 *
 * @param servo_num 舵机编号
 * @param angle 角度
 */
void set_servo_angle(uint8_t servo_num, uint8_t angle) {
    if (servo_num >= 32) {
        ESP_LOGE("servo_display", "Invalid servo number: %d", servo_num);
        return;
    }
    // 检查缓存的角度和要设置的角度是否相同
    if (servo_angle_cache[servo_num] == angle) {
        return;
    }
    uint8_t pca_addr = get_pca_addr(servo_num);
    uint8_t actual_servo_num = get_actual_servo_num(servo_num);
    LU9685_SetSingleAngle(pca_addr, actual_servo_num, angle);
    // 更新缓存
    servo_angle_cache[servo_num] = angle;
}

void display_digit_at_segment(uint8_t segment_base, uint8_t digit) {
    const uint8_t *hide_angles;
    if ((segment_base >= HOUR_TENS_BASE && segment_base < HOUR_ONES_BASE) ||
        (segment_base >= MINUTE_TENS_BASE && segment_base < MINUTE_ONES_BASE)) {
        hide_angles = HIDE_ANGLES_GROUP_1_3;
    } else {
        hide_angles = HIDE_ANGLES_GROUP_2_4;
    }

    uint8_t servo_2 = segment_base + 1;
    uint8_t servo_6 = segment_base + 5;
    uint8_t servo_7 = segment_base + 6;

    bool old_7_displayed = (servo_angle_cache[servo_7] == 90);
    bool new_7_displayed = digit_segments[digit][6];
    bool pin_2_is_displayed = (servo_angle_cache[servo_2] == 90);
    bool pin_6_is_displayed = (servo_angle_cache[servo_6] == 90);
    bool moved = false;

    if (old_7_displayed != new_7_displayed) {
        if (pin_2_is_displayed) {
            set_servo_angle(servo_2, hide_angles[1]);
            moved = true;
        }
        if (pin_6_is_displayed) {
            set_servo_angle(servo_6, hide_angles[5]);
            moved = true;
        }
    }

    if (moved) {
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // 设置所有段
    for (int i = 0; i < 7; i++) {
        uint8_t servo_num = segment_base + i;
        uint8_t target_angle = digit_segments[digit][i] ? 90 : hide_angles[i];
        set_servo_angle(servo_num, target_angle);
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
    vTaskDelay(pdMS_TO_TICKS(500));
    display_digit_at_segment(HOUR_ONES_BASE, hour_ones);
    vTaskDelay(pdMS_TO_TICKS(500));
    display_digit_at_segment(MINUTE_TENS_BASE, minute_tens);
    vTaskDelay(pdMS_TO_TICKS(500));
    display_digit_at_segment(MINUTE_ONES_BASE, minute_ones);
}