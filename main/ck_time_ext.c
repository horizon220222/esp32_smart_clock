#include <driver/uart.h>
#include <string.h>
#include "ck_time_ext.h"
#include "ck_servo_display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "time.h"
#include "ck_state.h"
#include "stdlib.h"

#define TAG "ck_time_ext"

// 全局变量，用于记录串口接收到的数据
static char received_data[3] = "02";

// 倒计时函数
void countdown_task() {
    int remaining_seconds = 60; // 1分30秒
    while (remaining_seconds > 0) {
        int minutes = remaining_seconds / 60;
        int seconds = remaining_seconds % 60;
        servo_display_set(minutes, seconds);
        vTaskDelay(pdMS_TO_TICKS(1000)); // 每秒更新一次
        remaining_seconds--;
    }
}

// 开场动画函数
void opening_animation(void) {
    for (int i = 33; i >= 0; i -= 11) {
        servo_display_set(i, i);
        vTaskDelay(pdMS_TO_TICKS(1000)); // 显示 1 秒
    }
}


void time_sync_task(void *pv)
{
    while (true) {
        if (strncmp(received_data, "02", 2) == 0) {
            time_t now = time(NULL);
            struct tm time_info;
            localtime_r(&now, &time_info);

            app_state_set_time(time_info.tm_hour, time_info.tm_min);
            servo_display_set(time_info.tm_hour, time_info.tm_min);
        }
        vTaskDelay(pdMS_TO_TICKS(10 * 1000));   // 每10s打印一次
    }
}

void uart_listen_task(void *pv) {
    uint8_t data[BUF_SIZE];
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        int len = uart_read_bytes(UART_NUM, data, BUF_SIZE, pdMS_TO_TICKS(100));
        ESP_LOGI(TAG, "Received data: %s", data);
        if (len > 0) {
            if (len < BUF_SIZE) {
                data[len] = '\0';
            }
            strncpy(received_data, (char *)data, 2);
            received_data[2] = '\0';

            // 将 received_data 转换为整数
            int cmd = atoi(received_data);
            switch (cmd) {
                case 0:
                    // 唤醒表情
                    display_digit_at_segment(HOUR_ONES_BASE, 11);
                    display_digit_at_segment(MINUTE_TENS_BASE, 11);
                    vTaskDelay(pdMS_TO_TICKS(10000));
                    strncpy(received_data, "02", 2);
                    break;
                case 1:
                    // 60秒倒计时函数
                    countdown_task();
                    vTaskDelay(pdMS_TO_TICKS(10000));
                    strncpy(received_data, "02", 2);
                    break;
                case 2:
                    // 显示当前时间，这里只是让他立即更新
                    time_t now = time(NULL);
                    struct tm time_info;
                    localtime_r(&now, &time_info);
                    app_state_set_time(time_info.tm_hour, time_info.tm_min);
                    servo_display_set(time_info.tm_hour, time_info.tm_min);
                    break;
                case 3:
                    // 显示编号
                    servo_display_set(3, 3);
                    vTaskDelay(pdMS_TO_TICKS(10000));
                    strncpy(received_data, "02", 2);
                    break;
                case 4:
                    // 用户提前结束
                    display_digit_at_segment(HOUR_ONES_BASE, 8);
                    display_digit_at_segment(MINUTE_TENS_BASE, 8);
                    vTaskDelay(pdMS_TO_TICKS(5000));
                    strncpy(received_data, "02", 2);
                    break;
                case 5:
                    // 显示开场
                    opening_animation();
                    vTaskDelay(pdMS_TO_TICKS(10000));
                    strncpy(received_data, "02", 2);
                    break;
                default:
                    // 处理未知命令
                    break;
            }
        }
    }
}