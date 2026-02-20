/**
 * @file WebInterfaceModule.cpp
 * @brief Web interface bridge for Supervisor profile.
 */

#include "WebInterfaceModule.h"

#define LOG_TAG "WebServr"
#include "Core/ModuleLog.h"

#include <string.h>
#include <stdlib.h>
#include <ArduinoJson.h>
#include "Core/DataKeys.h"
#include "Core/EventBus/EventPayloads.h"
#include "Modules/Network/WifiModule/WifiRuntime.h"
#include "WebInterfaceMenuIcons.h"

static void sanitizeJsonString_(char* s)
{
    if (!s) return;
    for (size_t i = 0; s[i] != '\0'; ++i) {
        if (s[i] == '"' || s[i] == '\\' || s[i] == '\n' || s[i] == '\r' || s[i] == '\t') {
            s[i] = ' ';
        }
    }
}

static bool parseBoolParam_(const String& in, bool fallback)
{
    if (in.length() == 0) return fallback;
    if (in.equalsIgnoreCase("1") || in.equalsIgnoreCase("true") || in.equalsIgnoreCase("yes") ||
        in.equalsIgnoreCase("on")) {
        return true;
    }
    if (in.equalsIgnoreCase("0") || in.equalsIgnoreCase("false") || in.equalsIgnoreCase("no") ||
        in.equalsIgnoreCase("off")) {
        return false;
    }
    return fallback;
}

static const char kFlowIoLogoSvg[] PROGMEM = R"SVG(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1200 360" role="img" aria-labelledby="title desc">
  <title id="title">Flow.IO Logo</title>
  <desc id="desc">Logo Flow.IO with water drop and waves.</desc>
  <defs>
    <linearGradient id="gradDrop" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#21C7FF"/>
      <stop offset="100%" stop-color="#0066FF"/>
    </linearGradient>
    <linearGradient id="gradWave" x1="0%" y1="0%" x2="100%" y2="0%">
      <stop offset="0%" stop-color="#00AEEF"/>
      <stop offset="100%" stop-color="#1ED6A6"/>
    </linearGradient>
    <filter id="softShadow" x="-20%" y="-20%" width="140%" height="140%">
      <feDropShadow dx="0" dy="4" stdDeviation="6" flood-color="#001A33" flood-opacity="0.2"/>
    </filter>
  </defs>

  <rect x="0" y="0" width="1200" height="360" fill="#FFFFFF"/>

  <g transform="translate(70,40)" filter="url(#softShadow)">
    <circle cx="140" cy="140" r="116" fill="#FFFFFF"/>
    <circle cx="140" cy="140" r="112" fill="none" stroke="#D7E9F8" stroke-width="2"/>

    <path d="M140 40
             C140 40, 72 116, 72 170
             C72 222, 106 252, 140 252
             C174 252, 208 222, 208 170
             C208 116, 140 40, 140 40 Z"
          fill="url(#gradDrop)"/>

    <path d="M82 168
             C98 154, 116 152, 134 160
             C152 168, 170 170, 196 158"
          fill="none" stroke="url(#gradWave)" stroke-width="10" stroke-linecap="round"/>

    <path d="M86 196
             C106 184, 126 184, 146 192
             C166 200, 182 200, 196 194"
          fill="none" stroke="url(#gradWave)" stroke-width="8" stroke-linecap="round" opacity="0.95"/>

  </g>

  <g transform="translate(340,94)">
    <text x="0" y="110" fill="#073B66" font-family="Avenir Next, Montserrat, Segoe UI, Arial, sans-serif" font-size="132" font-weight="800" letter-spacing="1">
      FLOW
    </text>
    <text x="420" y="110" fill="#00AEEF" font-family="Avenir Next, Montserrat, Segoe UI, Arial, sans-serif" font-size="132" font-weight="800">
      .
    </text>
    <text x="470" y="110" fill="#073B66" font-family="Avenir Next, Montserrat, Segoe UI, Arial, sans-serif" font-size="132" font-weight="800">
      IO
    </text>
    <text x="4" y="156" fill="#3C6E91" font-family="Avenir Next, Montserrat, Segoe UI, Arial, sans-serif" font-size="36" font-weight="500" letter-spacing="4">
      SMART POOL WATER MANAGEMENT
    </text>
  </g>
</svg>
)SVG";

static const char kFlowIoIconSvg[] PROGMEM = R"SVG(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512" role="img" aria-labelledby="title desc">
  <title id="title">Flow.IO App Icon</title>
  <desc id="desc">Flow.IO icon with drop and waves.</desc>
  <defs>
    <linearGradient id="dropGrad" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#21C7FF"/>
      <stop offset="100%" stop-color="#0066FF"/>
    </linearGradient>
    <linearGradient id="waveGrad" x1="0%" y1="0%" x2="100%" y2="0%">
      <stop offset="0%" stop-color="#00AEEF"/>
      <stop offset="100%" stop-color="#1ED6A6"/>
    </linearGradient>
  </defs>
  <rect x="24" y="24" width="464" height="464" rx="110" fill="#F6FBFF"/>
  <rect x="24" y="24" width="464" height="464" rx="110" fill="none" stroke="#D6E9F8" stroke-width="4"/>
  <path d="M256 102
           C256 102, 142 224, 142 306
           C142 382, 196 424, 256 424
           C316 424, 370 382, 370 306
           C370 224, 256 102, 256 102 Z"
        fill="url(#dropGrad)"/>
  <path d="M164 298
           C188 280, 216 276, 246 288
           C276 300, 306 304, 350 286"
        fill="none" stroke="url(#waveGrad)" stroke-width="20" stroke-linecap="round"/>
  <path d="M170 346
           C198 330, 228 330, 260 342
           C292 354, 320 356, 348 346"
        fill="none" stroke="url(#waveGrad)" stroke-width="16" stroke-linecap="round" opacity="0.95"/>
</svg>
)SVG";

