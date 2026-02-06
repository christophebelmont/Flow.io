#pragma once
/**
 * @file INTP.h
 * @brief Time/NTP service interface.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>

/** @brief NTP synchronization state. */
enum class NTPState : uint8_t { Disabled, WaitingNetwork, Syncing, Synced, ErrorWait };

/** @brief Service interface for time synchronization. */
struct TimeService {
    NTPState (*state)(void* ctx);
    bool (*isSynced)(void* ctx);
    uint64_t (*epoch)(void* ctx);
    bool (*formatLocalTime)(void* ctx, char* out, size_t len);
    void* ctx;
};
