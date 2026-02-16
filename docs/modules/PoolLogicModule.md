
# PoolLogicModule – API Reference

## Services

### Exposed
- None

### Consumed
- TimeSchedulerService, IOService, PoolDeviceService, DataStoreService

---

## Config Keys (NvsKeys)

- PoolLogic::* (thresholds, delays, slot ids)

---

## DataKeys

- PoolFiltrationActive
- PoolElectroActive
- PoolPsiError
- PoolMode

---

## EventBus

### Subscribed
- SchedulerEventTriggered
- DataChanged
- ConfigChanged

### Emitted
- None (writes intents via services)

---

## SystemLimits

- Domain thresholds
- Delay timers

---

## Code Example

```cpp
// Example usage snippet
```
