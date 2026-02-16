# IO Model

This document describes the IO abstraction used by IOModule and its consumers.

## Concepts

- **IoId**: numeric identifier for a logical IO endpoint
- **Endpoint**: runtime object representing a sensor or actuator with metadata and state
- **Driver**: hardware implementation that reads/writes pins or buses
- **Registry**: mapping from IoId to endpoints and metadata
- **Scheduler**: tick-based polling of drivers/endpoints

## Slot-based configuration

IOModule supports bounded counts:
- Analog endpoints: `MAX_ANALOG_ENDPOINTS`
- Digital inputs: `MAX_DIGITAL_INPUTS`
- Digital outputs: `MAX_DIGITAL_OUTPUTS`

## Scheduler ticks

Typical ticks:
- fast analog sampling (ADS1115)
- slow OneWire temperature sampling (DS18B20)
- digital input polling
