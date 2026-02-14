#pragma once

#include <stdint.h>

#include "Board/BoardPinMap.h"
#include "Core/SystemLimits.h"

struct DigitalOutDef {
    const char* name;
    uint8_t pin;
    bool momentary;
    uint16_t pulseMs;
};

struct DigitalInDef {
    const char* name;
    uint8_t pin;
};

namespace BoardLayout {

constexpr DigitalOutDef DOs[] = {
    {"filtration", Board::DO::Pump, false, 0},
    {"ph_pump", Board::DO::Heater, false, 0},
    {"chlorine_pump", Board::DO::Light, false, 0},
    {"chlorine_generator", Board::DO::Aux1, true, Limits::MomentaryPulseMs},
    {"robot", Board::DO::Aux2, false, 0},
    {"lights", Board::DO::Aux3, false, 0},
    {"fill_pump", Board::DO::Aux4, false, 0},
    {"water_heater", Board::DO::Aux5, false, 0},
};

constexpr DigitalInDef DIs[] = {
    {"flow", Board::DI::FlowSwitch},
};

constexpr uint8_t DigitalOutCount = (uint8_t)(sizeof(DOs) / sizeof(DOs[0]));
constexpr uint8_t DigitalInCount = (uint8_t)(sizeof(DIs) / sizeof(DIs[0]));

}  // namespace BoardLayout
