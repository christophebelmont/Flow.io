#pragma once
/**
 * @file ActuatorVirtualDriver.h
 * @brief Virtual sensor driver reading actuator runtime values.
 */
#include "Modules/Sensors/Engine/ISensor.h"
#include "Core/DataStore/DataStore.h"
#include "Modules/Actuators/ActuatorsRuntime.h"

class ActuatorVirtualDriver : public ISensor {
public:
    enum class Metric : uint8_t { TankFillPct = 0, UptimeHours = 1 };

    ActuatorVirtualDriver(const char* sensorName, DataStore* store, uint8_t index, Metric metric)
        : name_(sensorName), ds_(store), idx_(index), metric_(metric) {}

    const char* name() const override { return name_; }
    SensorReading read() override;

private:
    const char* name_ = nullptr;
    DataStore* ds_ = nullptr;
    uint8_t idx_ = 0;
    Metric metric_ = Metric::TankFillPct;
};
