#!/usr/bin/env python3
"""Generate cfgdocs/cfgmods payloads from module text manifests."""

from __future__ import annotations

import json
import os
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

Import = type("Import", (), {})

try:
    Import("env")  # type: ignore
except Exception:
    env = None


def _get_project_dir() -> Path:
    if env is not None:
        try:
            return Path(env.get("PROJECT_DIR"))
        except Exception:
            pass
    return Path(os.getcwd())


def _merge_meta_dict(base: dict, overlay: dict) -> dict:
    out = dict(base or {})
    if not isinstance(overlay, dict):
        return out
    for key, value in overlay.items():
        if isinstance(value, dict) and isinstance(out.get(key), dict):
            out[key] = _merge_meta_dict(out[key], value)
            continue
        if isinstance(value, list) and isinstance(out.get(key), list):
            merged: List[Any] = []
            seen = set()
            for source in (out.get(key, []), value):
                for item in source:
                    token = json.dumps(item, ensure_ascii=False, sort_keys=True)
                    if token in seen:
                        continue
                    seen.add(token)
                    merged.append(item)
            out[key] = merged
            continue
        out[key] = value
    return out


def _load_text_payload(path: Path) -> Optional[dict]:
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:
        print(f"[generate_config_docs] warning: invalid JSON in {path}: {exc}")
        return None
    if not isinstance(data, dict):
        print(f"[generate_config_docs] warning: ignored non-object payload in {path}")
        return None
    return data


def _text_manifest_files(src_root: Path, stem: str, locale: str = "fr") -> List[Path]:
    modules_root = src_root / "Modules"
    if not modules_root.exists():
        return []
    candidates = sorted(modules_root.rglob(f"text/{stem}*.json"))
    by_dir: Dict[Path, List[Path]] = {}
    for path in candidates:
        by_dir.setdefault(path.parent, []).append(path)

    selected: List[Path] = []
    target_name = f"{stem}.{locale}.json"
    for _, paths in sorted(by_dir.items(), key=lambda item: str(item[0])):
        picks = {path.name: path for path in paths}
        chosen = (
            picks.get(target_name)
            or picks.get(f"{stem}.json")
            or picks.get(f"{stem}.fr.json")
            or (sorted(paths)[0] if paths else None)
        )
        if chosen:
            selected.append(chosen)
    return selected


def _load_text_docs(src_root: Path, stem: str, locale: str = "fr") -> Tuple[Dict[str, dict], dict, List[Path]]:
    docs: Dict[str, dict] = {}
    meta: dict = {}
    loaded_files: List[Path] = []
    for path in _text_manifest_files(src_root, stem=stem, locale=locale):
        payload = _load_text_payload(path)
        if payload is None:
            continue
        loaded_files.append(path)
        payload_docs = payload.get("docs")
        if isinstance(payload_docs, dict):
            for raw_key, raw_val in payload_docs.items():
                if not isinstance(raw_key, str) or not isinstance(raw_val, dict):
                    continue
                docs[raw_key.strip("/")] = dict(raw_val)
        payload_meta = payload.get("meta")
        if not isinstance(payload_meta, dict):
            payload_meta = payload.get("_meta")
        if isinstance(payload_meta, dict):
            meta = _merge_meta_dict(meta, payload_meta)
    return docs, meta, loaded_files


def _load_text_translations(src_root: Path, locale: str = "fr") -> Tuple[Dict[str, str], List[Path]]:
    modules_root = src_root / "Modules"
    if not modules_root.exists():
        return {}, []
    out: Dict[str, str] = {}
    loaded_files: List[Path] = []
    for path in sorted(modules_root.rglob(f"text/i18n.{locale}.json")):
        payload = _load_text_payload(path)
        if payload is None:
            continue
        loaded_files.append(path)
        translations = payload.get("translations")
        source = translations if isinstance(translations, dict) else payload
        for raw_key, raw_val in source.items():
            if not isinstance(raw_key, str) or not isinstance(raw_val, str):
                continue
            key = raw_key.strip()
            if key:
                out[key] = raw_val
    return out, loaded_files


