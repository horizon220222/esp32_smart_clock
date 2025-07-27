#ifndef APP_STATE_H_
#define APP_STATE_H_
#include "esp_event.h"

typedef enum {
    APP_STATE_TIME_CHANGED,     // 时间已更新，当系统时间发生变化时触发此事件
    APP_STATE_ALARM_FIRED,      // 闹钟触发，当设定的闹钟时间到达时触发此事件
    APP_STATE_WIFI_CONNECTED,   // Wi-Fi 已连，当设备成功连接到 Wi-Fi 网络时触发此事件
    APP_STATE_WIFI_DISCONNECTED // Wi-Fi 断开，当设备与 Wi-Fi 网络断开连接时触发此事件
} app_state_event_id_t;

typedef struct {
    uint8_t hour;  // 小时，范围 0 - 23
    uint8_t min;   // 分钟，范围 0 - 59
} app_time_t;

ESP_EVENT_DECLARE_BASE(APP_EVENT);

esp_err_t app_state_init(void);
void app_state_set_wifi(bool connected);
bool app_state_is_wifi_connected(void);
void app_state_set_time(uint8_t hour, uint8_t min);
app_time_t app_state_get_time(void);

#endif