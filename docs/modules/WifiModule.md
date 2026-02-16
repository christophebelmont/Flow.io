# WifiModule

## Summary

**Module ID:** `wifi`  
**Type:** Active (FreeRTOS task)  
**Declared dependencies:** `loghub`, `eventbus`, `datastore`  
**Exposes services:** `wifi (WifiService)`  
**Consumes services:** `eventbus`, `datastore`, `loghub`  

## Configuration

ConfigStore keys:

- `NvsKeys::Wifi::Enabled`
- `NvsKeys::Wifi::Ssid`
- `NvsKeys::Wifi::Pass`

## Purpose

WifiModule manages Wi‑Fi connectivity and emits a `WifiNetReady` event when the network stack is ready.

## EventBus

Example network-ready event:

```cpp
WifiNetReadyPayload p{...};
eventBus->publish(EventType::WifiNetReady, &p, sizeof(p));
```

## Quality Gate Score

See `docs/README.md` for the full quality-gate table.
