# MQTT Protocol

This specification describes MQTT topics and payload conventions used by Flow.IO.

## Topic layout

Flow.IO uses a device-scoped topic prefix:

```
<base>/<device>/...
```

- `<base>` is `mqtt.baseTopic` (ConfigStore key `NvsKeys::Mqtt::BaseTopic`)
- `<device>` is the firmware device id (e.g. `ESP32-<MAC>`)

Standard suffixes are defined in `include/Core/MqttTopics.h`:

- `cmd` → command ingress
- `ack` → command/config replies
- `status` → availability / last will
- `cfg/set` → config patch ingress
- `cfg/ack` → config apply acknowledgments

## Commands

### Topic

```
<base>/<device>/cmd
```

### Payload

```json
{
  "cmd": "module.command",
  "args": { "k": "v" },
  "token": "optional-correlation-token"
}
```

### ACK topic

```
<base>/<device>/ack
```

### ACK payload

Success example:

```json
{ "ok": true, "cmd": "time.resync", "token": "abc123" }
```

Error example:

```json
{
  "ok": false,
  "cmd": "time.resync",
  "token": "abc123",
  "err": { "code": "BadCmdJson", "where": "cmd", "retryable": false }
}
```

## Configuration patches

### Topic

```
<base>/<device>/cfg/set
```

### Payload

Example:

```json
{
  "mqtt": { "host": "broker.local", "enabled": true },
  "time": { "tz": "CET-1CEST,M3.5.0/2,M10.5.0/3" }
}
```

### ACK topic

```
<base>/<device>/cfg/ack
```

## Config blocks publication

Retained blocks per module:

```
<base>/<device>/cfg/<moduleName>
```

## Runtime state publication

- Periodic publishers registered via `MQTTModule::addRuntimePublisher`
- Sensor bundles publisher via `MQTTModule::setSensorsPublisher`
- Snapshot payloads (e.g. IO snapshots)

Sizes and timing are bounded by `Limits::Mqtt::*`.
