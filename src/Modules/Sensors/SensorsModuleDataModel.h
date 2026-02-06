#pragma once
/**
 * @file SensorsModuleDataModel.h
 * @brief Sensors runtime data model contribution.
 */

struct SensorsRuntimeData {
    float ph = 0.0f;
    float orp = 0.0f;
    float psi = 0.0f;
    float waterTemp = 0.0f;
    float airTemp = 0.0f;
};

// MODULE_DATA_MODEL: SensorsRuntimeData sensors
