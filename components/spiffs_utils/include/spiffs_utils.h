#ifndef SPIFFS_UTIL
#define SPIFFS_UTIL

#include "esp_err.h"
#include "esp_log.h"



esp_err_t spiffs_utils_init(const char*,char* path);

#endif