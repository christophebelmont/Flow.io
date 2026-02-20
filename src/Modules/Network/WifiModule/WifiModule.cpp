/**
 * @file WifiModule.cpp
 * @brief Implementation file.
 */
#include "WifiModule.h"
#define LOG_TAG "WifiModu"
#include "Core/ModuleLog.h"
#include "Core/Runtime.h"
#include <ArduinoJson.h>
#include <esp_wifi.h>
#include <ctype.h>
#include <string.h>

WifiState WifiModule::svcState(void* ctx) {
    WifiModule* self = (WifiModule*)ctx;
    return self->state;
}

bool WifiModule::svcIsConnected(void* ctx) {
    (void)ctx;
    return WiFi.isConnected();
}

bool WifiModule::svcGetIP(void* ctx, char* out, size_t len) {
    (void)ctx;
    if (!out || len == 0) return false;

    if (!WiFi.isConnected()) {
        out[0] = '\0';
        return false;
    }

    IPAddress ip = WiFi.localIP();
    snprintf(out, len, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    return true;
}

bool WifiModule::svcRequestReconnect(void* ctx)
{
    WifiModule* self = static_cast<WifiModule*>(ctx);
    if (!self) return false;

    self->stopMdns_();
    self->gotIpSent = false;
    if (self->dataStore) {
        setWifiReady(*self->dataStore, false);
    }

    WiFi.disconnect(false, false);
    self->setState(WifiState::Idle);
    return true;
}

bool WifiModule::svcRequestScan(void* ctx, bool force)
{
    WifiModule* self = static_cast<WifiModule*>(ctx);
    if (!self) return false;
    return self->requestScan_(force);
}

bool WifiModule::svcScanStatusJson(void* ctx, char* out, size_t outLen)
{
    WifiModule* self = static_cast<WifiModule*>(ctx);
    if (!self) return false;
    return self->buildScanStatusJson_(out, outLen);
}

void WifiModule::setState(WifiState s) {
    if (s == state) return;
    state = s;
    stateTs = millis();

    if (state == WifiState::Idle || state == WifiState::ErrorWait || state == WifiState::Disabled) {
        stopMdns_();
        if (dataStore) {
            setWifiReady(*dataStore, false);
        }
        gotIpSent = false;
    }
}

void WifiModule::startConnect() {
    if (cfgData.ssid[0] == '\0') {
        const uint32_t now = millis();
        if ((now - lastEmptySsidLogMs) >= 10000U) {
            lastEmptySsidLogMs = now;
            LOGW("SSID empty, skipping connection");
        }
        setState(WifiState::Idle);
        return;
    }

    LOGI("Connecting to '%s'", cfgData.ssid);

    WiFi.disconnect(false, false);
    delay(50);

    const wifi_mode_t modeNow = WiFi.getMode();
    const bool keepAp = (modeNow == WIFI_MODE_AP || modeNow == WIFI_MODE_APSTA);
    WiFi.mode(keepAp ? WIFI_MODE_APSTA : WIFI_MODE_STA);
    WiFi.setSleep(false);               ///< ✅ important (stability)
    WiFi.begin(cfgData.ssid, cfgData.pass);

    setState(WifiState::Connecting);
}

bool WifiModule::requestScan_(bool force)
{
    const uint32_t now = millis();
    if (!force && scanLastDoneMs_ != 0U && (now - scanLastDoneMs_) < kScanThrottleMs) {
        return true;
    }
    portENTER_CRITICAL(&scanMux_);
    scanRequested_ = true;
    scanApRetryCount_ = 0;
    portEXIT_CRITICAL(&scanMux_);
    return true;
}

void WifiModule::processScan_()
{
    if (scanRunning_) {
        const int16_t status = WiFi.scanComplete();
        if (status == WIFI_SCAN_RUNNING) return;

        if (status < 0) {
            portENTER_CRITICAL(&scanMux_);
            scanRunning_ = false;
            scanLastError_ = status;
            scanLastDoneMs_ = millis();
            portEXIT_CRITICAL(&scanMux_);
            WiFi.scanDelete();
            LOGW("WiFi scan failed status=%d", (int)status);
            return;
        }

        WifiScanEntry local[kScanMaxResults] = {};
        uint8_t localCount = 0;
        const int16_t total = status;

        for (int16_t i = 0; i < total; ++i) {
            String ssid = WiFi.SSID(i);
            const bool hidden = (ssid.length() == 0);
            char ssidBuf[33] = {0};
            if (hidden) {
                snprintf(ssidBuf, sizeof(ssidBuf), "<hidden>");
            } else {
                snprintf(ssidBuf, sizeof(ssidBuf), "%s", ssid.c_str());
            }

            const int32_t rssi = WiFi.RSSI(i);
            const uint8_t auth = (uint8_t)WiFi.encryptionType(i);

            int8_t found = -1;
            for (uint8_t j = 0; j < localCount; ++j) {
                if (strcmp(local[j].ssid, ssidBuf) == 0) {
                    found = (int8_t)j;
                    break;
                }
            }

            if (found >= 0) {
                if (rssi > local[(uint8_t)found].rssi) {
                    local[(uint8_t)found].rssi = (int16_t)rssi;
                    local[(uint8_t)found].auth = auth;
                    local[(uint8_t)found].hidden = hidden;
                }
                continue;
            }

            if (localCount >= kScanMaxResults) continue;

            snprintf(local[localCount].ssid, sizeof(local[localCount].ssid), "%s", ssidBuf);
            local[localCount].rssi = (int16_t)rssi;
            local[localCount].auth = auth;
            local[localCount].hidden = hidden;
            ++localCount;
        }

        for (uint8_t i = 0; i < localCount; ++i) {
            for (uint8_t j = (uint8_t)(i + 1U); j < localCount; ++j) {
                if (local[j].rssi > local[i].rssi) {
                    WifiScanEntry tmp = local[i];
                    local[i] = local[j];
                    local[j] = tmp;
                }
            }
        }

        const wifi_mode_t modeAfterScan = WiFi.getMode();
        if (total == 0 && modeAfterScan == WIFI_MODE_APSTA && scanApRetryCount_ == 0U) {
            // In AP+STA mode, a first async scan can sporadically return 0.
            // Retry once with longer channel dwell before concluding "no network".
            ++scanApRetryCount_;
            WiFi.scanDelete();
            const int16_t retryStatus = WiFi.scanNetworks(true, false, false, 500);
            if (retryStatus != WIFI_SCAN_FAILED) {
                portENTER_CRITICAL(&scanMux_);
                scanRunning_ = true;
                scanLastStartMs_ = millis();
                scanLastError_ = 0;
                portEXIT_CRITICAL(&scanMux_);
                LOGW("WiFi scan AP retry started");
                return;
            }
            LOGW("WiFi scan AP retry start failed");
        }

        portENTER_CRITICAL(&scanMux_);
        scanCount_ = localCount;
        scanTotalFound_ = (uint8_t)((total > 255) ? 255 : total);
        for (uint8_t i = 0; i < localCount; ++i) {
            scanEntries_[i] = local[i];
        }
        scanHasResults_ = true;
        scanRunning_ = false;
        scanLastError_ = 0;
        scanLastDoneMs_ = millis();
        ++scanGeneration_;
        portEXIT_CRITICAL(&scanMux_);

        WiFi.scanDelete();
        LOGI("WiFi scan done total=%d kept=%u", (int)total, (unsigned)localCount);
        return;
    }

    if (!scanRequested_) return;

    const wifi_mode_t modeNow = WiFi.getMode();
    if (modeNow == WIFI_MODE_NULL) {
        WiFi.mode(WIFI_MODE_STA);
    } else if (modeNow == WIFI_MODE_AP) {
        WiFi.mode(WIFI_MODE_APSTA);
    }

    scanRequested_ = false;
    const int16_t startStatus = WiFi.scanNetworks(true, false, false, 360);
    if (startStatus == WIFI_SCAN_FAILED) {
        portENTER_CRITICAL(&scanMux_);
        scanRunning_ = false;
        scanLastError_ = WIFI_SCAN_FAILED;
        scanLastDoneMs_ = millis();
        portEXIT_CRITICAL(&scanMux_);
        LOGW("WiFi scan start failed");
        return;
    }

    portENTER_CRITICAL(&scanMux_);
    scanRunning_ = true;
    scanLastStartMs_ = millis();
    scanLastError_ = 0;
    portEXIT_CRITICAL(&scanMux_);
}

bool WifiModule::buildScanStatusJson_(char* out, size_t outLen)
{
    if (!out || outLen == 0) return false;

    WifiScanEntry local[kScanMaxResults] = {};
    bool running = false;
    bool requested = false;
    bool hasResults = false;
    int16_t lastErr = 0;
    uint8_t count = 0;
    uint8_t total = 0;
    uint32_t startedMs = 0;
    uint32_t doneMs = 0;
    uint16_t gen = 0;

    portENTER_CRITICAL(&scanMux_);
    running = scanRunning_;
    requested = scanRequested_;
    hasResults = scanHasResults_;
    lastErr = scanLastError_;
    count = scanCount_;
    total = scanTotalFound_;
    startedMs = scanLastStartMs_;
    doneMs = scanLastDoneMs_;
    gen = scanGeneration_;
    for (uint8_t i = 0; i < count; ++i) {
        local[i] = scanEntries_[i];
    }
    portEXIT_CRITICAL(&scanMux_);

    StaticJsonDocument<3072> doc;
    doc["ok"] = true;
    doc["running"] = running;
    doc["requested"] = requested;
    doc["has_results"] = hasResults;
    doc["count"] = count;
    doc["total_found"] = total;
    doc["generation"] = gen;
    doc["last_error"] = lastErr;
    doc["started_ms"] = startedMs;
    doc["updated_ms"] = doneMs;

    JsonArray nets = doc.createNestedArray("networks");
    for (uint8_t i = 0; i < count; ++i) {
        JsonObject n = nets.createNestedObject();
        n["ssid"] = local[i].ssid;
        n["rssi"] = local[i].rssi;
        n["auth"] = local[i].auth;
        n["secure"] = (local[i].auth != WIFI_AUTH_OPEN);
        n["hidden"] = local[i].hidden;
    }

    const size_t wrote = serializeJson(doc, out, outLen);
    return wrote > 0 && wrote < outLen;
}

void WifiModule::init(ConfigStore& cfg,
                      ServiceRegistry& services)
{
    constexpr uint8_t kCfgModuleId = (uint8_t)ConfigModuleId::Wifi;
    constexpr uint16_t kCfgBranchId = (uint16_t)ConfigBranchId::Wifi;
    /// récupérer service loghub (log async)
    logHub = services.get<LogHubService>("loghub");

    const DataStoreService* dsSvc = services.get<DataStoreService>("datastore");
    dataStore = dsSvc ? dsSvc->store : nullptr;

    /// Register config vars
    cfg.registerVar(enabledVar, kCfgModuleId, kCfgBranchId);
    cfg.registerVar(ssidVar, kCfgModuleId, kCfgBranchId);
    cfg.registerVar(passVar, kCfgModuleId, kCfgBranchId);
    cfg.registerVar(mdnsVar, kCfgModuleId, kCfgBranchId);

    /// Register WifiService
    static WifiService svc {
        WifiModule::svcState,
        WifiModule::svcIsConnected,
        WifiModule::svcGetIP,
        this,
        WifiModule::svcRequestReconnect,
        WifiModule::svcRequestScan,
        WifiModule::svcScanStatusJson
    };
    services.add("wifi", &svc);

    // Keep WiFi credentials managed by ConfigStore only (no duplicate driver persistence).
    WiFi.persistent(false);

    LOGI("WifiService registered");

    if (!cfgData.enabled) {
        setState(WifiState::Disabled);
        return;
    }

    setState(WifiState::Idle);
}

void WifiModule::stopMdns_()
{
    if (!mdnsStarted) return;
    MDNS.end();
    mdnsStarted = false;
    mdnsApplied[0] = '\0';
    LOGI("mDNS stopped");
}

void WifiModule::syncMdns_()
{
    if (!WiFi.isConnected()) {
        stopMdns_();
        return;
    }

    char host[sizeof(cfgData.mdns)] = {0};
    size_t w = 0;
    for (size_t i = 0; cfgData.mdns[i] != '\0' && w < (sizeof(host) - 1); ++i) {
        char c = cfgData.mdns[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-') {
            host[w++] = (char)tolower((unsigned char)c);
        } else if (c == ' ' || c == '_' || c == '.') {
            host[w++] = '-';
        }
    }
    host[w] = '\0';

    while (w > 0 && host[0] == '-') {
        memmove(host, host + 1, w);
        --w;
    }
    while (w > 0 && host[w - 1] == '-') {
        host[w - 1] = '\0';
        --w;
    }

    if (host[0] == '\0') {
        stopMdns_();
        return;
    }

    if (mdnsStarted && strcmp(mdnsApplied, host) == 0) return;

    if (mdnsStarted) {
        MDNS.end();
        mdnsStarted = false;
        mdnsApplied[0] = '\0';
    }

    if (!MDNS.begin(host)) {
        LOGW("mDNS start failed host=%s", host);
        return;
    }

    mdnsStarted = true;
    snprintf(mdnsApplied, sizeof(mdnsApplied), "%s", host);
    LOGI("mDNS started host=%s.local", mdnsApplied);
}

void WifiModule::loop() {
    processScan_();

    switch (state) {

    case WifiState::Disabled:
        vTaskDelay(pdMS_TO_TICKS(2000));
        break;

    case WifiState::Idle:
        startConnect();
        vTaskDelay(pdMS_TO_TICKS(1000));
        break;

    case WifiState::Connecting:
        if (WiFi.isConnected()) {
            IPAddress ip = WiFi.localIP();
            LOGI("Connected IP=%u.%u.%u.%u RSSI=%d",
            ip[0], ip[1], ip[2], ip[3],
            WiFi.RSSI());
            setState(WifiState::Connected);
        }
        else if (millis() - stateTs > 15000) {
            LOGW("Connect timeout");
            WiFi.disconnect(false, false);
            setState(WifiState::ErrorWait);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
        break;

    case WifiState::Connected:
        if (!WiFi.isConnected()) {
            LOGW("Disconnected");
            setState(WifiState::ErrorWait);
        }
        if (state == WifiState::Connected) {
            syncMdns_();
        }
        if (state == WifiState::Connected && !gotIpSent) {
            IPAddress ip = WiFi.localIP();
            if (ip[0] != 0 || ip[1] != 0 || ip[2] != 0 || ip[3] != 0) {
                if (dataStore) {
                    IpV4 ip4{};
                    ip4.b[0] = ip[0];
                    ip4.b[1] = ip[1];
                    ip4.b[2] = ip[2];
                    ip4.b[3] = ip[3];

                    setWifiIp(*dataStore, ip4);
                    setWifiReady(*dataStore, true);
                }
                gotIpSent = true;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
        break;

    case WifiState::ErrorWait:
        if (millis() - stateTs > 5000) {
            setState(WifiState::Idle);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        break;
    }
}
