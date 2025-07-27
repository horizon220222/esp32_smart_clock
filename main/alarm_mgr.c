#include "alarm_mgr.h"
#include "app_state.h"
#include "cJSON.h"
#include "freertos/semphr.h"
#include <nvs.h>
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "alarm_mgr";

static struct {
    StaticSemaphore_t mux_buffer;
    SemaphoreHandle_t mux;
    alarm_t alarms[ALARM_MAX];
    size_t alarm_cnt;
} alarm_mgr;

static void load_from_nvs(void)
{
    nvs_handle_t h;
    if (nvs_open("storage", NVS_READONLY, &h) == ESP_OK) {
        size_t len = sizeof(alarm_mgr.alarms);
        nvs_get_blob(h, ALARM_NVS_KEY, alarm_mgr.alarms, &len);
        alarm_mgr.alarm_cnt = len / sizeof(alarm_t);
        nvs_close(h);
    }
}

static void save_to_nvs(void)
{
    nvs_handle_t h;
    if (nvs_open("storage", NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_blob(h, ALARM_NVS_KEY, alarm_mgr.alarms, alarm_mgr.alarm_cnt * sizeof(alarm_t));
        nvs_commit(h);
        nvs_close(h);
    }
}

static void alarm_check_task(void* arg, esp_event_base_t base, int32_t id, void* data)
{
    app_time_t t = app_state_get_time();
    xSemaphoreTake(alarm_mgr.mux, portMAX_DELAY);
    for (size_t i = 0; i < alarm_mgr.alarm_cnt; ++i) {
        if (alarm_mgr.alarms[i].hour == t.hour && alarm_mgr.alarms[i].min == t.min) {
            esp_event_post(APP_EVENT, APP_STATE_ALARM_FIRED, NULL, 0, 0);
        }
    }
    xSemaphoreGive(alarm_mgr.mux);
}

esp_err_t alarm_mgr_init(void)
{
    alarm_mgr.mux = xSemaphoreCreateMutexStatic(&alarm_mgr.mux_buffer);
    if (!alarm_mgr.mux) return ESP_ERR_NO_MEM;

    load_from_nvs();
    esp_event_handler_register(APP_EVENT, APP_STATE_TIME_CHANGED, alarm_check_task, NULL);

    ESP_LOGI(TAG, "应用状态模块已就绪");
    return ESP_OK;
}

esp_err_t alarm_add(uint8_t hour, uint8_t min)
{
    if (alarm_mgr.alarm_cnt >= ALARM_MAX) return ESP_FAIL;
    xSemaphoreTake(alarm_mgr.mux, portMAX_DELAY);
    alarm_mgr.alarms[alarm_mgr.alarm_cnt++] = (alarm_t){ .hour = hour, .min = min };
    save_to_nvs();
    xSemaphoreGive(alarm_mgr.mux);
    return ESP_OK;
}

esp_err_t alarm_del(uint8_t idx)
{
    if (idx >= alarm_mgr.alarm_cnt) return ESP_FAIL;
    xSemaphoreTake(alarm_mgr.mux, portMAX_DELAY);
    alarm_mgr.alarms[idx] = alarm_mgr.alarms[--alarm_mgr.alarm_cnt];
    save_to_nvs();
    xSemaphoreGive(alarm_mgr.mux);
    return ESP_OK;
}

const alarm_t *alarm_list(size_t *cnt)
{
    *cnt = alarm_mgr.alarm_cnt;
    return alarm_mgr.alarms;
}