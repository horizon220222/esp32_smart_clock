#include "servo_display.h"
#include "esp_log.h"

static const char *TAG = "servo_display";

esp_err_t servo_display_init(void)
{
    ESP_LOGI(TAG, "舵机显示模块初始化完成（存根）");
    return ESP_OK;
}

void servo_display_set(const char *str)
{
    ESP_LOGI(TAG, "显示内容: %s", str);
}