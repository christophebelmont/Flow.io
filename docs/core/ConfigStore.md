# ConfigStore

## Purpose

ConfigStore provides:
- typed configuration registry (`ConfigVariable<T>`) per module
- persistence to NVS (Preferences namespace)
- JSON patch application (`cfg/set` style)

## NVS keys

Flow.IO centralizes NVS key names in `include/Core/NvsKeys.h`.

Example (MQTT module):

```cpp
ConfigVariable<char,0> hostVar {
  NVS_KEY(NvsKeys::Mqtt::Host), "host", "mqtt",
  ConfigType::CharArray, (char*)cfgData.host,
  ConfigPersistence::Persistent, sizeof(cfgData.host)
};
```

## Registration

Modules register variables in `init()`:

```cpp
cfg.registerVar(&hostVar);
cfg.registerVar(&enabledVar);
```

## JSON Apply

Config patches are applied using a bounded JSON document capacity defined in `SystemLimits.h`:

- `Limits::JsonConfigApplyBuf`

This allows multi-module config patches in one payload.

## ConfigChanged events

When a key changes, ConfigStore emits `EventType::ConfigChanged` with `ConfigChangedPayload` containing `nvsKey`.

Modules can subscribe via EventBus to update derived state.
