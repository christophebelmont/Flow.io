# EventBusModule

## Summary

**Module ID:** `eventbus`  
**Type:** Active (FreeRTOS task)  
**Declared dependencies:** `loghub`  
**Exposes services:** `eventbus (EventBusService)`  
**Consumes services:** `loghub`  

## Purpose

EventBusModule hosts and dispatches internal events using a bounded queue.

Example publish:

```cpp
eventBus->publish(EventType::ConfigChanged, &p, sizeof(p));
```

## Quality Gate Score

See `docs/README.md` for the full quality-gate table.