def _resolve_doc_i18n_fields(raw_doc: dict, translations: Dict[str, str]) -> dict:
    doc = dict(raw_doc or {})
    label_token = doc.get("label_t")
    help_token = doc.get("help_t")
    if isinstance(label_token, str) and label_token.strip():
        token = label_token.strip()
        doc["label"] = translations.get(token, token)
        doc["label_i18n"] = token
    if isinstance(help_token, str) and help_token.strip():
        token = help_token.strip()
        doc["help"] = translations.get(token, token)
        doc["help_i18n"] = token
    return doc


def _resolve_meta_i18n(node: Any, translations: Dict[str, str]) -> Any:
    if isinstance(node, list):
        return [_resolve_meta_i18n(item, translations) for item in node]
    if not isinstance(node, dict):
        return node
    out = {k: _resolve_meta_i18n(v, translations) for k, v in node.items()}
    label_token = out.get("label_t")
    help_token = out.get("help_t")
    if isinstance(label_token, str) and label_token.strip():
        token = label_token.strip()
        out["label"] = translations.get(token, token)
        out["label_i18n"] = token
    if isinstance(help_token, str) and help_token.strip():
        token = help_token.strip()
        out["help"] = translations.get(token, token)
        out["help_i18n"] = token
    return out


def _detect_pio_env() -> str:
    if env is not None:
        try:
            value = str(env.subst("$PIOENV") or "").strip()
            if value:
                return value
        except Exception:
            pass
    return str(os.getenv("PIOENV", "") or "").strip()


def _profile_from_pio_env(pio_env: str) -> str:
    name = str(pio_env or "").strip().lower()
    if "waveshare" in name:
        return "waveshare"
    if name.startswith("flowio") or "flowio" in name:
        return "flowio"
    return "generic"


def _profile_override_from_project_options() -> Optional[str]:
    from_env = str(os.getenv("FLOW_CFGDOC_PROFILE", "") or "").strip().lower()
    if from_env in ("flowio", "waveshare", "generic"):
        return from_env

    if env is None:
        return None
    try:
        value = str(env.GetProjectOption("custom_cfgdocs_profile") or "").strip().lower()
    except Exception:
        return None
    if value in ("flowio", "waveshare", "generic"):
        return value
    return None


def _to_int(value: Any) -> Optional[int]:
    try:
        return int(value)
    except Exception:
        return None


