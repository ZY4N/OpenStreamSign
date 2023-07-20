#pragma once

#include <obs-module.h>

#define PLUGIN_NAME "obs-plugintemplate"

#define logger_info(fmt, ...) blog(LOG_INFO, "[" PLUGIN_NAME "] " fmt, ##__VA_ARGS__)

#define logger_debug(fmt, ...) blog(LOG_DEBUG, "[" PLUGIN_NAME "] " fmt, ##__VA_ARGS__)

#define logger_warn(fmt, ...) blog(LOG_WARNING, "[" PLUGIN_NAME "] " fmt, ##__VA_ARGS__)

#define logger_error(fmt, ...) blog(LOG_ERROR, "[" PLUGIN_NAME "] " fmt, ##__VA_ARGS__)
