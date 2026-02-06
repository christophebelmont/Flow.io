#pragma once
/**
 * @file EventPayloads.h
 * @brief Payload types used by EventBus events.
 */
#include <stdint.h>

// Keep payloads small and trivially copyable.
// EventBus will copy payload bytes into its queue buffer.

/** @brief Payload for ConfigChanged events. */
struct ConfigChangedPayload {
    char nvsKey[32];
};

/** @brief Payload carrying network readiness information. */
struct WifiNetReadyPayload {
  uint8_t ip[4];
  uint8_t gw[4];
  uint8_t mask[4];
};

/** @brief Payload for sensor update notifications. */
struct SensorsUpdatedPayload {
    uint32_t tsMs;
};

/** @brief Payload for relay change events. */
struct RelayChangedPayload {
    uint8_t relayId;
    uint8_t state; // 0/1
};

/** @brief Payload for pool mode changes. */
struct PoolModeChangedPayload {
    uint8_t mode;
};

/** @brief Payload for alarm events. */
struct AlarmPayload {
    uint16_t alarmId;
};

/** @brief Identifiers for DataStore values. */
using DataKey = uint16_t;

/** @brief Payload for data change events. */
struct DataChangedPayload {
    DataKey id;
};

/** @brief Dirty flags for snapshot payloads. */
enum DirtyFlags : uint32_t {
    DIRTY_NONE    = 0,
    DIRTY_NETWORK = 1 << 0,
    DIRTY_TIME    = 1 << 1,
    DIRTY_MQTT    = 1 << 2,
    DIRTY_SENSORS = 1 << 3,
};

/** @brief Payload indicating a new data snapshot. */
struct DataSnapshotPayload {
    uint32_t dirtyFlags;
};
