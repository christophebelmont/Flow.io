
# MQTTModule – API Reference

## Services

### Exposed
- MqttService

### Consumed
- WifiService, CommandService, ConfigStoreService, EventBusService, DataStoreService

---

## Config Keys (NvsKeys)

- Mqtt::Host
- Mqtt::Port
- Mqtt::BaseTopic
- Mqtt::Enabled
- Mqtt::SensorMinPublishMs

---

## DataKeys

- MqttReady
- MqttRxDrop
- MqttParseFail
- MqttHandlerFail
- MqttOversizeDrop

---

## EventBus

### Subscribed
- WifiNetReady
- ConfigChanged
- DataChanged
- SnapshotAvailable

### Emitted
- None (publishes externally via MQTT)

---

## SystemLimits

- Limits::Mqtt::*
- Limits::JsonCmdBuf
- Limits::JsonCfgBuf

---

## Code Example

```cpp
// Example usage snippet
```
