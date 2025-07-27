#include <stdio.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "wifi_time.h"
#include "app_state.h"
#include "servo_display.h"
#include "alarm_mgr.h"

static const char *TAG = "main";

void time_sync_task(void *pv)
{
    while (true) {
        time_t now = time(NULL);
        struct tm time_info;
        localtime_r(&now, &time_info);
        app_state_set_time(time_info.tm_hour, time_info.tm_min);

        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &time_info);
        servo_display_set(time_str);

        vTaskDelay(pdMS_TO_TICKS(10 * 1000));   // 每10s打印一次
    }
}

static void alarm_cb(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    ESP_LOGI(TAG, "!!! ALARM !!!");
    ESP_LOGI(TAG, "!!! ALARM !!!");
}


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
    ESP_ERROR_CHECK(servo_display_init());
    ESP_ERROR_CHECK(alarm_mgr_init());

    // 时间同步任务
    xTaskCreate(time_sync_task,"time_sync_task",2048,NULL,5,NULL);

    // 订阅事件
    esp_event_handler_register(APP_EVENT, APP_STATE_ALARM_FIRED, alarm_cb, NULL);
    
    // 主循环
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
