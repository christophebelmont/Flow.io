# PoolDeviceModule

## Summary

**Module ID:** `pooldev`  
**Type:** Passive (no task)  
**Declared dependencies:** `io`, `loghub`, `datastore`  
**Exposes services:** `pooldev (PoolDeviceService)`  
**Consumes services:** `io`, `datastore`, `loghub`  

## Configuration

ConfigStore keys:

- `NvsKeys::PoolDevice::{EnabledFmt,TypeFmt,DependsFmt,FlowFmt,TankCapFmt,TankInitFmt}`

## Purpose

PoolDeviceModule models pool equipment as device slots and applies intents to IO.

Example IO write:

```cpp
IoStatus st = io->writeDigital(ioCtx, ioId, on, nowMs);
```

## Quality Gate Score

See `docs/README.md` for the full quality-gate table.
