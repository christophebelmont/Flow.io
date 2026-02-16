# HAModule

## Summary

**Module ID:** `ha`  
**Type:** Passive (no task)  
**Declared dependencies:** `mqtt`, `io`, `pooldev`  
**Exposes services:** `ha (HA integration)`  
**Consumes services:** `mqtt`, `io`, `pooldev`  

## Configuration

ConfigStore keys:

- `NvsKeys::Ha::Enabled`
- `NvsKeys::Ha::DiscoveryPrefix`
- `NvsKeys::Ha::DeviceId`
- `NvsKeys::Ha::Vendor`
- `NvsKeys::Ha::Model`

## Purpose

HAModule publishes Home Assistant discovery payloads and maps Flow.IO entities to HA components.

Example publish:

```cpp
char topic[128];
mqtt->formatTopic(topic, sizeof(topic), "ha/config");
mqtt->publish(topic, payload, 0, true);
```

## Quality Gate Score

See `docs/README.md` for the full quality-gate table.