def _apply_profile_specific_io_enum_sets(meta: dict, profile: str) -> dict:
    if not isinstance(meta, dict):
        return meta
    enum_sets = meta.get("enum_sets")
    if not isinstance(enum_sets, dict):
        return meta

    def sanitize_enum_entry(entry: dict, label: str) -> dict:
        out = dict(entry or {})
        out["label"] = label
        # Keep this label stable regardless of cfgdoc i18n token overlays.
        out.pop("label_t", None)
        out.pop("label_i18n", None)
        return out

    def non_connected_entry() -> dict:
        return {"value": 0, "label": "Non connecté"}

    def binding_entries_with_non_connected(entries: List[dict]) -> List[dict]:
        filtered = []
        for entry in entries:
            value = _to_int(entry.get("value"))
            if value is None or value == 0 or value == 65535:
                continue
            filtered.append(dict(entry))
        return [non_connected_entry()] + filtered

    # Analog bindings: Micronova's local DS18B20 GPIO binding is not valid on
    # flow.io boards, which expose DS18B20 probes through profile ports 120/121.
    analog_key = "flowio_binding_port_analog"
    analog_entries = enum_sets.get(analog_key)
    if profile in ("flowio", "waveshare") and isinstance(analog_entries, list):
        analog_filtered = [
            dict(entry)
            for entry in analog_entries
            if isinstance(entry, dict) and _to_int(entry.get("value")) != 2
        ]
        if profile == "waveshare":
            analog_labels_waveshare = {
                100: "ADSInt0 - ADS1115 interne canal 0 [100]",
                101: "ADSInt1 - ADS1115 interne canal 1 [101]",
                102: "ADSInt2 - ADS1115 interne canal 2 [102]",
                103: "ADSInt3 - ADS1115 interne canal 3 [103]",
                110: "ADSExt0 - ADS1115 externe paire diff 0 [110]",
                111: "ADSExt1 - ADS1115 externe paire diff 1 [111]",
                120: "OneWire1 - DS18B20 bus 1 [120]",
                121: "OneWire2 - DS18B20 bus 2 [121]",
                130: "SHT40Temp - SHT40 canal 0 [130]",
                131: "SHT40Humidity - SHT40 canal 1 [131]",
                132: "BMP280Temp - BMP280 canal 0 [132]",
                133: "BMP280Pressure - BMP280 canal 1 [133]",
                134: "BME680Temp - BME680 canal 0 [134]",
                135: "BME680Humidity - BME680 canal 1 [135]",
                136: "BME680Pressure - BME680 canal 2 [136]",
                137: "BME680Gas - BME680 canal 3 [137]",
                138: "INA226ShuntMv - INA226 canal 0 [138]",
                139: "INA226BusV - INA226 canal 1 [139]",
                140: "INA226CurrentMa - INA226 canal 2 [140]",
                141: "INA226PowerMw - INA226 canal 3 [141]",
                142: "INA226LoadV - INA226 canal 4 [142]",
            }
            analog_filtered = [
                sanitize_enum_entry(entry, analog_labels_waveshare[value])
                for entry in analog_filtered
                for value in [_to_int(entry.get("value"))]
                if value in analog_labels_waveshare
            ]
        enum_sets[analog_key] = binding_entries_with_non_connected(analog_filtered)

    # Digital input bindings: pin labels differ across flow.io and Waveshare.
    din_key = "flowio_binding_port_digital_input"
    din_entries = enum_sets.get(din_key)
    if isinstance(din_entries, list):
        current = [item for item in din_entries if isinstance(item, dict)]
        din_labels_flowio = {
            200: "DIN0 - GPIO34 [200]",
            201: "DIN1 - GPIO36 [201]",
            202: "DIN2 - GPIO39 [202]",
            203: "DIN3 - GPIO35 [203]",
        }
        din_labels_waveshare = {
            200: "DIN0 - GPIO4 [200]",
            201: "DIN1 - GPIO5 [201]",
            202: "DIN2 - GPIO6 [202]",
            203: "DIN3 - GPIO7 [203]",
            204: "DIN4 - GPIO8 [204]",
            205: "DIN5 - GPIO9 [205]",
            206: "DIN6 - GPIO10 [206]",
            207: "DIN7 - GPIO11 [207]",
            400: "GPB0 - MCP23017 input [400]",
            401: "GPB1 - MCP23017 input [401]",
            402: "GPB2 - MCP23017 input [402]",
            403: "GPB3 - MCP23017 input [403]",
            404: "GPB4 - MCP23017 input [404]",
            405: "GPB5 - MCP23017 input [405]",
            406: "GPB6 - MCP23017 input [406]",
            407: "GPB7 - MCP23017 input [407]",
        }

        selected_labels = None
        if profile == "flowio":
            selected_labels = din_labels_flowio
        elif profile == "waveshare":
            selected_labels = din_labels_waveshare

        if selected_labels is not None:
            filtered: List[dict] = []
            present_values = set()
            for entry in current:
                value = _to_int(entry.get("value"))
                if value is None or value not in selected_labels:
                    continue
                filtered.append(sanitize_enum_entry(entry, selected_labels[value]))
                present_values.add(value)
            for value, label in selected_labels.items():
                if value not in present_values:
                    filtered.append({"value": value, "label": label})
            enum_sets[din_key] = binding_entries_with_non_connected(filtered)

    # Digital output bindings: flow.io uses its onboard PCF8574; Waveshare exposes
    # its primary TCA9554/MCP23017 plus optional PCF8574 and TCA9554 instances.
    dout_key = "flowio_binding_port_digital_output"
    dout_entries = enum_sets.get(dout_key)
    if isinstance(dout_entries, list):
        current = [item for item in dout_entries if isinstance(item, dict)]
        filtered: List[dict] = []
        for entry in current:
            value = _to_int(entry.get("value"))
            if value is None:
                continue
            keep = True
            # Micronova aux_output must not be exposed on flow.io / flow.io profiles.
            if profile in ("flowio", "waveshare") and value == 1:
                keep = False
            if profile == "flowio":
                keep = keep and not (300 <= value <= 399)
            if keep:
                filtered.append(dict(entry))

        if profile == "waveshare":
            dout_labels_waveshare = {
                300: "EXIO1 - TCA9554 bit 0 [300]",
                301: "EXIO2 - TCA9554 bit 1 [301]",
                302: "EXIO3 - TCA9554 bit 2 [302]",
                303: "EXIO4 - TCA9554 bit 3 [303]",
                304: "EXIO5 - TCA9554 bit 4 [304]",
                305: "EXIO6 - TCA9554 bit 5 [305]",
                306: "EXIO7 - TCA9554 bit 6 [306]",
                307: "EXIO8 - TCA9554 bit 7 [307]",
                408: "GPA0 - MCP23017 output [408]",
                409: "GPA1 - MCP23017 output [409]",
                410: "GPA2 - MCP23017 output [410]",
                411: "GPA3 - MCP23017 output [411]",
                412: "GPA4 - MCP23017 output [412]",
                413: "GPA5 - MCP23017 output [413]",
                414: "GPA6 - MCP23017 output [414]",
                415: "GPA7 - MCP23017 output [415]",
                500: "PCF2/P0 - PCF8574 auxiliary bit 0 [500]",
                501: "PCF2/P1 - PCF8574 auxiliary bit 1 [501]",
                502: "PCF2/P2 - PCF8574 auxiliary bit 2 [502]",
                503: "PCF2/P3 - PCF8574 auxiliary bit 3 [503]",
                504: "PCF2/P4 - PCF8574 auxiliary bit 4 [504]",
                505: "PCF2/P5 - PCF8574 auxiliary bit 5 [505]",
                506: "PCF2/P6 - PCF8574 auxiliary bit 6 [506]",
                507: "PCF2/P7 - PCF8574 auxiliary bit 7 [507]",
                510: "TCA3/P0 - TCA9554 auxiliary bit 0 [510]",
                511: "TCA3/P1 - TCA9554 auxiliary bit 1 [511]",
                512: "TCA3/P2 - TCA9554 auxiliary bit 2 [512]",
                513: "TCA3/P3 - TCA9554 auxiliary bit 3 [513]",
                514: "TCA3/P4 - TCA9554 auxiliary bit 4 [514]",
                515: "TCA3/P5 - TCA9554 auxiliary bit 5 [515]",
                516: "TCA3/P6 - TCA9554 auxiliary bit 6 [516]",
                517: "TCA3/P7 - TCA9554 auxiliary bit 7 [517]",
            }
            relabeled: List[dict] = []
            present_values = set()
            for entry in filtered:
                value = _to_int(entry.get("value"))
                if value is not None and value in dout_labels_waveshare:
                    relabeled.append(sanitize_enum_entry(entry, dout_labels_waveshare[value]))
                    present_values.add(value)
                else:
                    if value is not None and value in dout_labels_waveshare:
                        relabeled.append(dict(entry))
            for value, label in dout_labels_waveshare.items():
                if value not in present_values:
                    relabeled.append({"value": value, "label": label})
            filtered = relabeled

        # Ensure flow.io exposes all 8 PCF bits (400..407) in UI bindings.
        if profile == "flowio":
            present_values = {_to_int(item.get("value")) for item in filtered}
            if 407 not in present_values:
                filtered.append(
                    {
                        "value": 407,
                        "label": "PortPCF0Bit7 - Sortie PCF8574 - Bit 7 [407]",
                    }
                )
        enum_sets[dout_key] = binding_entries_with_non_connected(filtered)

    # PoolLogic device slots: keep generic labels by default, but expose
    # profile wiring-specific mapping in UI for faster setup.
    slot_key = "poollogic_device_slot"
    slot_entries = enum_sets.get(slot_key)
    if profile == "waveshare" and isinstance(slot_entries, list):
        current = [item for item in slot_entries if isinstance(item, dict)]
        slot_labels_waveshare = {slot: f"pd{slot} -> d{slot:02d} [{slot}]" for slot in range(16)}
        current_by_value: Dict[int, dict] = {}
        for entry in current:
            value = _to_int(entry.get("value"))
            if value is not None:
                current_by_value[value] = entry
        relabeled: List[dict] = []
        for value in range(16):
            entry = current_by_value.get(value, {"value": value})
            relabeled.append(sanitize_enum_entry(entry, slot_labels_waveshare[value]))
        enum_sets[slot_key] = relabeled

    return meta


