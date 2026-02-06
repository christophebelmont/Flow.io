/**
 * @file ModuleLog.h
 * @brief Convenience logging macros for modules.
 */
#pragma once

#include "Core/Log.h"

// Ensure a default tag if not defined by the module.
#ifndef LOG_TAG
#define LOG_TAG "ModuleLg"
#endif

#undef LOGD
#undef LOGI
#undef LOGW
#undef LOGE

#define LOGD(...) ::Log::debug(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ::Log::info(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ::Log::warn(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ::Log::error(LOG_TAG, __VA_ARGS__)
