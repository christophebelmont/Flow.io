#pragma once
/**
 * @file ConfigBranchIds.h
 * @brief Stable config branch identifiers used for cfg/* routing.
 */

#include <stdint.h>

enum class ConfigBranchId : uint16_t {
    Unknown = 0,

    Wifi = 1,
    Mqtt = 2,
    Ha = 3,
    Time = 4,
    TimeScheduler = 5,
    SystemMonitor = 6,
    PoolLogic = 7,
    Alarms = 8,

    Io = 16,
    IoDebug = 17,
    IoInputA0 = 32,
    IoInputA1 = 33,
    IoInputA2 = 34,
    IoInputA3 = 35,
    IoInputA4 = 36,
    IoInputA5 = 37,
    IoOutputD0 = 48,
    IoOutputD1 = 49,
    IoOutputD2 = 50,
    IoOutputD3 = 51,
    IoOutputD4 = 52,
    IoOutputD5 = 53,
    IoOutputD6 = 54,
    IoOutputD7 = 55,

    PoolDevicePd0 = 64,
    PoolDevicePd1 = 65,
    PoolDevicePd2 = 66,
    PoolDevicePd3 = 67,
    PoolDevicePd4 = 68,
    PoolDevicePd5 = 69,
    PoolDevicePd6 = 70,
    PoolDevicePd7 = 71
};

constexpr ConfigBranchId configBranchFromPoolDeviceSlot(uint8_t slot)
{
    if (slot > 7U) return ConfigBranchId::Unknown;
    return static_cast<ConfigBranchId>((uint16_t)ConfigBranchId::PoolDevicePd0 + slot);
}

inline const char* configBranchModuleName(ConfigBranchId id)
{
    switch (id) {
        case ConfigBranchId::Wifi: return "wifi";
        case ConfigBranchId::Mqtt: return "mqtt";
        case ConfigBranchId::Ha: return "ha";
        case ConfigBranchId::Time: return "time";
        case ConfigBranchId::TimeScheduler: return "time/scheduler";
        case ConfigBranchId::SystemMonitor: return "sysmon";
        case ConfigBranchId::PoolLogic: return "poollogic";
        case ConfigBranchId::Alarms: return "alarms";
        case ConfigBranchId::Io: return "io";
        case ConfigBranchId::IoDebug: return "io/debug";
        case ConfigBranchId::IoInputA0: return "io/input/a0";
        case ConfigBranchId::IoInputA1: return "io/input/a1";
        case ConfigBranchId::IoInputA2: return "io/input/a2";
        case ConfigBranchId::IoInputA3: return "io/input/a3";
        case ConfigBranchId::IoInputA4: return "io/input/a4";
        case ConfigBranchId::IoInputA5: return "io/input/a5";
        case ConfigBranchId::IoOutputD0: return "io/output/d0";
        case ConfigBranchId::IoOutputD1: return "io/output/d1";
        case ConfigBranchId::IoOutputD2: return "io/output/d2";
        case ConfigBranchId::IoOutputD3: return "io/output/d3";
        case ConfigBranchId::IoOutputD4: return "io/output/d4";
        case ConfigBranchId::IoOutputD5: return "io/output/d5";
        case ConfigBranchId::IoOutputD6: return "io/output/d6";
        case ConfigBranchId::IoOutputD7: return "io/output/d7";
        case ConfigBranchId::PoolDevicePd0: return "pdm/pd0";
        case ConfigBranchId::PoolDevicePd1: return "pdm/pd1";
        case ConfigBranchId::PoolDevicePd2: return "pdm/pd2";
        case ConfigBranchId::PoolDevicePd3: return "pdm/pd3";
        case ConfigBranchId::PoolDevicePd4: return "pdm/pd4";
        case ConfigBranchId::PoolDevicePd5: return "pdm/pd5";
        case ConfigBranchId::PoolDevicePd6: return "pdm/pd6";
        case ConfigBranchId::PoolDevicePd7: return "pdm/pd7";
        case ConfigBranchId::Unknown:
        default:
            return nullptr;
    }
}
