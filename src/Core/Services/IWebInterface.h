#pragma once
/**
 * @file IWebInterface.h
 * @brief Web interface runtime control service.
 */

#include <stddef.h>

struct WebInterfaceService {
    bool (*setPaused)(void* ctx, bool paused);
    bool (*isPaused)(void* ctx);
    void* ctx;
};
