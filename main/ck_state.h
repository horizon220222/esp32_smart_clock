#ifndef _CK_STATE_H_
#define _CK_STATE_H_

#include "esp_event.h"

/**
 * @brief 应用状态事件 ID
 *
 * @note 该枚举类型定义了应用状态模块的事件 ID，用于在应用状态模块中触发事件。
 */
typedef enum {
    APP_STATE_TIME_CHANGED,
    APP_STATE_WIFI_CONNECTED,
    APP_STATE_WIFI_DISCONNECTED
} app_state_event_id_t;

/**
 * @brief 应用状态时间结构体
 *
 * @note 该结构体用于存储应用状态中的时间信息。
 */
typedef struct {
    uint8_t hour;
    uint8_t min;
} app_time_t;

ESP_EVENT_DECLARE_BASE(APP_EVENT);

esp_err_t app_state_init(void);
void app_state_set_wifi(bool connected);
bool app_state_is_wifi_connected(void);
void app_state_set_time(uint8_t hour, uint8_t min);
app_time_t app_state_get_time(void);

#endif