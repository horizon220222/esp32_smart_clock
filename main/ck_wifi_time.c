#include "ck_wifi_time.h"
#include <esp_netif_sntp.h>
#include "ck_state.h"
#include "esp_wifi.h"
#include "esp_sntp.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"

static const char *TAG = "ck_wifi_time";

/**
 * @brief SNTP 时间同步回调函数
 *
 * @param tv 时间同步完成后的时间信息
 */
static void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "SNTP 时间同步完成");
    setenv("TZ", CONFIG_TIMEZONE, 1);
    tzset();
}

/**
 * @brief Wi-Fi 事件处理函数
 *
 * @param arg 事件处理函数参数
 * @param event_base 事件基础
 * @param event_id 事件 ID
 * @param event_data 事件数据
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Wi-Fi 正在连接...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi 断开连接，正在重试...");
        app_state_set_wifi(false);
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "已获取 IP 地址，正在启动 SNTP...");
        app_state_set_wifi(true);

        esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, "ntp.aliyun.com");
        esp_sntp_setservername(1, "cn.pool.ntp.org");
        esp_sntp_setservername(2, "time.ustc.edu.cn");
        esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
        esp_sntp_init();
        ESP_LOGI(TAG, "SNTP 已启用");
    }
}

/**
 * @brief 初始化 Wi-Fi 时间模块
 *
 * @return esp_err_t 初始化结果，ESP_OK 表示成功，其他值表示失败
 */
esp_err_t wifi_time_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGI(TAG, "创建事件循环失败");
        return err;
    }
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
            .sta = {
                    .ssid = CONFIG_WIFI_SSID,
                    .password = CONFIG_WIFI_PASSWORD,
                    .threshold.authmode = WIFI_AUTH_WPA2_PSK,
                    .pmf_cfg = {
                            .capable = true,
                            .required = false
                    },
            },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "wifi初始化完成.");
    return ESP_OK;
}
