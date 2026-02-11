# SystemMonitorModule

## General description

`SystemMonitorModule` provides runtime health observability (heap, task stack watermark, Wi-Fi state, uptime) through periodic logs.

## Module dependencies

- `moduleId`: `sysmon`
- Declared dependencies: `loghub`

## Provided services

- None.

## Consumed services

- `wifi` (`WifiService`) for state/connection/IP
- `config` (`ConfigStoreService`) for potential config-level diagnostics
- `loghub` (`LogHubService`) for log emission

## ConfigStore values used

- `trace_enabled` (`NVS: sm_tren`, bool, default `true`)
  Active/desactive les logs periodiques de supervision.
- `trace_period_ms` (`NVS: sm_trms`, int32, default `5000`)
  Periode de base en millisecondes pour les logs periodiques.
  Le dump des stacks tourne toutes les `trace_period_ms * 6`, le JSON health toutes les `trace_period_ms * 12`.

## DataStore values used

- No direct `DataStore` read/write.
