#include <stdio.h>
#include <esp_log.h>
#include <driver/uart.h>
#include "nvs_flash.h"
#include "ck_wifi_time.h"
#include "ck_state.h"
#include "ck_servo_display.h"
#include "ck_time_ext.h" // 包含 ck_time_ext.h 头文件



void app_main(void)
{
    // 初始化
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(app_state_init());
    ESP_ERROR_CHECK(wifi_time_init());
    servo_display_init(50);

    // 初始化 UART 配置
    uart_config_t uart_config = {
            .baud_rate = 9600,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_APB,
    };
    // 安装 UART 驱动
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    // 配置 UART 参数
    uart_param_config(UART_NUM, &uart_config);

    // 调用开场动画函数
    opening_animation();

    // 创建时间同步任务
    xTaskCreate(time_sync_task, "time_sync_task", 1024, NULL, 5, NULL);
    // 创建串口监听任务
     xTaskCreate(uart_listen_task, "uart_listen_task", 2048, NULL, 5, NULL);

    // 主循环
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10 * 1000));
    }
}