def _resolved_docs(docs: Dict[str, dict], translations: Dict[str, str]) -> Dict[str, dict]:
    return {
        key: _resolve_doc_i18n_fields(value, translations)
        for key, value in docs.items()
        if isinstance(value, dict)
    }


def _write_json(path: Path, payload: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def main() -> None:
    project_dir = _get_project_dir()
    src_root = project_dir / "src"
    locale = os.getenv("FLOW_CFGDOC_LOCALE", "fr").strip().lower() or "fr"

    out_path = project_dir / "data" / "webinterface" / "cfgdocs.json"
    cfgmods_out_path = project_dir / "data" / "webinterface" / "cfgmods.json"

    cfgdocs_docs, cfgdocs_meta, cfgdocs_files = _load_text_docs(src_root, stem="cfgdocs", locale=locale)
    cfgmods_docs, cfgmods_meta, cfgmods_files = _load_text_docs(src_root, stem="cfgmods", locale=locale)
    i18n, i18n_files = _load_text_translations(src_root, locale=locale)

    pio_env = _detect_pio_env()
    profile = _profile_override_from_project_options() or _profile_from_pio_env(pio_env)

    combined_meta = _resolve_meta_i18n(_merge_meta_dict(cfgdocs_meta, cfgmods_meta), i18n)
    combined_meta = _apply_profile_specific_io_enum_sets(combined_meta, profile)

    merged_docs = _resolved_docs(dict(cfgdocs_docs), i18n)

    cfgdocs_payload = {
        "_meta": {
            "generated": True,
            "locale": locale,
            "source": "text",
            "total": len(merged_docs),
        },
        "meta": combined_meta if isinstance(combined_meta, dict) else {},
        "docs": dict(sorted(merged_docs.items(), key=lambda kv: kv[0])),
    }
    _write_json(out_path, cfgdocs_payload)

    cfgmods_payload = {
        "_meta": {
            "generated": True,
            "locale": locale,
            "version": 1,
            "source": "text",
        },
        "meta": combined_meta if isinstance(combined_meta, dict) else {},
        "docs": dict(sorted(_resolved_docs(cfgmods_docs, i18n).items(), key=lambda kv: kv[0])),
    }
    _write_json(cfgmods_out_path, cfgmods_payload)

    print(
        f"[generate_config_docs] wrote {out_path} "
        f"(docs={len(cfgdocs_payload['docs'])} cfgmods={len(cfgmods_payload['docs'])} "
        f"text_files={len(cfgdocs_files) + len(cfgmods_files)} i18n_files={len(i18n_files)} "
        f"pio_env={pio_env or '-'} profile={profile})"
    )


if __name__ == "__main__":
    main()
