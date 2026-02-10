#pragma once
/**
 * @file ITime.h
 * @brief Generic time synchronization service interface.
 */
#include <stddef.h>
#include <stdint.h>

/** @brief Time synchronization state. */
enum class TimeSyncState : uint8_t { Disabled, WaitingNetwork, Syncing, Synced, ErrorWait };

/** @brief Backward compatibility alias. */
using NTPState = TimeSyncState;

/** @brief Service interface for time synchronization and formatting. */
struct TimeService {
    TimeSyncState (*state)(void* ctx);
    bool (*isSynced)(void* ctx);
    uint64_t (*epoch)(void* ctx);
    bool (*formatLocalTime)(void* ctx, char* out, size_t len);
    void* ctx;
};
