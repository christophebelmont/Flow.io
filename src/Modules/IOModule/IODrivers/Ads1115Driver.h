#pragma once
/**
 * @file Ads1115Driver.h
 * @brief ADS1115 async driver wrapper.
 */

#include <ADS1X15.h>
#include <stdint.h>
#include "Modules/IOModule/IOBus/I2CBus.h"
#include "Modules/IOModule/IODrivers/IODriver.h"

struct Ads1115DriverConfig {
    uint8_t address = 0x48;
    uint8_t gain = ADS1X15_GAIN_6144MV;
    uint8_t dataRate = 1;
    uint32_t pollMs = 125;
    bool differentialPairs = false;
    float mvLsb = 0.1875f;
};

class Ads1115Driver : public IODriver {
public:
    Ads1115Driver(const char* driverId, I2CBus* bus, const Ads1115DriverConfig& cfg);

    const char* id() const override { return driverId_; }
    bool begin() override;
    void tick(uint32_t nowMs) override;

    bool readMilliVoltsChannel(uint8_t ch, float& outMv) const;
    bool readMilliVoltsDifferential01(float& outMv) const;
    bool readMilliVoltsDifferential23(float& outMv) const;

private:
    void requestNext_();
    float toMilliVolts_(int16_t raw);

    const char* driverId_ = nullptr;
    I2CBus* bus_ = nullptr;
    Ads1115DriverConfig cfg_{};

    ADS1115 ads_;
    bool ready_ = false;
    uint32_t lastTickMs_ = 0;

    bool requested_ = false;
    uint8_t nextSingleCh_ = 0;
    uint8_t nextDiffPair_ = 0;

    bool validSingle_[4] = {false, false, false, false};
    float mvSingle_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    bool validDiff01_ = false;
    bool validDiff23_ = false;
    float mvDiff01_ = 0.0f;
    float mvDiff23_ = 0.0f;
};
