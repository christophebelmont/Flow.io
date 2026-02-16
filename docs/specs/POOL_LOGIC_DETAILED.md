
# PoolLogicModule – Detailed Business Logic

## Overview

PoolLogicModule computes automation decisions for:

- Filtration
- Electrolysis (SWG)
- Robot
- Water filling
- Freeze protection
- PSI safety interlock
- ORP regulation

It consumes:

- Sensor Data (IOModule → DataStore)
- Scheduler edges (TimeModule → EventBus)
- Configuration (ConfigStore)
- Device control service (PoolDeviceModule)

---

## Filtration Logic

Filtration window is computed deterministically from:

- Water temperature
- Low temperature threshold
- Setpoint temperature
- Configured start minute
- Maximum stop minute

```cpp
FiltrationWindow computeFiltrationWindowDeterministic(
  float tempC,
  float lowC,
  float setpointC,
  uint16_t startMin,
  uint16_t stopMax);
```

### Behavior

- temp <= low → minimal filtration duration
- temp >= setpoint → maximal duration
- interpolation in-between
- clamped to [startMin, stopMax]

---

## Interlocks

### PSI Interlock

If pressure > high threshold:
- Immediate stop of filtration
- Electrolysis disabled
- Error state published

### Freeze Protection

If air temperature < winterStart:
- Filtration forced ON
- FreezeHold duration applied

### Electrolysis Safety

ElectrolyseMode requires:
- Filtration running
- ORP below setpoint
- SecureElectro flag
- Delay timers respected

### Filling Safety

Filling allowed only if:
- Level sensor below threshold
- Minimum ON duration respected
- Max tank capacity not exceeded

---

## Scheduler Integration

TimeModule emits `SchedulerEventTriggered`.

On Start edge for configured slot:
- FiltrationWindowActive = true

On Stop edge:
- FiltrationWindowActive = false

---

## DataKeys Published

- PoolFiltrationActive
- PoolElectroActive
- PoolRobotActive
- PoolFillingActive
- PoolMode
- PoolWinterMode
- PoolPsiError
- PoolFreezeHold

---

## Deterministic Control Loop

Executed in `loop()` with bounded delay:

```cpp
void PoolLogicModule::loop() {
  evaluateSensors();
  evaluateScheduler();
  computeFiltration();
  applyInterlocks();
  publishState();
}
```

No heap allocation occurs during loop execution.
