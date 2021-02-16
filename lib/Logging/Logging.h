#pragma once

#include <Arduino.h>


#define ESP_ALARM_LOG_FORMAT(format)  "[A][%s:%u] %s(): " format "\r\n", pathToFileName(__FILE__), __LINE__, __FUNCTION__


#define log_a(format, ...) log_printf(ESP_ALARM_LOG_FORMAT(format), ##__VA_ARGS__)