static const char kWebInterfacePage[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Superviseur Flow.IO</title>
  <style>
    :root {
      --md-bg: #fffbfe;
      --md-surface: #fffbfe;
      --md-on-surface: #1d1b20;
      --md-outline: #79747e;
      --md-primary: #6750a4;
      --md-on-primary: #ffffff;
      --md-secondary-container: #e8def8;
      --md-on-secondary-container: #1d192b;
      --md-surface-variant: #e7e0ec;
      --term-bg: #020617;
      --term-fg: #e2e8f0;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      min-height: 100vh;
      font-family: "Inter", "Segoe UI", Roboto, Arial, sans-serif;
      background: var(--md-bg);
      color: var(--md-on-surface);
    }
    .app { display: flex; min-height: 100vh; }
    .drawer {
      width: 280px;
      padding: 12px;
      border-right: 1px solid rgba(121,116,126,0.35);
      background: #ffffff;
      overflow: hidden;
      transition: width 0.2s ease;
      z-index: 20;
      display: flex;
      flex-direction: column;
    }
    .drawer.collapsed { width: 78px; }
    .drawer-header { display: flex; align-items: center; gap: 10px; margin-bottom: 8px; }
    .menu-btn {
      width: 42px; height: 42px; border: 0; border-radius: 12px;
      background: var(--md-secondary-container); color: var(--md-on-secondary-container);
      cursor: pointer; font-size: 18px; line-height: 1;
    }
    .drawer-user {
      font-size: 18px;
      font-weight: 700;
      color: #073b66;
      line-height: 1;
      white-space: nowrap;
    }
    .drawer.collapsed .drawer-user { display: none; }
    .menu-group { display: grid; gap: 6px; margin-top: 10px; }
    .drawer-footer {
      margin-top: auto;
      display: flex;
      justify-content: center;
      padding-top: 14px;
    }
    .app-icon-shell {
      width: 100%;
      max-width: 210px;
      border-radius: 10px;
      background: #ffffff;
      border: 1px solid #d6e9f8;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 6px 8px;
    }
    .app-icon {
      width: 100%;
      max-width: 170px;
      height: auto;
      display: block;
    }
    .menu-item {
      border: 0; border-radius: 14px; padding: 12px 14px;
      display: flex; align-items: center; gap: 12px; cursor: pointer;
      background: transparent; color: var(--md-on-surface); text-align: left;
      font-size: 14px; font-weight: 500; white-space: nowrap;
    }
    .menu-item.active { background: var(--md-secondary-container); color: var(--md-on-secondary-container); font-weight: 600; }
    .menu-item .ico {
      width: 22px;
      text-align: center;
      display: inline-flex;
      justify-content: center;
      align-items: center;
    }
    .menu-item .ico-svg {
      font-size: 22px;
      width: 22px;
      height: 22px;
      display: block;
    }
    .drawer.collapsed .menu-item .label { display: none; }
    @media (min-width: 901px) {
      .drawer.collapsed .drawer-header { justify-content: center; }
      .drawer.collapsed .menu-group { justify-items: center; }
      .drawer.collapsed .menu-btn,
      .drawer.collapsed .menu-item {
        width: 48px;
        min-width: 48px;
        height: 42px;
        padding-left: 0;
        padding-right: 0;
        justify-content: center;
      }
      .drawer.collapsed .menu-item .ico { width: auto; }
      .drawer.collapsed .app-icon-shell {
        width: 48px;
        max-width: 48px;
        min-height: 48px;
        padding: 8px 6px;
        border-radius: 14px;
      }
      .drawer.collapsed .app-icon {
        max-width: 100%;
      }
    }
    .content { flex: 1; padding: 18px; min-width: 0; }
    .mobile-topbar { display: none; }
    .mobile-topbar .mobile-title {
      font-size: 16px;
      font-weight: 700;
      color: #073b66;
      line-height: 1;
    }
    .page { display: none; }
    .page.active { display: block; }
    .topbar { display: flex; align-items: center; justify-content: space-between; margin-bottom: 12px; gap: 12px; }
    .topbar h1 { margin: 0; font-size: 20px; font-weight: 700; }
    .status-chip {
      font-size: 12px; padding: 7px 10px; border-radius: 999px;
      background: var(--md-surface-variant);
    }
    .card {
      background: rgba(255,251,254,0.95);
      border: 1px solid rgba(121,116,126,0.35);
      border-radius: 20px;
      padding: 14px;
    }
    .terminal {
      border: 1px solid #334155;
      background: var(--term-bg);
      color: var(--term-fg);
      border-radius: 10px;
      height: 66vh;
      overflow: auto;
      padding: 10px;
      white-space: pre-wrap;
      font-family: "Cascadia Mono", "JetBrains Mono", "Fira Code", "SFMono-Regular", Menlo, Monaco, Consolas, monospace;
      font-size: 11px;
      line-height: 1.25;
    }
    .log-line { white-space: pre-wrap; }
    .term-toolbar { display: flex; gap: 8px; margin-top: 10px; }
    .term-toolbar input { flex: 1; }
    .btn-toggle-off {
      background: #eef2f7;
      color: #5b6673;
      border-color: rgba(121,116,126,0.35);
    }
    .form-grid { display: grid; gap: 12px; grid-template-columns: repeat(2, minmax(220px, 1fr)); }
    .field { display: grid; gap: 6px; min-width: 0; }
    .field.full { grid-column: 1 / -1; }
    .field label { font-size: 12px; font-weight: 600; opacity: 0.78; }
    .password-wrap { position: relative; }
    .password-wrap input { width: 100%; padding-right: 46px; }
    .password-toggle {
      position: absolute;
      right: 7px;
      top: 50%;
      transform: translateY(-50%);
      width: 30px;
      height: 30px;
      border: 0;
      border-radius: 8px;
      padding: 0;
      background: transparent;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      cursor: pointer;
      opacity: 0.72;
    }
    .password-toggle:hover { background: rgba(121,116,126,0.12); opacity: 1; }
    .password-toggle:focus-visible { outline: 2px solid rgba(103,80,164,0.4); }
    .password-toggle svg { width: 18px; height: 18px; display: block; }
    input, select, button {
      border-radius: 12px; border: 1px solid rgba(121,116,126,0.45);
      background: white; color: var(--md-on-surface); font: inherit; padding: 10px 12px;
    }
    input:focus, select:focus { outline: 2px solid rgba(103,80,164,0.4); border-color: var(--md-primary); }
    .btn-row { display: flex; flex-wrap: wrap; gap: 8px; margin-top: 12px; }
    button { cursor: pointer; }
    .btn-primary { background: var(--md-primary); color: var(--md-on-primary); border-color: var(--md-primary); font-weight: 600; }
    .btn-tonal { background: var(--md-secondary-container); color: var(--md-on-secondary-container); border-color: transparent; font-weight: 600; }
    .upgrade-status {
      margin-top: 10px; padding: 10px; border-radius: 12px;
      background: rgba(232,222,248,0.7); font-size: 13px;
    }
    .upgrade-progress {
      margin-top: 12px;
      height: 8px;
      border-radius: 999px;
      overflow: hidden;
      background: var(--md-surface-variant);
      border: 1px solid rgba(121,116,126,0.22);
    }
    .upgrade-progress-bar {
      height: 100%;
      width: 0%;
      border-radius: inherit;
      background: linear-gradient(90deg, #6750a4 0%, #7f67be 100%);
      transition: width 0.2s ease;
    }
    .config-status {
      margin-top: 10px;
      padding: 10px;
      border-radius: 12px;
      background: rgba(214, 233, 248, 0.8);
      font-size: 13px;
      color: #073b66;
    }
    .config-section {
      margin-top: 2px;
      margin-bottom: 14px;
    }
    .config-section h2 {
      margin: 0 0 10px 0;
      font-size: 14px;
      font-weight: 700;
      color: #073b66;
    }
    .system-actions {
      display: grid;
      gap: 12px;
    }
    .system-action {
      border: 1px solid rgba(121,116,126,0.28);
      border-radius: 14px;
      padding: 12px;
      background: rgba(255,255,255,0.8);
    }
    .system-action h3 {
      margin: 0 0 6px 0;
      font-size: 14px;
      font-weight: 700;
      color: #073b66;
    }
    .system-action p {
      margin: 0 0 10px 0;
      font-size: 13px;
      color: #516170;
    }
    .control-list {
      display: grid;
      gap: 6px;
      max-width: 640px;
      margin: 0 auto;
    }
    .control-item {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 12px;
      padding: 10px 4px;
      border: none;
      background: transparent;
    }
    .control-name {
      font-size: 14px;
      font-weight: 600;
      color: #073b66;
    }
    .control-card {
      border: none;
      background: transparent;
      padding: 6px 0;
    }
    .md3-switch {
      position: relative;
      width: 52px;
      height: 32px;
      flex: 0 0 auto;
    }
    .md3-switch input {
      position: absolute;
      inset: 0;
      margin: 0;
      padding: 0;
      border: 0;
      opacity: 0;
      cursor: pointer;
      appearance: none;
      -webkit-appearance: none;
    }
    .md3-track {
      position: absolute;
      inset: 0;
      border-radius: 999px;
      border: 2px solid #7c8ea4;
      background: #e5edf5;
      transition: all 0.18s ease;
    }
    .md3-thumb {
      position: absolute;
      top: 50%;
      left: 8px;
      width: 14px;
      height: 14px;
      border-radius: 50%;
      background: #3e4f62;
      transform: translateY(-50%);
      transition: all 0.18s ease;
    }
    .md3-switch input:checked + .md3-track {
      border-color: #0066ff;
      background: #9dc7ff;
    }
    .md3-switch input:checked + .md3-track + .md3-thumb {
      left: 30px;
      background: #ffffff;
    }
    .md3-switch input:focus-visible + .md3-track {
      box-shadow: 0 0 0 3px rgba(0, 102, 255, 0.25);
    }
    .overlay { position: fixed; inset: 0; background: rgba(24,20,38,0.25); display: none; z-index: 10; }
    @media (max-width: 900px) {
      .drawer {
        position: fixed; left: 0; top: 0; bottom: 0;
        width: min(280px, calc(100vw - 32px));
        transform: translateX(-104%);
        transition: transform 0.2s ease;
        box-shadow: 0 10px 28px rgba(7, 59, 102, 0.22);
      }
      .drawer.mobile-open { transform: translateX(0); }
      .drawer.collapsed { width: 280px; }
      .drawer.collapsed .drawer-user, .drawer.collapsed .menu-item .label { display: inline; }
      .overlay.visible { display: block; }
      .content { width: 100%; padding: 12px; }
      .mobile-topbar {
        position: sticky;
        top: 0;
        z-index: 5;
        display: flex;
        align-items: center;
        gap: 10px;
        margin: -12px -12px 12px -12px;
        padding: 10px 12px;
        border-bottom: 1px solid rgba(121,116,126,0.35);
        background: #ffffff;
      }
      .form-grid { grid-template-columns: 1fr; }
    }
  </style>
</head>
<body>
  <div id="overlay" class="overlay"></div>
  <div class="app">
    <aside id="drawer" class="drawer">
      <div class="drawer-header">
        <button id="menuToggle" class="menu-btn" data-menu-toggle aria-label="Toggle menu">=</button>
        <div class="drawer-user">Superviseur</div>
      </div>
      <nav class="menu-group">
        <button class="menu-item active" data-page="page-terminal"><span class="ico"><img class="ico-svg" src="/assets/icon-journaux.svg" alt="" /></span><span class="label">Journaux</span></button>
        <button class="menu-item" data-page="page-upgrade"><span class="ico"><img class="ico-svg" src="/assets/icon-upgrade.svg" alt="" /></span><span class="label">Mise à jour Firmware</span></button>
        <button class="menu-item" data-page="page-config"><span class="ico"><img class="ico-svg" src="/assets/icon-config.svg?v=2" alt="" /></span><span class="label">Configuration</span></button>
        <button class="menu-item" data-page="page-system"><span class="ico"><img class="ico-svg" src="/assets/icon-system.svg" alt="" /></span><span class="label">Système</span></button>
        <button class="menu-item" data-page="page-control"><span class="ico"><img class="ico-svg" src="/assets/icon-control.svg" alt="" /></span><span class="label">Contrôle</span></button>
      </nav>
      <div class="drawer-footer">
        <span class="app-icon-shell"><img class="app-icon" src="/assets/flowio-logo.svg" alt="Flow.IO" /></span>
      </div>
    </aside>

    <main class="content">
      <div class="mobile-topbar">
        <button class="menu-btn" data-menu-toggle aria-label="Open menu">=</button>
        <div class="mobile-title">Superviseur</div>
      </div>
      <section id="page-terminal" class="page active">
        <div class="topbar">
          <h1>Journaux</h1>
          <span id="wsStatus" class="status-chip">connexion...</span>
        </div>
        <div class="card">
          <div id="term" class="terminal"></div>
          <div class="term-toolbar">
            <button id="toggleAutoscroll" class="btn-tonal" aria-pressed="true">Défilement auto : activé</button>
            <input id="line" placeholder="Envoyer une ligne vers l'UART" />
            <button id="send" class="btn-tonal">Envoyer</button>
            <button id="clear">Effacer</button>
          </div>
        </div>
      </section>

      <section id="page-upgrade" class="page">
        <div class="topbar">
          <h1>Mise à jour Firmware</h1>
          <span id="upStatusChip" class="status-chip">inactif</span>
        </div>
        <div class="card">
          <div class="form-grid">
            <div class="field full">
              <label for="updateHost">Serveur HTTP (nom d'hôte ou IP, protocole optionnel)</label>
              <input id="updateHost" placeholder="ex. 192.168.1.20 ou http://192.168.1.20" />
            </div>
            <div class="field">
              <label for="flowPath">Chemin image Flow.IO</label>
              <input id="flowPath" placeholder="/build/FlowIO.bin" />
            </div>
            <div class="field">
              <label for="nextionPath">Chemin image Nextion</label>
              <input id="nextionPath" placeholder="/build/Nextion.tft" />
            </div>
          </div>
          <div class="btn-row">
            <button id="saveCfg" class="btn-tonal">Enregistrer la configuration</button>
            <button id="upFlow" class="btn-primary">Mettre à jour Flow.IO</button>
            <button id="upNextion" class="btn-primary">Mettre à jour Nextion</button>
            <button id="refreshState">Rafraîchir l'état</button>
          </div>
          <div class="upgrade-progress" aria-label="Progression mise à jour firmware">
            <div id="upgradeProgressBar" class="upgrade-progress-bar"></div>
          </div>
          <div id="upgradeStatusText" class="upgrade-status">Aucune opération en cours.</div>
        </div>
      </section>

      <section id="page-config" class="page">
        <div class="topbar">
          <h1>Configuration</h1>
          <span class="status-chip">WiFi + MQTT</span>
        </div>
        <div class="card">
          <div class="config-section">
            <h2>WiFi</h2>
            <div class="form-grid">
              <div class="field">
                <label for="wifiEnabled">Activer WiFi (1/0)</label>
                <input id="wifiEnabled" type="number" min="0" max="1" placeholder="1" />
              </div>
              <div class="field">
                <label for="wifiSsid">SSID</label>
                <input id="wifiSsid" placeholder="Nom du réseau WiFi" />
              </div>
              <div class="field full">
                <label for="wifiSsidList">Réseaux détectés</label>
                <select id="wifiSsidList">
                  <option value="">Scan requis...</option>
                </select>
              </div>
              <div class="field full">
                <label for="wifiPass">Mot de passe</label>
                <div class="password-wrap">
                  <input id="wifiPass" type="password" placeholder="Mot de passe WiFi" />
                  <button id="toggleWifiPass" class="password-toggle" type="button" aria-label="Afficher le mot de passe WiFi" aria-pressed="false"></button>
                </div>
              </div>
            </div>
            <div class="btn-row">
              <button id="scanWifi" class="btn-tonal">Scanner les réseaux</button>
              <button id="applyWifiCfg" class="btn-primary">Appliquer WiFi</button>
            </div>
            <div id="wifiConfigStatus" class="config-status">Configuration WiFi prête.</div>
          </div>

          <div class="config-section">
            <h2>MQTT</h2>
            <div class="form-grid">
              <div class="field full">
                <label for="mqttServer">Serveur MQTT</label>
                <input id="mqttServer" placeholder="ex. 192.168.1.20" />
              </div>
              <div class="field">
                <label for="mqttPort">Port</label>
                <input id="mqttPort" type="number" min="1" max="65535" placeholder="1883" />
              </div>
              <div class="field">
                <label for="mqttUser">Identifiant</label>
                <input id="mqttUser" placeholder="identifiant" />
              </div>
              <div class="field full">
                <label for="mqttPass">Mot de passe</label>
                <div class="password-wrap">
                  <input id="mqttPass" type="password" placeholder="mot de passe" />
                  <button id="toggleMqttPass" class="password-toggle" type="button" aria-label="Afficher le mot de passe MQTT" aria-pressed="false"></button>
                </div>
              </div>
            </div>
            <div class="btn-row">
              <button id="applyMqttCfg" class="btn-primary">Appliquer MQTT</button>
            </div>
            <div id="mqttConfigStatus" class="config-status">Configuration MQTT prête.</div>
          </div>
        </div>
      </section>

      <section id="page-system" class="page">
        <div class="topbar">
          <h1>Système</h1>
          <span class="status-chip">administrateur</span>
        </div>
        <div class="card">
          <div class="system-actions">
            <div class="system-action">
              <h3>Redémarrer le Superviseur</h3>
              <p>Redémarre uniquement le Superviseur pour relancer les services sans modifier la configuration.</p>
              <button id="rebootSupervisor" class="btn-tonal">Redémarrer</button>
            </div>
            <div class="system-action">
              <h3>Réinitialisation usine</h3>
              <p>Efface la configuration enregistrée puis redémarre l'équipement avec les valeurs d'usine.</p>
              <button id="factoryReset" class="btn-primary">Réinitialisation usine</button>
            </div>
          </div>
          <div id="systemStatusText" class="config-status">Aucune action système en cours.</div>
        </div>
      </section>

      <section id="page-control" class="page">
        <div class="topbar">
          <h1>Contrôle</h1>
          <span class="status-chip">manuel</span>
        </div>
        <div class="card control-card">
          <div class="control-list">
            <label class="control-item"><span class="control-name">Remplissage</span><span class="md3-switch"><input type="checkbox" /><span class="md3-track"></span><span class="md3-thumb"></span></span></label>
            <label class="control-item"><span class="control-name">Electrolyseur</span><span class="md3-switch"><input type="checkbox" /><span class="md3-track"></span><span class="md3-thumb"></span></span></label>
            <label class="control-item"><span class="control-name">Filtration</span><span class="md3-switch"><input type="checkbox" /><span class="md3-track"></span><span class="md3-thumb"></span></span></label>
            <label class="control-item"><span class="control-name">Robot</span><span class="md3-switch"><input type="checkbox" /><span class="md3-track"></span><span class="md3-thumb"></span></span></label>
            <label class="control-item"><span class="control-name">Pompe pH</span><span class="md3-switch"><input type="checkbox" /><span class="md3-track"></span><span class="md3-thumb"></span></span></label>
            <label class="control-item"><span class="control-name">Pompe Chlore</span><span class="md3-switch"><input type="checkbox" /><span class="md3-track"></span><span class="md3-thumb"></span></span></label>
            <label class="control-item"><span class="control-name">Eclairage</span><span class="md3-switch"><input type="checkbox" /><span class="md3-track"></span><span class="md3-thumb"></span></span></label>
            <label class="control-item"><span class="control-name">Régulation pH</span><span class="md3-switch"><input type="checkbox" /><span class="md3-track"></span><span class="md3-thumb"></span></span></label>
            <label class="control-item"><span class="control-name">Régulation Orp</span><span class="md3-switch"><input type="checkbox" /><span class="md3-track"></span><span class="md3-thumb"></span></span></label>
            <label class="control-item"><span class="control-name">Mode automatique</span><span class="md3-switch"><input type="checkbox" /><span class="md3-track"></span><span class="md3-thumb"></span></span></label>
            <label class="control-item"><span class="control-name">Hivernage</span><span class="md3-switch"><input type="checkbox" /><span class="md3-track"></span><span class="md3-thumb"></span></span></label>
          </div>
        </div>
      </section>
    </main>
  </div>

  <script>
    const drawer = document.getElementById('drawer');
    const overlay = document.getElementById('overlay');
    const menuToggles = Array.from(document.querySelectorAll('[data-menu-toggle]'));
    const menuItems = Array.from(document.querySelectorAll('.menu-item'));
    const pages = Array.from(document.querySelectorAll('.page'));

    function isMobileLayout() {
      return window.innerWidth <= 900;
    }

    function setMobileDrawerOpen(open) {
      drawer.classList.toggle('mobile-open', open);
      overlay.classList.toggle('visible', open);
    }

    function closeMobileDrawer() {
      if (isMobileLayout()) {
        setMobileDrawerOpen(false);
      }
    }

    function showPage(pageId) {
      pages.forEach((el) => el.classList.toggle('active', el.id === pageId));
      menuItems.forEach((el) => el.classList.toggle('active', el.dataset.page === pageId));
      closeMobileDrawer();
    }

    async function ouvrirPageParDefautSelonModeReseau() {
      try {
        const res = await fetch('/api/network/mode', { cache: 'no-store' });
        const data = await res.json();
        if (res.ok && data && data.ok === true && data.mode === 'ap') {
          showPage('page-config');
        }
      } catch (err) {
        // Keep default page when network mode endpoint is unavailable.
      }
    }

    menuItems.forEach((item) => item.addEventListener('click', () => showPage(item.dataset.page)));

    menuToggles.forEach((btn) => btn.addEventListener('click', () => {
      if (isMobileLayout()) {
        setMobileDrawerOpen(!drawer.classList.contains('mobile-open'));
      } else {
        drawer.classList.toggle('collapsed');
      }
    }));

    overlay.addEventListener('click', closeMobileDrawer);
    window.addEventListener('resize', () => {
      if (!isMobileLayout()) {
        setMobileDrawerOpen(false);
      }
    });

    const term = document.getElementById('term');
    const wsStatus = document.getElementById('wsStatus');
    const line = document.getElementById('line');
    const sendBtn = document.getElementById('send');
    const clearBtn = document.getElementById('clear');
    const toggleAutoscrollBtn = document.getElementById('toggleAutoscroll');
    let autoScrollEnabled = true;

    const updateHost = document.getElementById('updateHost');
    const flowPath = document.getElementById('flowPath');
    const nextionPath = document.getElementById('nextionPath');
    const saveCfgBtn = document.getElementById('saveCfg');
    const upFlowBtn = document.getElementById('upFlow');
    const upNextionBtn = document.getElementById('upNextion');
    const refreshStateBtn = document.getElementById('refreshState');
    const upgradeStatusText = document.getElementById('upgradeStatusText');
    const upgradeProgressBar = document.getElementById('upgradeProgressBar');
    const upStatusChip = document.getElementById('upStatusChip');

    const mqttServer = document.getElementById('mqttServer');
    const mqttPort = document.getElementById('mqttPort');
    const mqttUser = document.getElementById('mqttUser');
    const mqttPass = document.getElementById('mqttPass');
    const toggleMqttPassBtn = document.getElementById('toggleMqttPass');
    const applyMqttCfgBtn = document.getElementById('applyMqttCfg');
    const mqttConfigStatus = document.getElementById('mqttConfigStatus');
    const wifiEnabled = document.getElementById('wifiEnabled');
    const wifiSsid = document.getElementById('wifiSsid');
    const wifiSsidList = document.getElementById('wifiSsidList');
    const wifiPass = document.getElementById('wifiPass');
    const toggleWifiPassBtn = document.getElementById('toggleWifiPass');
    const scanWifiBtn = document.getElementById('scanWifi');
    const applyWifiCfgBtn = document.getElementById('applyWifiCfg');
    const wifiConfigStatus = document.getElementById('wifiConfigStatus');
    const rebootSupervisorBtn = document.getElementById('rebootSupervisor');
    const factoryResetBtn = document.getElementById('factoryReset');
    const systemStatusText = document.getElementById('systemStatusText');
    let wifiScanPollTimer = null;

    const wsProto = location.protocol === 'https:' ? 'wss' : 'ws';
    const ws = new WebSocket(wsProto + '://' + location.host + '/wsserial');

    ws.onopen = () => wsStatus.textContent = 'connecté';
    ws.onclose = () => wsStatus.textContent = 'déconnecté';
    ws.onerror = () => wsStatus.textContent = 'erreur';

    const ansiState = { fg: null };
    const ansiFgMap = {
      30: '#94a3b8', 31: '#ef4444', 32: '#22c55e', 33: '#f59e0b',
      34: '#60a5fa', 35: '#f472b6', 36: '#22d3ee', 37: '#e2e8f0',
      90: '#64748b', 91: '#f87171', 92: '#4ade80', 93: '#fbbf24',
      94: '#93c5fd', 95: '#f9a8d4', 96: '#67e8f9', 97: '#f8fafc'
    };
    const ansiRe = /\u001b\[([0-9;]*)m/g;

    function applySgrCodes(rawCodes) {
      const codes = rawCodes === '' ? [0] : rawCodes.split(';').map((v) => parseInt(v, 10)).filter(Number.isFinite);
      for (const code of codes) {
        if (code === 0 || code === 39) {
          ansiState.fg = null;
          continue;
        }
        if (Object.prototype.hasOwnProperty.call(ansiFgMap, code)) {
          ansiState.fg = ansiFgMap[code];
        }
      }
    }

    function decodeAnsiLine(rawLine) {
      let out = '';
      let cursor = 0;
      let lineColor = ansiState.fg;
      rawLine.replace(ansiRe, (full, codes, idx) => {
        out += rawLine.slice(cursor, idx);
        applySgrCodes(codes);
        if (ansiState.fg) lineColor = ansiState.fg;
        cursor = idx + full.length;
        return '';
      });
      out += rawLine.slice(cursor);
      return { text: out, color: lineColor };
    }

    ws.onmessage = (ev) => {
      const raw = String(ev.data || '');
      const parsed = decodeAnsiLine(raw);
      const row = document.createElement('div');
      row.className = 'log-line';
      if (parsed.color) row.style.color = parsed.color;
      row.textContent = parsed.text;
      term.appendChild(row);
      while (term.childNodes.length > 2000) term.removeChild(term.firstChild);
      if (autoScrollEnabled) term.scrollTop = term.scrollHeight;
    };

    function refreshAutoscrollUi() {
      toggleAutoscrollBtn.textContent = autoScrollEnabled ? 'Défilement auto : activé' : 'Défilement auto : désactivé';
      toggleAutoscrollBtn.setAttribute('aria-pressed', autoScrollEnabled ? 'true' : 'false');
      toggleAutoscrollBtn.classList.toggle('btn-tonal', autoScrollEnabled);
      toggleAutoscrollBtn.classList.toggle('btn-toggle-off', !autoScrollEnabled);
    }

    function sendLine() {
      const txt = line.value;
      if (!txt) return;
      if (ws.readyState === WebSocket.OPEN) ws.send(txt);
      line.value = '';
      line.focus();
    }

    const iconeOeilOuvert =
      '<svg viewBox="0 0 24 24" aria-hidden="true"><path fill="currentColor" d="M12 5C6.7 5 2.73 8.11 1.16 11.25a1.7 1.7 0 0 0 0 1.5C2.73 15.89 6.7 19 12 19s9.27-3.11 10.84-6.25a1.7 1.7 0 0 0 0-1.5C21.27 8.11 17.3 5 12 5Zm0 12c-4.46 0-7.88-2.66-9.29-5 .64-1.06 1.74-2.21 3.17-3.11A11.7 11.7 0 0 1 12 7c4.46 0 7.88 2.66 9.29 5-.64 1.06-1.74 2.21-3.17 3.11A11.7 11.7 0 0 1 12 17Zm0-6.4A2.4 2.4 0 1 0 14.4 13 2.4 2.4 0 0 0 12 10.6Z"/></svg>';
    const iconeOeilBarre =
      '<svg viewBox="0 0 24 24" aria-hidden="true"><path fill="currentColor" d="M3.28 2.22 2.22 3.28l2.64 2.64c-1.54 1.14-2.84 2.65-3.7 4.34a1.7 1.7 0 0 0 0 1.5C2.73 14.89 6.7 18 12 18c2.2 0 4.18-.53 5.93-1.38l2.79 2.79 1.06-1.06ZM12 16.2c-4.46 0-7.88-2.66-9.29-5 .73-1.21 2.09-2.57 3.9-3.53l1.66 1.66a3.8 3.8 0 0 0 5.4 5.4l1.75 1.75c-1.02.44-2.16.72-3.42.72Zm.04-9.4 4.35 4.35c0-.05.01-.1.01-.15a4.4 4.4 0 0 0-4.4-4.4h-.01a.8.8 0 0 1 .05.2Zm9.8 4.45C20.27 8.11 16.3 5 11 5c-1.8 0-3.46.35-4.97.95l1.47 1.47A11.5 11.5 0 0 1 12 6.8c4.46 0 7.88 2.66 9.29 5-.43.71-1.03 1.47-1.79 2.16l1.28 1.28c.86-.78 1.56-1.65 2.06-2.63a1.7 1.7 0 0 0 0-1.36Z"/></svg>';

    function mettreAJourEtatVisibiliteMotDePasse(inputEl, toggleBtn, labelAfficher, labelMasquer) {
      if (!inputEl || !toggleBtn) return;
      const isVisible = inputEl.type === 'text';
      toggleBtn.innerHTML = isVisible ? iconeOeilOuvert : iconeOeilBarre;
      toggleBtn.setAttribute('aria-pressed', isVisible ? 'true' : 'false');
      toggleBtn.setAttribute('aria-label', isVisible ? labelMasquer : labelAfficher);
      toggleBtn.setAttribute('title', isVisible ? 'Mot de passe en clair' : 'Mot de passe masqué');
    }

    function basculerVisibiliteMotDePasse(inputEl, toggleBtn, labelAfficher, labelMasquer) {
      if (!inputEl || !toggleBtn) return;
      const isMasked = inputEl.type === 'password';
      inputEl.type = isMasked ? 'text' : 'password';
      mettreAJourEtatVisibiliteMotDePasse(inputEl, toggleBtn, labelAfficher, labelMasquer);
    }
    sendBtn.addEventListener('click', sendLine);
    line.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') sendLine();
    });
    clearBtn.addEventListener('click', () => { term.textContent = ''; });
    toggleAutoscrollBtn.addEventListener('click', () => {
      autoScrollEnabled = !autoScrollEnabled;
      refreshAutoscrollUi();
      if (autoScrollEnabled) term.scrollTop = term.scrollHeight;
    });
    refreshAutoscrollUi();

    function setUpgradeProgress(value) {
      const p = Math.max(0, Math.min(100, Number(value) || 0));
      upgradeProgressBar.style.width = p + '%';
    }

    function setUpgradeMessage(text) {
      upgradeStatusText.textContent = text;
    }

    function updateUpgradeView(data) {
      if (!data || data.ok !== true) return;
      const state = data.state || 'inconnu';
      const target = data.target || '-';
      const progress = Number.isFinite(data.progress) ? data.progress : 0;
      const msg = data.msg || '';
      let stateLabel = state;
      if (state === 'idle') stateLabel = 'inactif';
      else if (state === 'queued') stateLabel = 'en attente';
      else if (state === 'running') stateLabel = 'en cours';
      else if (state === 'done') stateLabel = 'terminé';
      else if (state === 'error') stateLabel = 'erreur';
      upStatusChip.textContent = stateLabel;
      let p = progress;
      if (state === 'done') p = 100;
      if (state === 'queued' && p <= 0) p = 2;
      setUpgradeProgress(p);
      setUpgradeMessage(stateLabel + ' | cible=' + target + (msg ? ' | ' + msg : ''));
    }

    async function loadUpgradeConfig() {
      try {
        const res = await fetch('/api/fwupdate/config', { cache: 'no-store' });
        const data = await res.json();
        if (data && data.ok) {
          updateHost.value = data.update_host || '';
          flowPath.value = data.flowio_path || '';
          nextionPath.value = data.nextion_path || '';
        }
      } catch (err) {
        setUpgradeMessage('Échec du chargement de la configuration : ' + err);
      }
    }

    async function saveUpgradeConfig() {
      const body = new URLSearchParams();
      body.set('update_host', updateHost.value.trim());
      body.set('flowio_path', flowPath.value.trim());
      body.set('nextion_path', nextionPath.value.trim());
      const res = await fetch('/api/fwupdate/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded;charset=UTF-8' },
        body: body.toString()
      });
      const data = await res.json().catch(() => ({}));
      if (!res.ok || !data.ok) throw new Error('échec enregistrement');
      setUpgradeMessage('Configuration enregistrée.');
    }

    async function refreshUpgradeStatus() {
      try {
        const res = await fetch('/api/fwupdate/status', { cache: 'no-store' });
        const data = await res.json();
        if (data && data.ok) updateUpgradeView(data);
      } catch (err) {
        setUpgradeMessage('Échec de lecture de l\'état : ' + err);
      }
    }

    async function startUpgrade(target) {
      try {
        await saveUpgradeConfig();
        const endpoint = target === 'flowio' ? '/fwupdate/flowio' : '/fwupdate/nextion';
        const res = await fetch(endpoint, { method: 'POST' });
        const data = await res.json().catch(() => ({}));
        if (!res.ok || !data.ok) throw new Error('échec démarrage');
        setUpgradeProgress(1);
        setUpgradeMessage('Demande de mise à jour acceptée pour ' + target + '.');
        await refreshUpgradeStatus();
      } catch (err) {
        setUpgradeMessage('Échec de la mise à jour : ' + err);
      }
    }

    async function loadMqttConfig() {
      try {
        const res = await fetch('/api/mqtt/config', { cache: 'no-store' });
        const data = await res.json();
        if (data && data.ok) {
          mqttServer.value = data.server || '';
          mqttPort.value = Number.isFinite(data.port) ? String(data.port) : '1883';
          mqttUser.value = data.username || '';
          mqttPass.value = data.password || '';
          mqttConfigStatus.textContent = 'Configuration MQTT chargée.';
        }
      } catch (err) {
        mqttConfigStatus.textContent = 'Chargement MQTT échoué: ' + err;
      }
    }

    async function saveMqttConfig() {
      const body = new URLSearchParams();
      body.set('server', mqttServer.value.trim());
      body.set('port', (mqttPort.value || '1883').trim());
      body.set('username', mqttUser.value.trim());
      body.set('password', mqttPass.value);

      const res = await fetch('/api/mqtt/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded;charset=UTF-8' },
        body: body.toString()
      });
      const data = await res.json().catch(() => ({}));
      if (!res.ok || !data.ok) throw new Error('échec application');
      mqttConfigStatus.textContent = 'Configuration MQTT appliquée.';
    }

    function stopWifiScanPolling() {
      if (wifiScanPollTimer) {
        clearTimeout(wifiScanPollTimer);
        wifiScanPollTimer = null;
      }
    }

    function scheduleWifiScanPolling() {
      stopWifiScanPolling();
      wifiScanPollTimer = setTimeout(() => {
        refreshWifiScanStatus(false);
      }, 1200);
    }

    function renderWifiScanList(data) {
      const networks = (data && Array.isArray(data.networks)) ? data.networks : [];
      const currentSsid = (wifiSsid.value || '').trim();
      const prevSelected = wifiSsidList.value || '';

      wifiSsidList.innerHTML = '';
      const manualOpt = document.createElement('option');
      manualOpt.value = '';
      manualOpt.textContent = 'Saisie manuelle';
      wifiSsidList.appendChild(manualOpt);

      for (const net of networks) {
        if (!net || typeof net.ssid !== 'string' || net.ssid.length === 0) continue;
        if (net.hidden) continue;
        const opt = document.createElement('option');
        const secureSuffix = net.secure ? ' (securise)' : ' (ouvert)';
        const rssiSuffix = Number.isFinite(net.rssi) ? (' ' + net.rssi + ' dBm') : '';
        opt.value = net.ssid;
        opt.textContent = net.ssid + secureSuffix + rssiSuffix;
        wifiSsidList.appendChild(opt);
      }

      const values = Array.from(wifiSsidList.options).map((o) => o.value);
      if (currentSsid && values.includes(currentSsid)) {
        wifiSsidList.value = currentSsid;
      } else if (prevSelected && values.includes(prevSelected)) {
        wifiSsidList.value = prevSelected;
      } else {
        wifiSsidList.value = '';
      }
    }

    function updateWifiScanStatusText(data, reqError) {
      if (reqError) {
        wifiConfigStatus.textContent = 'Scan WiFi indisponible: ' + reqError;
        return;
      }
      if (!data || data.ok !== true) {
        wifiConfigStatus.textContent = 'Scan WiFi : réponse invalide.';
        return;
      }

      const running = !!data.running;
      const requested = !!data.requested;
      const count = Number.isFinite(data.count) ? data.count : 0;
      const totalFound = Number.isFinite(data.total_found) ? data.total_found : count;
      if (running || requested) {
        wifiConfigStatus.textContent = 'Scan WiFi en cours...';
        return;
      }
      if (count > 0) {
        wifiConfigStatus.textContent = 'Scan WiFi terminé : ' + count + ' réseaux affichés (' + totalFound + ' détectés).';
        return;
      }
      wifiConfigStatus.textContent = 'Aucun réseau visible détecté.';
    }

    async function requestWifiScan(force) {
      const body = new URLSearchParams();
      body.set('force', force ? '1' : '0');
      const res = await fetch('/api/wifi/scan', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded;charset=UTF-8' },
        body: body.toString()
      });
      const data = await res.json().catch(() => ({}));
      if (!res.ok || !data.ok) throw new Error('échec démarrage scan');
      return data;
    }

    async function refreshWifiScanStatus(triggerScan) {
      try {
        if (triggerScan) {
          await requestWifiScan(true);
        }
        const res = await fetch('/api/wifi/scan', { cache: 'no-store' });
        const data = await res.json();
        if (!res.ok || !data || data.ok !== true) throw new Error('échec lecture état');

        renderWifiScanList(data);
        updateWifiScanStatusText(data, null);

        if (data.running || data.requested) {
          scheduleWifiScanPolling();
        } else {
          stopWifiScanPolling();
        }
      } catch (err) {
        stopWifiScanPolling();
        updateWifiScanStatusText(null, err);
      }
    }

    async function loadWifiConfig() {
      try {
        const res = await fetch('/api/wifi/config', { cache: 'no-store' });
        const data = await res.json();
        if (data && data.ok) {
          wifiEnabled.value = data.enabled ? '1' : '0';
          wifiSsid.value = data.ssid || '';
          wifiPass.value = data.pass || '';
          wifiConfigStatus.textContent = 'Configuration WiFi chargée.';
        }
      } catch (err) {
        wifiConfigStatus.textContent = 'Chargement WiFi échoué: ' + err;
      }
    }

    async function saveWifiConfig() {
      const body = new URLSearchParams();
      body.set('enabled', (wifiEnabled.value || '1').trim());
      body.set('ssid', wifiSsid.value.trim());
      body.set('pass', wifiPass.value);

      const res = await fetch('/api/wifi/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded;charset=UTF-8' },
        body: body.toString()
      });
      const data = await res.json().catch(() => ({}));
      if (!res.ok || !data.ok) throw new Error('échec application');
      wifiConfigStatus.textContent = 'Configuration WiFi appliquée (reconnexion en cours).';
    }

    async function callSystemAction(action) {
      const endpoint = action === 'factory_reset' ? '/api/system/factory-reset' : '/api/system/reboot';
      const res = await fetch(endpoint, { method: 'POST' });
      const data = await res.json().catch(() => ({}));
      if (!res.ok || !data.ok) throw new Error('échec action');
      systemStatusText.textContent = action === 'factory_reset'
        ? 'Réinitialisation usine lancée. Redémarrage en cours...'
        : 'Redémarrage du Superviseur lancé...';
    }

    saveCfgBtn.addEventListener('click', async () => {
      try {
        await saveUpgradeConfig();
      } catch (err) {
        setUpgradeMessage('Échec de l\'enregistrement : ' + err);
      }
    });
    upFlowBtn.addEventListener('click', () => startUpgrade('flowio'));
    upNextionBtn.addEventListener('click', () => startUpgrade('nextion'));
    refreshStateBtn.addEventListener('click', refreshUpgradeStatus);
    applyMqttCfgBtn.addEventListener('click', async () => {
      try {
        await saveMqttConfig();
      } catch (err) {
        mqttConfigStatus.textContent = 'Application MQTT échouée: ' + err;
      }
    });
    if (toggleWifiPassBtn && wifiPass) {
      mettreAJourEtatVisibiliteMotDePasse(
        wifiPass,
        toggleWifiPassBtn,
        'Afficher le mot de passe WiFi',
        'Masquer le mot de passe WiFi'
      );
      toggleWifiPassBtn.addEventListener('click', () => {
        basculerVisibiliteMotDePasse(
          wifiPass,
          toggleWifiPassBtn,
          'Afficher le mot de passe WiFi',
          'Masquer le mot de passe WiFi'
        );
      });
    }
    if (toggleMqttPassBtn && mqttPass) {
      mettreAJourEtatVisibiliteMotDePasse(
        mqttPass,
        toggleMqttPassBtn,
        'Afficher le mot de passe MQTT',
        'Masquer le mot de passe MQTT'
      );
      toggleMqttPassBtn.addEventListener('click', () => {
        basculerVisibiliteMotDePasse(
          mqttPass,
          toggleMqttPassBtn,
          'Afficher le mot de passe MQTT',
          'Masquer le mot de passe MQTT'
        );
      });
    }
    wifiSsidList.addEventListener('change', () => {
      const picked = (wifiSsidList.value || '').trim();
      if (picked.length > 0) {
        wifiSsid.value = picked;
      }
    });
    scanWifiBtn.addEventListener('click', () => {
      refreshWifiScanStatus(true);
    });
    applyWifiCfgBtn.addEventListener('click', async () => {
      try {
        await saveWifiConfig();
      } catch (err) {
        wifiConfigStatus.textContent = 'Application WiFi échouée: ' + err;
      }
    });
    rebootSupervisorBtn.addEventListener('click', async () => {
      try {
        await callSystemAction('reboot');
      } catch (err) {
        systemStatusText.textContent = 'Redémarrage échoué : ' + err;
      }
    });
    factoryResetBtn.addEventListener('click', async () => {
      if (!confirm('Confirmer la réinitialisation usine ? Cette action efface la configuration.')) return;
      try {
        await callSystemAction('factory_reset');
      } catch (err) {
        systemStatusText.textContent = 'Réinitialisation usine échouée : ' + err;
      }
    });

    loadUpgradeConfig();
    setUpgradeProgress(0);
    loadWifiConfig();
    loadMqttConfig();
    refreshWifiScanStatus(true);
    ouvrirPageParDefautSelonModeReseau();
    refreshUpgradeStatus();
    setInterval(refreshUpgradeStatus, 2000);
  </script>
