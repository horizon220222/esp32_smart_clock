#ifndef SERVO_DISPLAY_H
#define SERVO_DISPLAY_H

#include "esp_err.h"

esp_err_t servo_display_init(void);
void servo_display_set(const char *str);   // 仅支持 "HHMM" 或 "HH:MM"

#endif