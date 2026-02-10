# WifiModule

## Description generale

`WifiModule` gere la connexion STA Wi-Fi (etat, reconnexion, timeout), expose un service de statut reseau et publie l'etat reseau dans `DataStore`.

## Dependances module

- `moduleId`: `wifi`
- Dependances declarees: `loghub`, `datastore`

## Services proposes

- `wifi` -> `WifiService`
  - `state`
  - `isConnected`
  - `getIP`

## Services consommes

- `datastore` (`DataStoreService`) pour publier l'etat runtime
- `loghub` (`LogHubService`) pour log interne

## Valeurs ConfigStore utilisees

- Module JSON: `wifi`
- Cles:
  - `enabled`
  - `ssid`
  - `pass`

## Valeurs DataStore utilisees

- Ecrit:
  - `wifi.ready` (DataKey `DATAKEY_WIFI_READY` = 1, dirty `DIRTY_NETWORK`)
  - `wifi.ip` (DataKey `DATAKEY_WIFI_IP` = 2, dirty `DIRTY_NETWORK`)
- Lu:
  - aucune lecture metier du runtime wifi
