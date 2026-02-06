#pragma once
/**
 * @file IpV4.h
 * @brief IPv4 address container type.
 */
#include <stdint.h>

/** @brief IPv4 address container. */
struct IpV4 {
    uint8_t b[4];
};
