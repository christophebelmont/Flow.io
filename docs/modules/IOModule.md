# IOModule

## Summary

**Module ID:** `io`  
**Type:** Active (FreeRTOS task)  
**Declared dependencies:** `loghub`, `datastore`, `mqtt`, `ha`  
**Exposes services:** `io (IO service)`, `io.ledmask (mask service)`, `runtime snapshots (IRuntimeSnapshotProvider)`  
**Consumes services:** `datastore`, `mqtt`, `ha`, `loghub`  

## Purpose

IOModule is the hardware abstraction and signal acquisition layer.
It defines IO endpoints (sensors and actuators), drives hardware drivers (GPIO/I2C/OneWire), and publishes IO runtime snapshots.

Key goals:
- stable `IoId`-based addressing for all consumers
- bounded endpoint counts and deterministic polling
- explicit endpoint metadata for remote discovery (e.g. Home Assistant)

## Architecture

### Buses and Drivers

IOModule owns bus objects and driver instances for:
- I2C (ADS1115, PCF8574)
- OneWire (DS18B20)
- GPIO (digital IO pins)

Drivers are adapted into endpoint objects (sensor/actuator endpoints).

### Registry

`IORegistry` maps `IoId` → endpoint pointer + metadata.

Typical IO service calls:
- `readDigital(IoId, ...)`
- `writeDigital(IoId, ...)`
- `readAnalog(IoId, ...)`
- `meta(IoId, ...)`

## Slot model

IOModule is slot-based and bounded:

```cpp
static constexpr uint8_t MAX_ANALOG_ENDPOINTS = 12;
static constexpr uint8_t MAX_DIGITAL_INPUTS  = 8;
static constexpr uint8_t MAX_DIGITAL_OUTPUTS = 12;
```

Each slot stores a definition + runtime state:
- calibration (`c0`, `c1`), validity range, precision
- median filter for analog values (`RunningMedianAverageFloat`)
- pulse timing for momentary outputs (deadline-driven)

## Scheduler ticks

IOModule uses an internal scheduler to poll different groups at different rates:

- ADS analog sampling (fast)
- DS18B20 temperature sampling (slow)
- digital input polling

Example registration (conceptual, based on the tick callbacks in the module):

```cpp
scheduler_.addTick("ads.fast", cfgData_.adsPollMs, &IOModule::tickFastAds_, this);
scheduler_.addTick("ds.slow",  cfgData_.dsPollMs,  &IOModule::tickSlowDs_, this);
scheduler_.addTick("din",      cfgData_.digitalPollMs, &IOModule::tickDigitalInputs_, this);
```

## Configuration

### ConfigStore variables

IOModule registers a large set of NVS keys (see `NvsKeys::Io::*`), including:
- enable flag, I2C SDA/SCL pins
- analog/digital slot configuration blocks
- ADS/PCF/trace parameters

The keys are centralized in `include/Core/NvsKeys.h`.

### SystemLimits

IOModule uses SystemLimits for trace period defaults:

```cpp
int32_t tracePeriodMs = (int32_t)Limits::IoTracePeriodMs;
```

## DataStore integration

IOModule publishes its runtime data through `IORuntimeData` (generated aggregation) and emits change notifications for sensor/actuator updates.

## Runtime snapshots

IOModule implements `IRuntimeSnapshotProvider` and provides multiple snapshot routes (inputs, outputs, per-slot endpoints).

Example API:

```cpp
uint8_t runtimeSnapshotCount() const override;
const char* runtimeSnapshotSuffix(uint8_t idx) const override;
bool buildRuntimeSnapshot(uint8_t idx, char* out, size_t len, uint32_t& maxTsOut) const override;
```

## Code examples

### Define an analog input

```cpp
IOAnalogDefinition def{};
strncpy(def.id, "water_temp", sizeof(def.id)-1);
def.ioId = IO_ID_AI_BASE + 0;
def.source = IO_SRC_DS18_WATER;
def.c0 = 1.0f; def.c1 = 0.0f;
io->defineAnalogInput(def);
```

### Define a digital output (momentary)

```cpp
IODigitalOutputDefinition out{};
strncpy(out.id, "filling_valve", sizeof(out.id)-1);
out.ioId = IO_ID_DO_BASE + 2;
out.pin = 26;
out.activeHigh = true;
out.momentary = true;
out.pulseMs = 500;
io->defineDigitalOutput(out);
```

## Quality Gate Score

See `docs/README.md` for the full quality-gate table.
