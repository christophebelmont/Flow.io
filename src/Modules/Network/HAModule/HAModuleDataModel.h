#pragma once
/**
 * @file HAModuleDataModel.h
 * @brief Home Assistant runtime data model contribution.
 */

struct HARuntimeData {
    bool autoconfigPublished = false;
    char vendor[32] = {0};
    char deviceId[32] = {0};
};

// MODULE_DATA_MODEL: HARuntimeData ha
