# SystemLimits

Flow.IO centralizes compile-time limits under `include/Core/SystemLimits.h`.

## Why SystemLimits

SystemLimits defines:
- bounded JSON document sizes
- bounded buffers for topics, payloads, identifiers
- bounded queue sizes
- timing constants and backoff profiles

This ensures every module uses explicit, reviewable constraints.

## Example: MQTT limits

```cpp
namespace Limits::Mqtt::Buffers {
  constexpr size_t Topic = 128;
  constexpr size_t RxPayload = 384;
}
namespace Limits::Mqtt::Timing {
  constexpr uint32_t LoopDelayMs = 50;
}
```

## Example: global limits

```cpp
namespace Limits {
  constexpr size_t JsonCmdBuf = 1024;
  constexpr uint8_t EventQueueLen = 16;
}
```
