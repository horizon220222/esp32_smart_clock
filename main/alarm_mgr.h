#ifndef ALARM_MGR_H
#define ALARM_MGR_H

#include "esp_err.h"

#define ALARM_NVS_KEY "alarms_mgr"
#define ALARM_MAX 10

typedef struct {
    uint8_t hour;
    uint8_t min;
} alarm_t;

esp_err_t alarm_mgr_init(void);
esp_err_t alarm_add(uint8_t hour, uint8_t min);
esp_err_t alarm_del(uint8_t idx);
const alarm_t *alarm_list(size_t *cnt);

#endif