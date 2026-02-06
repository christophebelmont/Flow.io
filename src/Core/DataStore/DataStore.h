#pragma once
/**
 * @file DataStore.h
 * @brief Runtime data store with EventBus notifications.
 */
#include <stdint.h>
#include <string.h>

#include "Core/DataModel.h"
#include "Core/EventBus/EventBus.h"
#include "Core/EventBus/EventId.h"
#include "Core/EventBus/EventPayloads.h"

/**
 * @brief Stores runtime data and publishes changes on the EventBus.
 */
class DataStore {
public:
    /** @brief Construct an empty data store. */
    DataStore() = default;

    /** @brief Inject EventBus dependency for notifications. */
    void setEventBus(EventBus* bus) { _bus = bus; }

    /** @brief Safe read access to the full runtime model. */
    const RuntimeData& data() const { return _rt; }
    /** @brief Mutable access for module-owned setters. */
    RuntimeData& dataMutable() { return _rt; }

    /** @brief Notify a data change with dirty flags. */
    void notifyChanged(DataKey key, uint32_t dirtyMask);

    /** @brief Current dirty flags bitmask. */
    uint32_t dirtyFlags() const { return _dirtyFlags; }
    /** @brief Consume and clear dirty flags. */
    uint32_t consumeDirtyFlags();

private:
    RuntimeData _rt{};
    EventBus* _bus = nullptr;

    volatile uint32_t _dirtyFlags = DIRTY_NONE;

private:
    void markDirty(uint32_t mask);
    void publishChanged(DataKey key);
    void publishSnapshot();
};
