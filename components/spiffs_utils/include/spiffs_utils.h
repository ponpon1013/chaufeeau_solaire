#ifndef SPIFFS_UTIL
#define SPIFFS_UTIL

#include "esp_err.h"
#include "esp_log.h"

#define SPIFFS_WIFI "storage_wifi"

esp_err_t spiffs_utils_init();

#endif