#pragma once
/**
 * @file ServiceRegistry.h
 * @brief Typed service registry for cross-module access.
 */
#include <stdint.h>
#include <cstring>

/** @brief Maximum number of registered services. */
constexpr uint8_t MAX_SERVICES = 16;

/** @brief Raw registry entry. */
struct ServiceEntry {
    const char* id;
    const void* ptr;
};

/**
 * @brief Registry of named services (opaque pointers).
 */
class ServiceRegistry {
public:
    /** @brief Register a service pointer under a string id. */
    bool add(const char* id, const void* service);
    /** @brief Fetch a raw service pointer by id. */
    const void* getRaw(const char* id) const;

    /** @brief Fetch a typed service pointer by id. */
    template<typename T>
    const T* get(const char* id) const {
        return reinterpret_cast<const T*>(getRaw(id));
    }

private:
    ServiceEntry entries[MAX_SERVICES]{};
    uint8_t count = 0;
};
