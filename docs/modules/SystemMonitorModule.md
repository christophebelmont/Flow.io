# SystemMonitorModule

## Summary

**Module ID:** `sysmon`  
**Type:** Active (FreeRTOS task)  
**Declared dependencies:** `loghub`, `datastore`, `eventbus`  
**Exposes services:** `sysmon (monitoring service)`  
**Consumes services:** `loghub`, `datastore`, `eventbus`  

## Configuration

ConfigStore keys:

- `NvsKeys::SystemMonitor::TraceEnabled`
- `NvsKeys::SystemMonitor::TracePeriodMs`

## Purpose

SystemMonitorModule provides periodic health monitoring and trace reporting.

## Quality Gate Score

See `docs/README.md` for the full quality-gate table.
