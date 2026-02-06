/**
 * @file DataStore.cpp
 * @brief Implementation file.
 */
#include "Core/DataStore/DataStore.h"
#define LOG_TAG_CORE "DataStor"

void DataStore::markDirty(uint32_t mask)
{
    _dirtyFlags |= mask;
}

uint32_t DataStore::consumeDirtyFlags()
{
    uint32_t f = _dirtyFlags;
    _dirtyFlags = DIRTY_NONE;
    return f;
}

void DataStore::publishChanged(DataKey key)
{
    if (!_bus) return;
    DataChangedPayload p{ key };
    _bus->post(EventId::DataChanged, &p, sizeof(p));
}

void DataStore::publishSnapshot()
{
    if (!_bus) return;
    DataSnapshotPayload p{ (uint32_t)_dirtyFlags };
    _bus->post(EventId::DataSnapshotAvailable, &p, sizeof(p));
}

void DataStore::notifyChanged(DataKey key, uint32_t dirtyMask)
{
    markDirty(dirtyMask);
    publishChanged(key);
    publishSnapshot();
}
