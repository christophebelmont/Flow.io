/**
 * @file WebInterfaceModule.cpp
 * @brief Web interface bridge for Supervisor profile.
 */

#include "WebInterfaceModule.h"

#define LOG_TAG "WebIntf"
#include "Core/ModuleLog.h"

#include <string.h>
#include "Core/DataKeys.h"
#include "Core/EventBus/EventPayloads.h"
#include "Modules/Network/WifiModule/WifiRuntime.h"

static const char kWebInterfacePage[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Flow.IO Supervisor Interface</title>
  <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Material+Symbols+Outlined:opsz,wght,FILL,GRAD@24,400,0,0" />
  <style>
    :root {
      --md-bg: #fef7ff;
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
      background:
        radial-gradient(1200px 800px at -10% -20%, #ede4ff 0%, transparent 45%),
        radial-gradient(900px 600px at 120% -10%, #ffe7ef 0%, transparent 45%),
        var(--md-bg);
      color: var(--md-on-surface);
    }
    .app { display: flex; min-height: 100vh; }
    .drawer {
      width: 280px;
      padding: 12px;
      border-right: 1px solid rgba(121,116,126,0.35);
      background: rgba(255,251,254,0.95);
      overflow: hidden;
      transition: width 0.2s ease;
      z-index: 20;
    }
    .drawer.collapsed { width: 78px; }
    .drawer-header { display: flex; align-items: center; gap: 10px; margin-bottom: 8px; }
    .menu-btn {
      width: 42px; height: 42px; border: 0; border-radius: 12px;
      background: var(--md-secondary-container); color: var(--md-on-secondary-container);
      cursor: pointer; font-size: 18px; line-height: 1;
    }
    .brand { font-weight: 700; white-space: nowrap; }
    .drawer.collapsed .brand { display: none; }
    .menu-group { display: grid; gap: 6px; margin-top: 10px; }
    .menu-item {
      border: 0; border-radius: 14px; padding: 12px 14px;
      display: flex; align-items: center; gap: 12px; cursor: pointer;
      background: transparent; color: var(--md-on-surface); text-align: left;
      font-size: 14px; font-weight: 500; white-space: nowrap;
    }
    .menu-item.active { background: var(--md-secondary-container); color: var(--md-on-secondary-container); font-weight: 600; }
    .menu-item .ico {
      width: 20px;
      text-align: center;
      display: inline-flex;
      justify-content: center;
      align-items: center;
    }
    .material-symbols-outlined {
      font-size: 20px;
      line-height: 1;
      font-variation-settings: 'FILL' 0, 'wght' 400, 'GRAD' 0, 'opsz' 24;
    }
    .drawer.collapsed .menu-item .label { display: none; }
    .content { flex: 1; padding: 18px; min-width: 0; }
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
    .form-grid { display: grid; gap: 12px; grid-template-columns: repeat(2, minmax(220px, 1fr)); }
    .field { display: grid; gap: 6px; min-width: 0; }
    .field.full { grid-column: 1 / -1; }
    .field label { font-size: 12px; font-weight: 600; opacity: 0.78; }
    input, button {
      border-radius: 12px; border: 1px solid rgba(121,116,126,0.45);
      background: white; color: var(--md-on-surface); font: inherit; padding: 10px 12px;
    }
    input:focus { outline: 2px solid rgba(103,80,164,0.4); border-color: var(--md-primary); }
    .btn-row { display: flex; flex-wrap: wrap; gap: 8px; margin-top: 12px; }
    button { cursor: pointer; }
    .btn-primary { background: var(--md-primary); color: var(--md-on-primary); border-color: var(--md-primary); font-weight: 600; }
    .btn-tonal { background: var(--md-secondary-container); color: var(--md-on-secondary-container); border-color: transparent; font-weight: 600; }
    .upgrade-status {
      margin-top: 10px; padding: 10px; border-radius: 12px;
      background: rgba(232,222,248,0.7); font-size: 13px;
    }
    .overlay { position: fixed; inset: 0; background: rgba(24,20,38,0.25); display: none; z-index: 10; }
    @media (max-width: 900px) {
      .drawer {
        position: fixed; left: 0; top: 0; bottom: 0;
        transform: translateX(-100%); transition: transform 0.2s ease;
      }
      .drawer.mobile-open { transform: translateX(0); }
      .drawer.collapsed { width: 280px; }
      .drawer.collapsed .brand, .drawer.collapsed .menu-item .label { display: inline; }
      .overlay.visible { display: block; }
      .content { width: 100%; padding: 12px; }
      .form-grid { grid-template-columns: 1fr; }
    }
  </style>
</head>
<body>
  <div id="overlay" class="overlay"></div>
  <div class="app">
    <aside id="drawer" class="drawer">
      <div class="drawer-header">
        <button id="menuToggle" class="menu-btn" aria-label="Toggle menu">=</button>
        <div class="brand">Flow.IO Supervisor</div>
      </div>
      <nav class="menu-group">
        <button class="menu-item active" data-page="page-terminal"><span class="ico material-symbols-outlined">terminal</span><span class="label">Journaux</span></button>
        <button class="menu-item" data-page="page-upgrade"><span class="ico material-symbols-outlined">update</span><span class="label">Mise à jour Firmware</span></button>
      </nav>
    </aside>

    <main class="content">
      <section id="page-terminal" class="page active">
        <div class="topbar">
          <h1>Journaux</h1>
          <span id="wsStatus" class="status-chip">connecting...</span>
        </div>
        <div class="card">
          <div id="term" class="terminal"></div>
          <div class="term-toolbar">
            <input id="line" placeholder="Send line to UART" />
            <button id="send" class="btn-tonal">Send</button>
            <button id="clear">Clear</button>
          </div>
        </div>
      </section>

      <section id="page-upgrade" class="page">
        <div class="topbar">
          <h1>Mise à jour Firmware</h1>
          <span id="upStatusChip" class="status-chip">idle</span>
        </div>
        <div class="card">
          <div class="form-grid">
            <div class="field full">
              <label for="updateHost">HTTP Server (hostname or IP, optional protocol)</label>
              <input id="updateHost" placeholder="e.g. 192.168.1.20 or http://192.168.1.20" />
            </div>
            <div class="field">
              <label for="flowPath">Flow.IO image path</label>
              <input id="flowPath" placeholder="/build/FlowIO.bin" />
            </div>
            <div class="field">
              <label for="nextionPath">Nextion image path</label>
              <input id="nextionPath" placeholder="/build/Nextion.tft" />
            </div>
          </div>
          <div class="btn-row">
            <button id="saveCfg" class="btn-tonal">Save Config</button>
            <button id="upFlow" class="btn-primary">Upgrade Flow.IO</button>
            <button id="upNextion" class="btn-primary">Upgrade Nextion</button>
            <button id="refreshState">Refresh Status</button>
          </div>
          <div id="upgradeStatus" class="upgrade-status">No operation running.</div>
        </div>
      </section>
    </main>
  </div>

  <script>
    const drawer = document.getElementById('drawer');
    const overlay = document.getElementById('overlay');
    const menuToggle = document.getElementById('menuToggle');
    const menuItems = Array.from(document.querySelectorAll('.menu-item'));
    const pages = Array.from(document.querySelectorAll('.page'));

    function closeMobileDrawer() {
      if (window.innerWidth <= 900) {
        drawer.classList.remove('mobile-open');
        overlay.classList.remove('visible');
      }
    }

    function showPage(pageId) {
      pages.forEach((el) => el.classList.toggle('active', el.id === pageId));
      menuItems.forEach((el) => el.classList.toggle('active', el.dataset.page === pageId));
      closeMobileDrawer();
    }

    menuItems.forEach((item) => item.addEventListener('click', () => showPage(item.dataset.page)));

    menuToggle.addEventListener('click', () => {
      if (window.innerWidth <= 900) {
        drawer.classList.toggle('mobile-open');
        overlay.classList.toggle('visible');
      } else {
        drawer.classList.toggle('collapsed');
      }
    });

    overlay.addEventListener('click', closeMobileDrawer);
    window.addEventListener('resize', () => {
      if (window.innerWidth > 900) {
        drawer.classList.remove('mobile-open');
        overlay.classList.remove('visible');
      }
    });

    const term = document.getElementById('term');
    const wsStatus = document.getElementById('wsStatus');
    const line = document.getElementById('line');
    const sendBtn = document.getElementById('send');
    const clearBtn = document.getElementById('clear');

    const updateHost = document.getElementById('updateHost');
    const flowPath = document.getElementById('flowPath');
    const nextionPath = document.getElementById('nextionPath');
    const saveCfgBtn = document.getElementById('saveCfg');
    const upFlowBtn = document.getElementById('upFlow');
    const upNextionBtn = document.getElementById('upNextion');
    const refreshStateBtn = document.getElementById('refreshState');
    const upgradeStatus = document.getElementById('upgradeStatus');
    const upStatusChip = document.getElementById('upStatusChip');

    const wsProto = location.protocol === 'https:' ? 'wss' : 'ws';
    const ws = new WebSocket(wsProto + '://' + location.host + '/wsserial');

    ws.onopen = () => wsStatus.textContent = 'connected';
    ws.onclose = () => wsStatus.textContent = 'disconnected';
    ws.onerror = () => wsStatus.textContent = 'error';

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
      term.scrollTop = term.scrollHeight;
    };

    function sendLine() {
      const txt = line.value;
      if (!txt) return;
      if (ws.readyState === WebSocket.OPEN) ws.send(txt);
      line.value = '';
      line.focus();
    }
    sendBtn.addEventListener('click', sendLine);
    line.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') sendLine();
    });
    clearBtn.addEventListener('click', () => { term.textContent = ''; });

    function updateUpgradeView(data) {
      if (!data || data.ok !== true) return;
      const state = data.state || 'unknown';
      const target = data.target || '-';
      const progress = Number.isFinite(data.progress) ? data.progress : 0;
      const msg = data.msg || '';
      upStatusChip.textContent = state;
      upgradeStatus.textContent = state + ' | target=' + target + ' | progress=' + progress + '% | ' + msg;
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
        upgradeStatus.textContent = 'Config load failed: ' + err;
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
      if (!res.ok || !data.ok) throw new Error('save failed');
      upgradeStatus.textContent = 'Configuration saved.';
    }

    async function refreshUpgradeStatus() {
      try {
        const res = await fetch('/api/fwupdate/status', { cache: 'no-store' });
        const data = await res.json();
        if (data && data.ok) updateUpgradeView(data);
      } catch (err) {
        upgradeStatus.textContent = 'Status read failed: ' + err;
      }
    }

    async function startUpgrade(target) {
      try {
        await saveUpgradeConfig();
        const endpoint = target === 'flowio' ? '/fwupdate/flowio' : '/fwupdate/nextion';
        const res = await fetch(endpoint, { method: 'POST' });
        const data = await res.json().catch(() => ({}));
        if (!res.ok || !data.ok) throw new Error('start failed');
        upgradeStatus.textContent = 'Upgrade request accepted for ' + target + '.';
        await refreshUpgradeStatus();
      } catch (err) {
        upgradeStatus.textContent = 'Upgrade failed: ' + err;
      }
    }

    saveCfgBtn.addEventListener('click', async () => {
      try {
        await saveUpgradeConfig();
      } catch (err) {
        upgradeStatus.textContent = 'Save failed: ' + err;
      }
    });
    upFlowBtn.addEventListener('click', () => startUpgrade('flowio'));
    upNextionBtn.addEventListener('click', () => startUpgrade('nextion'));
    refreshStateBtn.addEventListener('click', refreshUpgradeStatus);

    loadUpgradeConfig();
    refreshUpgradeStatus();
    setInterval(refreshUpgradeStatus, 2000);
  </script>
