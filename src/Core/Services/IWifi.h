#pragma once
/**
 * @file IWifi.h
 * @brief WiFi service interface.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>

/** @brief WiFi connection state. */
enum class WifiState : uint8_t {
    Disabled,
    Idle,
    Connecting,
    Connected,
    ErrorWait
};

/** @brief Service interface for WiFi status. */
struct WifiService {
    WifiState (*state)(void* ctx);
    bool (*isConnected)(void* ctx);
    bool (*getIP)(void* ctx, char* out, size_t len);
    void* ctx;
    bool (*requestReconnect)(void* ctx);
    bool (*requestScan)(void* ctx, bool force);
    bool (*scanStatusJson)(void* ctx, char* out, size_t outLen);
};
