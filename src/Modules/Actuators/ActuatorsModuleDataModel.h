#pragma once
/**
 * @file ActuatorsModuleDataModel.h
 * @brief Actuators runtime data model contribution.
 */

#include <stdint.h>

constexpr uint8_t ACTUATOR_MAX = 6;

struct ActuatorRuntime {
    bool on = false;
    bool commanded = false;
    uint32_t uptimeMs = 0;
    float tankFillPct = 100.0f;
};

struct ActuatorsRuntimeData {
    ActuatorRuntime slots[ACTUATOR_MAX];
};

// MODULE_DATA_MODEL: ActuatorsRuntimeData actuators