</body>
</html>
)HTML";

void WebInterfaceModule::init(ConfigStore& cfg, ServiceRegistry& services)
{
    constexpr uint8_t kCfgModuleId = (uint8_t)ConfigModuleId::Mqtt;
    constexpr uint16_t kCfgBranchId = (uint16_t)ConfigBranchId::Mqtt;
    cfgStore_ = &cfg;
    cfg.registerVar(mqttHostVar_, kCfgModuleId, kCfgBranchId);
    cfg.registerVar(mqttPortVar_, kCfgModuleId, kCfgBranchId);
    cfg.registerVar(mqttUserVar_, kCfgModuleId, kCfgBranchId);
    cfg.registerVar(mqttPassVar_, kCfgModuleId, kCfgBranchId);

    services_ = &services;
    logHub_ = services.get<LogHubService>("loghub");
    wifiSvc_ = services.get<WifiService>("wifi");
    cmdSvc_ = services.get<CommandService>("cmd");
    netAccessSvc_ = services.get<NetworkAccessService>("network_access");
    const DataStoreService* dsSvc = services.get<DataStoreService>("datastore");
    dataStore_ = dsSvc ? dsSvc->store : nullptr;
    auto* ebSvc = services.get<EventBusService>("eventbus");
    eventBus_ = ebSvc ? ebSvc->bus : nullptr;
    fwUpdateSvc_ = services.get<FirmwareUpdateService>("fwupdate");
    if (eventBus_) {
        eventBus_->subscribe(EventId::DataChanged, &WebInterfaceModule::onEventStatic_, this);
    }

    static WebInterfaceService webInterfaceSvc{
        &WebInterfaceModule::svcSetPaused_,
        &WebInterfaceModule::svcIsPaused_,
        nullptr
    };
    webInterfaceSvc.ctx = this;
    services.add("webinterface", &webInterfaceSvc);

    uart_.setRxBufferSize(kUartRxBufferSize);
    uart_.begin(kUartBaud, SERIAL_8N1, kUartRxPin, kUartTxPin);
    netReady_ = dataStore_ ? wifiReady(*dataStore_) : false;
    LOGI("WebInterface init uart=Serial2 baud=%lu rx=%d tx=%d line_buf=%u rx_buf=%u (server deferred)",
         (unsigned long)kUartBaud,
         kUartRxPin,
         kUartTxPin,
         (unsigned)kLineBufferSize,
         (unsigned)kUartRxBufferSize);
}