</body>
</html>
)HTML";

void WebInterfaceModule::init(ConfigStore&, ServiceRegistry& services)
{
    services_ = &services;
    logHub_ = services.get<LogHubService>("loghub");
    wifiSvc_ = services.get<WifiService>("wifi");
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

    server_.on("/fwupdate/flowio", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleUpdateRequest_(request, FirmwareUpdateTarget::FlowIO);
    });

    server_.on("/fwupdate/nextion", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleUpdateRequest_(request, FirmwareUpdateTarget::Nextion);
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

    if (wifiSvc_ && wifiSvc_->isConnected && wifiSvc_->isConnected(wifiSvc_->ctx)) {
        char ip[16] = {0};
        if (wifiSvc_->getIP && wifiSvc_->getIP(wifiSvc_->ctx, ip, sizeof(ip)) && ip[0] != '\0') {
            LOGI("WebInterface ready: http://%s/webinterface", ip);
        } else {
            LOGI("WebInterface ready: WiFi connected (IP unavailable)");
        }
    } else {
        LOGI("WebInterface ready: waiting for WiFi IP");
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
        if (client) client->text("[webinterface] connected");
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
        if (client) client->text("[webinterface] uart busy (firmware update in progress)");
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
    if (!started_) {
        if (!netReady_) {
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
