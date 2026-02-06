#pragma once
/**
 * @file NTPModuleDataModel.h
 * @brief NTP runtime data model contribution.
 */

/** @brief NTP runtime status. */
struct NTPRuntimeData {
    bool timeReady = false;
};

// MODULE_DATA_MODEL: NTPRuntimeData ntp