void WebInterfaceModule::startServer_()
{
    if (started_) return;

    server_.on("/assets/flowio-icon.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "image/svg+xml", kFlowIoIconSvg);
    });
    server_.on("/assets/flowio-logo.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "image/svg+xml", kFlowIoLogoSvg);
    });
    server_.on("/assets/icon-journaux.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "image/svg+xml", kMenuIconJournauxSvg);
    });
    server_.on("/assets/icon-upgrade.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "image/svg+xml", kMenuIconUpgradeSvg);
    });
    server_.on("/assets/icon-config.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "image/svg+xml", kMenuIconConfigSvg);
    });
    server_.on("/assets/icon-system.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "image/svg+xml", kMenuIconSystemSvg);
    });
    server_.on("/assets/icon-control.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "image/svg+xml", kMenuIconControlSvg);
    });

    server_.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->redirect("/webinterface");
    });

    server_.on("/webinterface", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/html", kWebInterfacePage);
    });
    server_.on("/webserial", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->redirect("/webinterface");
    });

    server_.on("/webinterface/health", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/plain", "ok");
    });
    server_.on("/webserial/health", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->redirect("/webinterface/health");
    });
    server_.on("/api/network/mode", HTTP_GET, [this](AsyncWebServerRequest* request) {
        NetworkAccessMode mode = NetworkAccessMode::None;
        if (!netAccessSvc_ && services_) {
            netAccessSvc_ = services_->get<NetworkAccessService>("network_access");
        }
        if (netAccessSvc_ && netAccessSvc_->mode) {
            mode = netAccessSvc_->mode(netAccessSvc_->ctx);
        } else if (wifiSvc_ && wifiSvc_->isConnected && wifiSvc_->isConnected(wifiSvc_->ctx)) {
            mode = NetworkAccessMode::Station;
        }

        const char* modeTxt = "none";
        if (mode == NetworkAccessMode::Station) modeTxt = "station";
        else if (mode == NetworkAccessMode::AccessPoint) modeTxt = "ap";

        char ip[16] = {0};
        (void)getNetworkIp_(ip, sizeof(ip), nullptr);

        char out[96] = {0};
        const int n = snprintf(out,
                               sizeof(out),
                               "{\"ok\":true,\"mode\":\"%s\",\"ip\":\"%s\"}",
                               modeTxt,
                               ip);
        if (n <= 0 || (size_t)n >= sizeof(out)) {
            request->send(500, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"Failed\",\"where\":\"network.mode\"}}");
            return;
        }
        request->send(200, "application/json", out);
    });
    server_.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->redirect("/webinterface");
    });
    server_.on("/gen_204", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->redirect("/webinterface");
    });
    server_.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->redirect("/webinterface");
    });
    server_.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->redirect("/webinterface");
    });
    server_.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->redirect("/webinterface");
    });

    auto fwStatusHandler = [this](AsyncWebServerRequest* request) {
        if (!fwUpdateSvc_ && services_) {
            fwUpdateSvc_ = services_->get<FirmwareUpdateService>("fwupdate");
        }
        if (!fwUpdateSvc_ || !fwUpdateSvc_->statusJson) {
            request->send(503, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"NotReady\",\"where\":\"fwupdate.status\"}}");
            return;
        }

        char out[320] = {0};
        if (!fwUpdateSvc_->statusJson(fwUpdateSvc_->ctx, out, sizeof(out))) {
            request->send(500, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"Failed\",\"where\":\"fwupdate.status\"}}");
            return;
        }
        request->send(200, "application/json", out);
    };
    server_.on("/fwupdate/status", HTTP_GET, fwStatusHandler);
    server_.on("/api/fwupdate/status", HTTP_GET, fwStatusHandler);

    server_.on("/api/fwupdate/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!fwUpdateSvc_ && services_) {
            fwUpdateSvc_ = services_->get<FirmwareUpdateService>("fwupdate");
        }
        if (!fwUpdateSvc_ || !fwUpdateSvc_->configJson) {
            request->send(503, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"NotReady\",\"where\":\"fwupdate.config\"}}");
            return;
        }

        char out[320] = {0};
        if (!fwUpdateSvc_->configJson(fwUpdateSvc_->ctx, out, sizeof(out))) {
            request->send(500, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"Failed\",\"where\":\"fwupdate.config\"}}");
            return;
        }
        request->send(200, "application/json", out);
    });

    server_.on("/api/fwupdate/config", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!fwUpdateSvc_ && services_) {
            fwUpdateSvc_ = services_->get<FirmwareUpdateService>("fwupdate");
        }
        if (!fwUpdateSvc_ || !fwUpdateSvc_->setConfig) {
            request->send(503, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"NotReady\",\"where\":\"fwupdate.set_config\"}}");
            return;
        }

        String hostStr;
        String flowStr;
        String nxStr;
        if (request->hasParam("update_host", true)) {
            hostStr = request->getParam("update_host", true)->value();
        }
        if (request->hasParam("flowio_path", true)) {
            flowStr = request->getParam("flowio_path", true)->value();
        }
        if (request->hasParam("nextion_path", true)) {
            nxStr = request->getParam("nextion_path", true)->value();
        }

        char err[96] = {0};
        if (!fwUpdateSvc_->setConfig(fwUpdateSvc_->ctx,
                                     hostStr.c_str(),
                                     flowStr.c_str(),
                                     nxStr.c_str(),
                                     err,
                                     sizeof(err))) {
            request->send(409,
                          "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"Failed\",\"where\":\"fwupdate.set_config\"}}");
            return;
        }

        request->send(200, "application/json", "{\"ok\":true}");
    });

    server_.on("/api/mqtt/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
        char host[sizeof(mqttCfg_.host)] = {0};
        char user[sizeof(mqttCfg_.user)] = {0};
        char pass[sizeof(mqttCfg_.pass)] = {0};
        snprintf(host, sizeof(host), "%s", mqttCfg_.host);
        snprintf(user, sizeof(user), "%s", mqttCfg_.user);
        snprintf(pass, sizeof(pass), "%s", mqttCfg_.pass);
        sanitizeJsonString_(host);
        sanitizeJsonString_(user);
        sanitizeJsonString_(pass);

        char out[512] = {0};
        const int n = snprintf(out,
                               sizeof(out),
                               "{\"ok\":true,\"server\":\"%s\",\"port\":%ld,\"username\":\"%s\",\"password\":\"%s\"}",
                               host,
                               (long)mqttCfg_.port,
                               user,
                               pass);
        if (n <= 0 || (size_t)n >= sizeof(out)) {
            request->send(500, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"Failed\",\"where\":\"mqtt.config.get\"}}");
            return;
        }
        request->send(200, "application/json", out);
    });

    server_.on("/api/mqtt/config", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!cfgStore_) {
            request->send(503, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"NotReady\",\"where\":\"mqtt.config.set\"}}");
            return;
        }

        String serverStr = request->hasParam("server", true)
                               ? request->getParam("server", true)->value()
                               : String(mqttCfg_.host);
        String userStr = request->hasParam("username", true)
                             ? request->getParam("username", true)->value()
                             : String(mqttCfg_.user);
        String passStr = request->hasParam("password", true)
                             ? request->getParam("password", true)->value()
                             : String(mqttCfg_.pass);

        int32_t portVal = mqttCfg_.port;
        if (request->hasParam("port", true)) {
            String portStr = request->getParam("port", true)->value();
            if (portStr.length() == 0) {
                portVal = Limits::Mqtt::Defaults::Port;
            } else {
                char* end = nullptr;
                const long parsed = strtol(portStr.c_str(), &end, 10);
                if (!end || *end != '\0' || parsed < 1 || parsed > 65535) {
                    request->send(400, "application/json",
                                  "{\"ok\":false,\"err\":{\"code\":\"InvalidArg\",\"where\":\"mqtt.port\"}}");
                    return;
                }
                portVal = (int32_t)parsed;
            }
        }

        bool ok = true;
        ok = ok && cfgStore_->set(mqttHostVar_, serverStr.c_str());
        ok = ok && cfgStore_->set(mqttPortVar_, portVal);
        ok = ok && cfgStore_->set(mqttUserVar_, userStr.c_str());
        ok = ok && cfgStore_->set(mqttPassVar_, passStr.c_str());
        if (!ok) {
            request->send(500, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"Failed\",\"where\":\"mqtt.config.set\"}}");
            return;
        }

        request->send(200, "application/json", "{\"ok\":true}");
    });

    server_.on("/api/wifi/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!cfgStore_) {
            request->send(503, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"NotReady\",\"where\":\"wifi.config.get\"}}");
            return;
        }

        char wifiJson[320] = {0};
        if (!cfgStore_->toJsonModule("wifi", wifiJson, sizeof(wifiJson), nullptr)) {
            request->send(500, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"Failed\",\"where\":\"wifi.config.get\"}}");
            return;
        }

        StaticJsonDocument<320> doc;
        const DeserializationError err = deserializeJson(doc, wifiJson);
        if (err || !doc.is<JsonObjectConst>()) {
            request->send(500, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"InvalidData\",\"where\":\"wifi.config.get\"}}");
            return;
        }

        JsonObjectConst root = doc.as<JsonObjectConst>();
        bool enabled = root["enabled"] | true;
        const char* ssid = root["ssid"] | "";
        const char* pass = root["pass"] | "";

        char ssidSafe[96] = {0};
        char passSafe[96] = {0};
        snprintf(ssidSafe, sizeof(ssidSafe), "%s", ssid ? ssid : "");
        snprintf(passSafe, sizeof(passSafe), "%s", pass ? pass : "");
        sanitizeJsonString_(ssidSafe);
        sanitizeJsonString_(passSafe);

        char out[360] = {0};
        const int n = snprintf(out,
                               sizeof(out),
                               "{\"ok\":true,\"enabled\":%s,\"ssid\":\"%s\",\"pass\":\"%s\"}",
                               enabled ? "true" : "false",
                               ssidSafe,
                               passSafe);
        if (n <= 0 || (size_t)n >= sizeof(out)) {
            request->send(500, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"Failed\",\"where\":\"wifi.config.get\"}}");
            return;
        }
        request->send(200, "application/json", out);
    });

    server_.on("/api/wifi/config", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!cfgStore_) {
            request->send(503, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"NotReady\",\"where\":\"wifi.config.set\"}}");
            return;
        }

        const String enabledStr = request->hasParam("enabled", true)
                                      ? request->getParam("enabled", true)->value()
                                      : String("1");
        const bool enabled = parseBoolParam_(enabledStr, true);
        const String ssid = request->hasParam("ssid", true)
                                ? request->getParam("ssid", true)->value()
                                : String();
        const String pass = request->hasParam("pass", true)
                                ? request->getParam("pass", true)->value()
                                : String();

        StaticJsonDocument<320> patch;
        JsonObject root = patch.to<JsonObject>();
        JsonObject wifi = root.createNestedObject("wifi");
        wifi["enabled"] = enabled;
        wifi["ssid"] = ssid.c_str();
        wifi["pass"] = pass.c_str();

        char patchJson[320] = {0};
        if (serializeJson(patch, patchJson, sizeof(patchJson)) == 0) {
            request->send(500, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"Failed\",\"where\":\"wifi.config.set\"}}");
            return;
        }

        if (!cfgStore_->applyJson(patchJson)) {
            request->send(500, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"Failed\",\"where\":\"wifi.config.set\"}}");
            return;
        }

        if (!netAccessSvc_ && services_) {
            netAccessSvc_ = services_->get<NetworkAccessService>("network_access");
        }
        if (netAccessSvc_ && netAccessSvc_->notifyWifiConfigChanged) {
            netAccessSvc_->notifyWifiConfigChanged(netAccessSvc_->ctx);
        }

        request->send(200, "application/json", "{\"ok\":true}");
    });

    server_.on("/api/wifi/scan", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!wifiSvc_ && services_) {
            wifiSvc_ = services_->get<WifiService>("wifi");
        }
        if (!wifiSvc_ || !wifiSvc_->scanStatusJson) {
            request->send(503, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"NotReady\",\"where\":\"wifi.scan.get\"}}");
            return;
        }

        char out[3072] = {0};
        if (!wifiSvc_->scanStatusJson(wifiSvc_->ctx, out, sizeof(out))) {
            request->send(500, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"Failed\",\"where\":\"wifi.scan.get\"}}");
            return;
        }
        request->send(200, "application/json", out);
    });

    server_.on("/api/wifi/scan", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!wifiSvc_ && services_) {
            wifiSvc_ = services_->get<WifiService>("wifi");
        }
        if (!wifiSvc_ || !wifiSvc_->requestScan) {
            request->send(503, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"NotReady\",\"where\":\"wifi.scan.start\"}}");
            return;
        }

        String forceStr = request->hasParam("force", true)
                              ? request->getParam("force", true)->value()
                              : String("1");
        const bool force = parseBoolParam_(forceStr, true);
        if (!wifiSvc_->requestScan(wifiSvc_->ctx, force)) {
            request->send(500, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"Failed\",\"where\":\"wifi.scan.start\"}}");
            return;
        }

        if (wifiSvc_->scanStatusJson) {
            char out[3072] = {0};
            if (wifiSvc_->scanStatusJson(wifiSvc_->ctx, out, sizeof(out))) {
                request->send(202, "application/json", out);
                return;
            }
        }

        request->send(202, "application/json", "{\"ok\":true,\"accepted\":true}");
    });

    server_.on("/api/system/reboot", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!cmdSvc_ && services_) {
            cmdSvc_ = services_->get<CommandService>("cmd");
        }
        if (!cmdSvc_ || !cmdSvc_->execute) {
            request->send(503, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"NotReady\",\"where\":\"system.reboot\"}}");
            return;
        }

        char reply[196] = {0};
        const bool ok = cmdSvc_->execute(cmdSvc_->ctx, "system.reboot", "{}", nullptr, reply, sizeof(reply));
        if (!ok) {
            const String body = (reply[0] != '\0')
                                    ? String(reply)
                                    : String("{\"ok\":false,\"err\":{\"code\":\"Failed\",\"where\":\"system.reboot\"}}");
            request->send(500, "application/json", body);
            return;
        }
        const String body = (reply[0] != '\0') ? String(reply) : String("{\"ok\":true}");
        request->send(200, "application/json", body);
    });

    server_.on("/api/system/factory-reset", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!cmdSvc_ && services_) {
            cmdSvc_ = services_->get<CommandService>("cmd");
        }
        if (!cmdSvc_ || !cmdSvc_->execute) {
            request->send(503, "application/json",
                          "{\"ok\":false,\"err\":{\"code\":\"NotReady\",\"where\":\"system.factory_reset\"}}");
            return;
        }

        char reply[220] = {0};
        const bool ok = cmdSvc_->execute(cmdSvc_->ctx, "system.factory_reset", "{}", nullptr, reply, sizeof(reply));
        if (!ok) {
            const String body =
                (reply[0] != '\0')
                    ? String(reply)
                    : String("{\"ok\":false,\"err\":{\"code\":\"Failed\",\"where\":\"system.factory_reset\"}}");
            request->send(500, "application/json", body);
            return;
        }
        const String body = (reply[0] != '\0') ? String(reply) : String("{\"ok\":true}");
        request->send(200, "application/json", body);
    });

    server_.on("/fwupdate/flowio", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleUpdateRequest_(request, FirmwareUpdateTarget::FlowIO);
    });

    server_.on("/fwupdate/nextion", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleUpdateRequest_(request, FirmwareUpdateTarget::Nextion);
    });

    server_.onNotFound([](AsyncWebServerRequest* request) {
        request->redirect("/webinterface");
    });

    ws_.onEvent([this](AsyncWebSocket* server,
                       AsyncWebSocketClient* client,
                       AwsEventType type,
                       void* arg,
                       uint8_t* data,
                       size_t len) {
        this->onWsEvent_(server, client, type, arg, data, len);
    });

    server_.addHandler(&ws_);
    server_.begin();
    started_ = true;
    LOGI("WebInterface server started, listening on 0.0.0.0:%d", kServerPort);

    char ip[16] = {0};
    NetworkAccessMode mode = NetworkAccessMode::None;
    if (getNetworkIp_(ip, sizeof(ip), &mode) && ip[0] != '\0') {
        if (mode == NetworkAccessMode::AccessPoint) {
            LOGI("WebInterface URL (AP): http://%s/webinterface", ip);
        } else {
            LOGI("WebInterface URL: http://%s/webinterface", ip);
        }
    } else {
        LOGI("WebInterface URL: waiting for network IP");
    }
}

