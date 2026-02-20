#pragma once
/**
 * @file IConfig.h
 * @brief Config store service interface.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>

/** @brief Service interface for config JSON import/export. */
struct ConfigStoreService {
    bool (*applyJson)(void* ctx, const char* json);
    void (*toJson)(void* ctx, char* out, size_t outLen);
    bool (*toJsonModule)(void* ctx, const char* module, char* out, size_t outLen, bool* truncated);
    uint8_t (*listModules)(void* ctx, const char** out, uint8_t max);
    bool (*erase)(void* ctx);
    void* ctx;
};
