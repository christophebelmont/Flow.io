
# IOModule – API Reference

## Services

### Exposed
- IOService

### Consumed
- DataStoreService, LogHubService

---

## Config Keys (NvsKeys)

- Io::* (slot definitions, SDA/SCL, enable flags)

---

## DataKeys

- All sensor/actuator runtime keys in IORuntimeData

---

## EventBus

### Subscribed
- ConfigChanged

### Emitted
- DataChanged (sensor updates)

---

## SystemLimits

- MAX_ANALOG_ENDPOINTS
- MAX_DIGITAL_INPUTS
- MAX_DIGITAL_OUTPUTS

---

## Code Example

```cpp
// Example usage snippet
```
