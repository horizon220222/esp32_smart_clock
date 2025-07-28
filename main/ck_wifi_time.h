#ifndef _CK_WIFI_TIME_H
#define _CK_WIFI_TIME_H

#include "esp_err.h"

#define CONFIG_WIFI_SSID "204小居-书房"
#define CONFIG_WIFI_PASSWORD "lhcj@1024"
#define CONFIG_TIMEZONE "CST-8"

esp_err_t wifi_time_init(void);

#endif