void WebInterfaceModule::onWsEvent_(AsyncWebSocket*,
                                 AsyncWebSocketClient* client,
                                 AwsEventType type,
                                 void* arg,
                                 uint8_t* data,
                                 size_t len)
{
    if (type == WS_EVT_CONNECT) {
        if (client) client->text("[webinterface] connecté");
        return;
    }

    if (type != WS_EVT_DATA || !arg || !data || len == 0) return;

    AwsFrameInfo* info = reinterpret_cast<AwsFrameInfo*>(arg);
    if (!info->final || info->index != 0 || info->len != len || info->opcode != WS_TEXT) return;

    constexpr size_t kMaxIncoming = 192;
    char msg[kMaxIncoming] = {0};
    size_t n = (len < (kMaxIncoming - 1)) ? len : (kMaxIncoming - 1);
    memcpy(msg, data, n);
    msg[n] = '\0';

    if (uartPaused_) {
        if (client) client->text("[webinterface] uart occupé (mise à jour firmware en cours)");
        return;
    }

    uart_.write(reinterpret_cast<const uint8_t*>(msg), n);
    uart_.write('\n');
}

void WebInterfaceModule::handleUpdateRequest_(AsyncWebServerRequest* request, FirmwareUpdateTarget target)
{
    if (!request) return;
    if (!fwUpdateSvc_ && services_) {
        fwUpdateSvc_ = services_->get<FirmwareUpdateService>("fwupdate");
    }
    if (!fwUpdateSvc_ || !fwUpdateSvc_->start) {
        request->send(503, "application/json",
                      "{\"ok\":false,\"err\":{\"code\":\"NotReady\",\"where\":\"fwupdate.start\"}}");
        return;
    }

    const AsyncWebParameter* pUrl = request->hasParam("url", true) ? request->getParam("url", true) : nullptr;
    String urlStr;
    if (pUrl) {
        urlStr = pUrl->value();
    }
    const char* url = (urlStr.length() > 0) ? urlStr.c_str() : nullptr;

    char err[144] = {0};
    if (!fwUpdateSvc_->start(fwUpdateSvc_->ctx, target, url, err, sizeof(err))) {
        request->send(409,
                      "application/json",
                      "{\"ok\":false,\"err\":{\"code\":\"Failed\",\"where\":\"fwupdate.start\"}}");
        return;
    }

    request->send(202, "application/json", "{\"ok\":true,\"accepted\":true}");
}

