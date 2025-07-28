#include "ck_state.h"
#include <stdint.h>
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "ck_state";

ESP_EVENT_DEFINE_BASE(APP_EVENT);

/**
 * @brief 应用状态模块全局变量
 *
 * @note 该结构体用于存储应用状态信息，包括时间和 Wi-Fi 连接状态。
 */
static struct {
    StaticSemaphore_t mux_buffer;
    SemaphoreHandle_t mux;
    app_time_t time;
    bool wifi_connected;
} g;

/**
 * @brief 初始化应用状态模块
 *
 * @return esp_err_t 初始化结果，ESP_OK 表示成功，其他值表示失败
 */
esp_err_t app_state_init(void) {
    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGI(TAG, "创建事件循环失败");
        return err;
    }

    g.mux = xSemaphoreCreateMutexStatic(&g.mux_buffer);
    if (!g.mux) return ESP_ERR_NO_MEM;

    g.time.hour = 0;
    g.time.min = 0;
    g.wifi_connected = false;

    ESP_LOGI(TAG, "应用状态模块已就绪");
    return ESP_OK;
}

/**
 * @brief 设置 Wi-Fi 连接状态
 *
 * @param connected 连接状态，true 表示已连接，false 表示未连接
 */
void app_state_set_wifi(bool connected) {
    xSemaphoreTake(g.mux, portMAX_DELAY);
    bool changed = (g.wifi_connected != connected);
    g.wifi_connected = connected;
    xSemaphoreGive(g.mux);

    if (changed) {
        // 如果修改了就关播消息
        app_state_event_id_t ev = connected ? APP_STATE_WIFI_CONNECTED : APP_STATE_WIFI_DISCONNECTED;
        esp_event_post(APP_EVENT, ev, NULL, 0, 0);
    }
}

/**
 * @brief 获取 Wi-Fi 连接状态
 *
 * @return true 表示已连接，false 表示未连接
 */
bool app_state_is_wifi_connected(void) {
    bool ret;
    xSemaphoreTake(g.mux, portMAX_DELAY);
    ret = g.wifi_connected;
    xSemaphoreGive(g.mux);
    return ret;
}

/**
 * @brief 设置应用时间
 *
 * @param hour 小时，范围 0-23
 * @param min 分钟，范围 0-59
 */
void app_state_set_time(uint8_t hour, uint8_t min) {
    xSemaphoreTake(g.mux, portMAX_DELAY);
    bool changed = (g.time.hour != hour) || (g.time.min != min);
    g.time.hour = hour;
    g.time.min = min;
    xSemaphoreGive(g.mux);

    if (changed) {
        esp_event_post(APP_EVENT, APP_STATE_TIME_CHANGED, NULL, 0, 0);
    }
}

/**
 * @brief 获取应用时间
 *
 * @return app_time_t 应用时间信息
 */
app_time_t app_state_get_time(void) {
    app_time_t ret;
    xSemaphoreTake(g.mux, portMAX_DELAY);
    ret = g.time;
    xSemaphoreGive(g.mux);
    return ret;
}