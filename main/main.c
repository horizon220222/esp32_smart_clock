#include <stdio.h>
#include <time.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "ck_wifi_time.h"
#include "ck_state.h"
#include "ck_servo_display.h"

static const char *TAG = "main";

void time_sync_task(void *pv)
{
    while (true) {
        time_t now = time(NULL);
        struct tm time_info;
        localtime_r(&now, &time_info);
        app_state_set_time(time_info.tm_hour, time_info.tm_min);
        servo_display_set(time_info.tm_hour, time_info.tm_min);
        vTaskDelay(pdMS_TO_TICKS(10 * 1000));   // 每10s打印一次
    }
}


void app_main(void)
{
    // 初始化
    ESP_ERROR_CHECK(nvs_flash_erase());
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(app_state_init());
    ESP_ERROR_CHECK(wifi_time_init());
    servo_display_init(50);

    // 时间同步任务
    xTaskCreate(time_sync_task,"time_sync_task",2048,NULL,5,NULL);

    // 主循环
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
