#pragma once
/**
 * @file IDomainStatus.h
 * @brief Shared domain-slot runtime health service.
 */

#include <stdint.h>

#include "Domain/DomainTypes.h"
#include "Core/Services/IIO.h"
#include "Core/Services/IPoolDevice.h"

enum class DomainSlotRuntimeState : uint8_t {
    Sleeping = 0,
    Active,
    Error
};

enum class DomainSlotErrorReason : uint8_t {
    None = 0,
    Unbound,
    NotConfigured,
    NoBinding,
    Disabled,
    NoValidValue,
    PoolDeviceBlocked,
    ReadFailed
};

struct DomainSlotStatus {
    DomainSlotId domainSlot = DOMAIN_SLOT_INVALID;
    IoSlotId ioSlot = IO_SLOT_INVALID;
    IoId ioId = IO_ID_INVALID;
    DomainSlotRuntimeState state = DomainSlotRuntimeState::Sleeping;
    DomainSlotErrorReason errorReason = DomainSlotErrorReason::None;
    uint8_t active = 0U;
    uint8_t error = 0U;
    uint8_t hasMeta = 0U;
    uint8_t hasValue = 0U;
    uint8_t hasBindingPort = 0U;
    uint8_t hasPoolDevice = 0U;
    IoEndpointMeta meta{};
    IoValue value{};
    PoolDeviceSvcMeta poolMeta{};
    uint8_t poolActualOn = 0U;
    uint32_t poolActualTsMs = 0U;
};

struct DomainStatusSummary {
    uint16_t total = 0U;
    uint16_t active = 0U;
    uint16_t sleeping = 0U;
    uint16_t error = 0U;
};

struct DomainStatusService {
    bool (*slotStatus)(void* ctx, DomainSlotId domainSlot, DomainSlotStatus* outStatus);
    bool (*summary)(void* ctx, DomainStatusSummary* outSummary);
    bool (*hasDomainSlotError)(void* ctx);
    bool (*firstError)(void* ctx, DomainSlotStatus* outStatus);
    void* ctx;
};

constexpr const char* domainSlotRuntimeStateName(DomainSlotRuntimeState state)
{
    switch (state) {
        case DomainSlotRuntimeState::Sleeping: return "sleeping";
        case DomainSlotRuntimeState::Active: return "active";
        case DomainSlotRuntimeState::Error: return "error";
    }
    return "sleeping";
}

constexpr const char* domainSlotErrorReasonName(DomainSlotErrorReason reason)
{
    switch (reason) {
        case DomainSlotErrorReason::None: return "";
        case DomainSlotErrorReason::Unbound: return "unbound";
        case DomainSlotErrorReason::NotConfigured: return "not_configured";
        case DomainSlotErrorReason::NoBinding: return "no_binding";
        case DomainSlotErrorReason::Disabled: return "disabled";
        case DomainSlotErrorReason::NoValidValue: return "no_valid_value";
        case DomainSlotErrorReason::PoolDeviceBlocked: return "pool_device_blocked";
        case DomainSlotErrorReason::ReadFailed: return "read_failed";
    }
    return "unknown";
}