bool WebInterfaceModule::isWebReachable_() const
{
    if (netAccessSvc_ && netAccessSvc_->isWebReachable) {
        return netAccessSvc_->isWebReachable(netAccessSvc_->ctx);
    }
    if (wifiSvc_ && wifiSvc_->isConnected) {
        return wifiSvc_->isConnected(wifiSvc_->ctx);
    }
    return netReady_;
}

bool WebInterfaceModule::getNetworkIp_(char* out, size_t len, NetworkAccessMode* modeOut) const
{
    if (out && len > 0) out[0] = '\0';
    if (modeOut) *modeOut = NetworkAccessMode::None;
    if (!out || len == 0) return false;

    if (netAccessSvc_ && netAccessSvc_->getIP) {
        if (netAccessSvc_->getIP(netAccessSvc_->ctx, out, len)) {
            if (modeOut && netAccessSvc_->mode) {
                *modeOut = netAccessSvc_->mode(netAccessSvc_->ctx);
            }
            return out[0] != '\0';
        }
    }

    if (wifiSvc_ && wifiSvc_->getIP) {
        if (wifiSvc_->getIP(wifiSvc_->ctx, out, len)) {
            if (modeOut) *modeOut = NetworkAccessMode::Station;
            return out[0] != '\0';
        }
    }

    return false;
}

