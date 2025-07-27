#ifndef WIFI_TIME_H
#define WIFI_TIME_H

#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_sntp.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#define CONFIG_WIFI_SSID "204小居-书房"
#define CONFIG_WIFI_PASSWORD "lhcj@1024"
#define CONFIG_TIMEZONE "CST-8"

esp_err_t wifi_time_init(void);

#endif