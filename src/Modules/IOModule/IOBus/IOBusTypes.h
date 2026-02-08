#pragma once
/**
 * @file IOBusTypes.h
 * @brief Shared IO bus enums.
 */

#include <stdint.h>

enum IOBusKind : uint8_t {
    IO_BUS_I2C = 0,
    IO_BUS_ONEWIRE = 1,
    IO_BUS_GPIO = 2
};