bool WebInterfaceModule::isLogByte_(uint8_t c)
{
    return c == '\t' || c == 0x1B || c >= 32;
}

bool WebInterfaceModule::svcSetPaused_(void* ctx, bool paused)
{
    WebInterfaceModule* self = static_cast<WebInterfaceModule*>(ctx);
    if (!self) return false;
    self->uartPaused_ = paused;
    if (paused) {
        self->lineLen_ = 0;
    }
    return true;
}

bool WebInterfaceModule::svcIsPaused_(void* ctx)
{
    WebInterfaceModule* self = static_cast<WebInterfaceModule*>(ctx);
    if (!self) return false;
    return self->uartPaused_;
}

void WebInterfaceModule::onEventStatic_(const Event& e, void* user)
{
    WebInterfaceModule* self = static_cast<WebInterfaceModule*>(user);
    if (!self) return;
    self->onEvent_(e);
}

void WebInterfaceModule::onEvent_(const Event& e)
{
    if (e.id != EventId::DataChanged) return;
    if (!e.payload || e.len < sizeof(DataChangedPayload)) return;
    const DataChangedPayload* p = static_cast<const DataChangedPayload*>(e.payload);
    if (p->id != DataKeys::WifiReady) return;

    netReady_ = dataStore_ ? wifiReady(*dataStore_) : false;
}

