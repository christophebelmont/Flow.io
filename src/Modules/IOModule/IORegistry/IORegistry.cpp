/**
 * @file IORegistry.cpp
 * @brief Implementation file.
 */

#include "IORegistry.h"

bool IORegistry::add(IOEndpoint* endpoint)
{
    if (!endpoint) return false;
    if (count_ >= IO_REGISTRY_MAX_ENDPOINTS) return false;
    endpoints_[count_++] = endpoint;
    return true;
}

IOEndpoint* IORegistry::find(const char* id) const
{
    if (!id) return nullptr;

    for (uint8_t i = 0; i < count_; ++i) {
        IOEndpoint* e = endpoints_[i];
        if (!e || !e->id()) continue;
        if (strcmp(e->id(), id) == 0) return e;
    }

    return nullptr;
}

IOEndpoint* IORegistry::at(uint8_t i) const
{
    if (i >= count_) return nullptr;
    return endpoints_[i];
}

bool IORegistry::read(const char* id, IOEndpointValue& out) const
{
    IOEndpoint* e = find(id);
    if (!e) return false;
    return e->read(out);
}

bool IORegistry::write(const char* id, const IOEndpointValue& in) const
{
    IOEndpoint* e = find(id);
    if (!e) return false;
    return e->write(in);
}
