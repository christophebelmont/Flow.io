#pragma once
/**
 * @file WifiModuleDataModel.h
 * @brief Wifi runtime data model contribution.
 */
#include "Core/Types/IpV4.h"

/** @brief WiFi runtime status. */
struct WifiRuntimeData {
    bool ready = false;
    IpV4 ip {0,0,0,0};
};

// MODULE_DATA_MODEL: WifiRuntimeData wifi
