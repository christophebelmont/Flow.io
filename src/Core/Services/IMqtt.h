#pragma once
/**
 * @file IMqtt.h
 * @brief MQTT service interface.
 */

#include <stddef.h>

/** @brief Service wrapper for publishing and topic formatting via MQTTModule. */
struct MqttService {
    bool (*publish)(void* ctx, const char* topic, const char* payload, int qos, bool retain);
    void (*formatTopic)(void* ctx, const char* suffix, char* out, size_t outLen);
    bool (*isConnected)(void* ctx);
    void* ctx;
};
