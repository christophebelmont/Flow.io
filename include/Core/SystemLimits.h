#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Limits {

constexpr size_t TopicBuf = 128;
constexpr size_t JsonPatchBuf = 1024;
constexpr uint8_t MaxRuntimeRoutes = 32;
constexpr uint16_t MomentaryPulseMs = 500;

}  // namespace Limits