void WebInterfaceModule::flushLine_(bool force)
{
    if (lineLen_ == 0) return;
    if (!force) return;

    lineBuf_[lineLen_] = '\0';
    ws_.textAll(lineBuf_);
    lineLen_ = 0;
}

void WebInterfaceModule::loop()
{
    if (!netAccessSvc_ && services_) {
        netAccessSvc_ = services_->get<NetworkAccessService>("network_access");
    }

    if (!started_) {
        if (!isWebReachable_()) {
            vTaskDelay(pdMS_TO_TICKS(100));
            return;
        }
        startServer_();
    }

    if (uartPaused_) {
        if (started_) ws_.cleanupClients();
        vTaskDelay(pdMS_TO_TICKS(40));
        return;
    }

    while (uart_.available() > 0) {
        int raw = uart_.read();
        if (raw < 0) break;

        const uint8_t c = static_cast<uint8_t>(raw);

        if (c == '\r') continue;
        if (c == '\n') {
            flushLine_(true);
            continue;
        }

        if (lineLen_ >= (kLineBufferSize - 1)) {
            flushLine_(true);
        }

        if (lineLen_ < (kLineBufferSize - 1)) {
            lineBuf_[lineLen_++] = isLogByte_(c) ? static_cast<char>(c) : '.';
        }
    }

    if (started_) ws_.cleanupClients();

    vTaskDelay(pdMS_TO_TICKS(10));
}
