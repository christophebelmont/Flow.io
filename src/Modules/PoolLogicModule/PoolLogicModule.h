#pragma once
/**
 * @file PoolLogicModule.h
 * @brief Business logic module that computes filtration schedule from water temperature.
 */
#include "Core/ModulePassive.h"
#include "Core/Services/Services.h"
#include "Core/ConfigTypes.h"

/** @brief Event ids owned by PoolLogicModule. */
constexpr uint16_t POOLLOGIC_EVENT_DAILY_RECALC = 0x2101;
constexpr uint16_t POOLLOGIC_EVENT_FILTRATION_WINDOW = 0x2102;

class PoolLogicModule : public ModulePassive {
public:
    const char* moduleId() const override { return "poollogic"; }

    uint8_t dependencyCount() const override { return 4; }
    const char* dependency(uint8_t i) const override {
        if (i == 0) return "loghub";
        if (i == 1) return "datastore";
        if (i == 2) return "eventbus";
        if (i == 3) return "time";
        return nullptr;
    }

    void init(ConfigStore& cfg, ServiceRegistry& services) override;

private:
    static constexpr uint8_t SLOT_DAILY_RECALC = 3;
    static constexpr uint8_t SLOT_FILTR_WINDOW = 4;
    static constexpr uint8_t WATER_TEMP_IO_INDEX_DEFAULT = 3;

    float waterTempLowThreshold_ = 12.0f;
    float waterTempSetpoint_ = 24.0f;
    uint8_t filtrationStartMin_ = 8;
    uint8_t filtrationStopMax_ = 23;
    uint8_t waterTempIoIndex_ = WATER_TEMP_IO_INDEX_DEFAULT;
    bool enabled_ = true;

    ConfigStore* cfgStore_ = nullptr;
    DataStore* dataStore_ = nullptr;
    EventBus* eventBus_ = nullptr;
    const TimeSchedulerService* schedSvc_ = nullptr;
    const LogHubService* logHub_ = nullptr;

    ConfigVariable<bool,0> enabledVar_{
        NVS_KEY("pl_en"), "enabled", "poollogic", ConfigType::Bool,
        &enabled_, ConfigPersistence::Persistent, 0
    };
    ConfigVariable<float,0> tempLowVar_{
        NVS_KEY("pl_tlow"), "water_temp_low_threshold", "poollogic", ConfigType::Float,
        &waterTempLowThreshold_, ConfigPersistence::Persistent, 0
    };
    ConfigVariable<float,0> tempSetpointVar_{
        NVS_KEY("pl_tset"), "water_temp_setpoint", "poollogic", ConfigType::Float,
        &waterTempSetpoint_, ConfigPersistence::Persistent, 0
    };
    ConfigVariable<uint8_t,0> startMinVar_{
        NVS_KEY("pl_smin"), "filtration_start_min", "poollogic", ConfigType::UInt8,
        &filtrationStartMin_, ConfigPersistence::Persistent, 0
    };
    ConfigVariable<uint8_t,0> stopMaxVar_{
        NVS_KEY("pl_smax"), "filtration_stop_max", "poollogic", ConfigType::UInt8,
        &filtrationStopMax_, ConfigPersistence::Persistent, 0
    };
    ConfigVariable<uint8_t,0> waterTempIdxVar_{
        NVS_KEY("pl_widx"), "water_temp_io_idx", "poollogic", ConfigType::UInt8,
        &waterTempIoIndex_, ConfigPersistence::Persistent, 0
    };

    static void onEventStatic_(const Event& e, void* user);
    void onEvent_(const Event& e);

    void ensureDailySlot_();
    bool computeFiltrationWindow_(float waterTemp, uint8_t& startHourOut, uint8_t& stopHourOut, uint8_t& durationOut);
    void recalcAndApplyFiltrationWindow_();
};

