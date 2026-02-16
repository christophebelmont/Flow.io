# MQTTModule

## Summary

**Module ID:** `mqtt`  
**Type:** Active (FreeRTOS task)  
**Declared dependencies:** `loghub`, `wifi`, `cmd`, `time`  
**Exposes services:** `mqtt (MqttService)`  
**Consumes services:** `wifi`, `cmd`, `config`, `time.scheduler`, `eventbus`, `datastore`, `loghub`  

## Purpose

MQTTModule provides the main remote supervision and control interface. It owns the MQTT client, manages connection state, and implements the Flow.IO MQTT protocol:
- command ingress (`cmd`) and acknowledgments (`ack`)
- config patch ingress (`cfg/set`) and acknowledgments (`cfg/ack`)
- retained config blocks publication (`cfg/<module>`)
- runtime payload publishing via a publisher registry

## Configuration

### ConfigStore variables (NVS keys)

- `NvsKeys::Mqtt::Enabled`
- `NvsKeys::Mqtt::Host`
- `NvsKeys::Mqtt::Port`
- `NvsKeys::Mqtt::User`
- `NvsKeys::Mqtt::Pass`
- `NvsKeys::Mqtt::BaseTopic`
- `NvsKeys::Mqtt::SensorMinPublishMs`

Example registration (from the module’s config variables):

```cpp
ConfigVariable<char,0> baseTopicVar {
  NVS_KEY(NvsKeys::Mqtt::BaseTopic), "baseTopic", "mqtt", ConfigType::CharArray,
  (char*)cfgData.baseTopic, ConfigPersistence::Persistent, sizeof(cfgData.baseTopic)
};
```

### SystemLimits

MQTTModule relies heavily on bounded sizes defined in `include/Core/SystemLimits.h`:
- `Limits::Mqtt::Buffers::*` (topics, payloads, IDs)
- `Limits::Mqtt::Capacity::*` (publisher limits)
- `Limits::Mqtt::Timing::*` (loop delay, timeouts)
- `Limits::Mqtt::Backoff::*` (reconnect profile)
- `Limits::JsonCmdBuf`, `Limits::JsonCfgBuf`, `Limits::JsonPatchBuf`

## Topics

MQTT topics follow:

```
<base>/<device>/<suffix>
```

Standard suffixes are defined in `include/Core/MqttTopics.h`.

Topic creation example (from `buildTopics()`):

```cpp
snprintf(topicCmd, sizeof(topicCmd), "%s/%s/%s",
         cfgData.baseTopic, deviceId, MqttTopics::SuffixCmd);
snprintf(topicCfgSet, sizeof(topicCfgSet), "%s/%s/%s",
         cfgData.baseTopic, deviceId, MqttTopics::SuffixCfgSet);
```

Retained config blocks are published to:

```
<base>/<device>/cfg/<module>
```

## Runtime publishing

### Publisher registry

MQTTModule supports a bounded registry of publishers:

```cpp
struct RuntimePublisher {
  const char* topic;
  uint32_t periodMs;
  int qos;
  bool retain;
  uint32_t lastMs;
  bool (*build)(MQTTModule*, char* out, size_t outLen);
};
```

Registering a publisher:

```cpp
mqtt->addRuntimePublisher("sensors", 10000, 0, false,
  [](MQTTModule* self, char* out, size_t outLen) {
    return buildSensorsJson(*self->dataStorePtr(), out, outLen);
  });
```

### Sensor bundle publishing

Sensor publishing is supported through a single “bundle” builder that can use DirtyFlags to throttle output.

```cpp
mqtt->setSensorsPublisher("sensors",
  [](MQTTModule* self, char* out, size_t outLen) {
    return buildSensorBundle(*self->dataStorePtr(), out, outLen);
  });
```

## Command processing

Command payloads use bounded JSON parsing:

```cpp
StaticJsonDocument<Limits::JsonCmdBuf> doc;
auto err = deserializeJson(doc, msg.payload);
if (err) {
  publishRxError_(topicAck, ErrorCode::BadCmdJson, "cmd", true);
  return;
}
const char* cmd = doc["cmd"];
```

The command is dispatched to CommandModule using the CommandService interface.

## DataStore integration

MQTT runtime counters and state are exposed via `Modules/Network/MQTTModule/MQTTRuntime.h`:

- `DataKeys::MqttReady`
- `DataKeys::MqttRxDrop`
- `DataKeys::MqttParseFail`
- `DataKeys::MqttHandlerFail`
- `DataKeys::MqttOversizeDrop`

## EventBus integration

MQTTModule subscribes to:
- `WifiNetReady` to transition out of “waiting network”
- `ConfigChanged` to refresh retained config blocks
- `DataChanged` to trigger sensor/actuator publishing
- `SnapshotAvailable` for runtime snapshots

## Quality Gate Score

See `docs/README.md` for the full quality-gate table.
