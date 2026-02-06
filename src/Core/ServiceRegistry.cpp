/**
 * @file ServiceRegistry.cpp
 * @brief Implementation file.
 */
#include "ServiceRegistry.h"
#define LOG_TAG_CORE "SvcRegst"

bool ServiceRegistry::add(const char* id, const void* service) {
    if (count >= MAX_SERVICES) return false;
    entries[count++] = {id, service};
    return true;
}

const void* ServiceRegistry::getRaw(const char* id) const {
    for (uint8_t i = 0; i < count; ++i) {
        if (strcmp(entries[i].id, id) == 0)
            return entries[i].ptr;
    }
    return nullptr;
}
