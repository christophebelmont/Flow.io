/**
 * @file WifiModule.cpp
 * @brief Implementation file.
 */
#include "WifiModule.h"
#define LOG_TAG "WifiModu"
#include "Core/ModuleLog.h"
#include "Core/Runtime.h"


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

void WifiModule::setState(WifiState s) {
    if (s == state) return;
    state = s;
    stateTs = millis();

    if (state == WifiState::Idle || state == WifiState::ErrorWait) {
        if (dataStore) {
            setWifiReady(*dataStore, false);
        }
        gotIpSent = false;
    }
}

void WifiModule::startConnect() {
    if (cfgData.ssid[0] == '\0') {
        LOGW("SSID empty, skipping connection");
        setState(WifiState::Idle);
        return;
    }

    LOGI("Connecting to '%s'", cfgData.ssid);

    WiFi.disconnect(true);
    delay(50);

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);               ///< ✅ important (stability)
    WiFi.begin(cfgData.ssid, cfgData.pass);

    setState(WifiState::Connecting);
}

void WifiModule::init(ConfigStore& cfg,
                      I2CManager&,
                      ServiceRegistry& services)
{
    /// récupérer service loghub (log async)
    logHub = services.get<LogHubService>("loghub");

    const DataStoreService* dsSvc = services.get<DataStoreService>("datastore");
    dataStore = dsSvc ? dsSvc->store : nullptr;

    /// Register config vars
    cfg.registerVar(enabledVar);
    cfg.registerVar(ssidVar);
    cfg.registerVar(passVar);

    /// Register WifiService
    static WifiService svc {
        WifiModule::svcState,
        WifiModule::svcIsConnected,
        WifiModule::svcGetIP,
        this
    };
    services.add("wifi", &svc);

    LOGI("WifiService registered");

    if (!cfgData.enabled) {
        setState(WifiState::Disabled);
        return;
    }

    setState(WifiState::Idle);
}

void WifiModule::loop() {
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
            WiFi.disconnect(true);
            setState(WifiState::ErrorWait);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
        break;

    case WifiState::Connected:
        if (!WiFi.isConnected()) {
            LOGW("Disconnected");
            setState(WifiState::ErrorWait);
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
