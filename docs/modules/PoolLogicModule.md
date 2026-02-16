# PoolLogicModule

## Summary

**Module ID:** `poollogic`  
**Type:** Active (FreeRTOS task)  
**Declared dependencies:** `loghub`, `datastore`, `eventbus`, `time.scheduler`, `pooldev`, `io`  
**Consumes services:** `datastore`, `eventbus`, `time.scheduler`, `pooldev`, `io`, `loghub`  

## Configuration

ConfigStore keys:

- `NvsKeys::PoolLogic::*`

## Purpose

PoolLogicModule implements the automation policies for filtration, electrolysis, robot, and filling.

Example: react to scheduler start edge:

```cpp
if (payload.edge == (uint8_t)SchedulerEdge::Start && payload.slot == cfgData_.filtrationSlot) {
  filtrationWindowActive_ = true;
}
```

## Quality Gate Score

See `docs/README.md` for the full quality-gate table.
