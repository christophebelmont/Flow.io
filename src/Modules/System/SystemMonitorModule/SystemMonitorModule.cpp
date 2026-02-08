/**
 * @file SystemMonitorModule.cpp
 * @brief Implementation file.
 */
#include "SystemMonitorModule.h"
#include "Core/ModuleManager.h"   ///< required for iteration
#include <Arduino.h>
#include <WiFi.h>                ///< only for RSSI (optional)
#define LOG_TAG "SysMonit"
#include "Core/ModuleLog.h"


const char* SystemMonitorModule::wifiStateStr(WifiState st) {
    switch (st) {
    case WifiState::Disabled:    return "Disabled";
    case WifiState::Idle:        return "Idle";
    case WifiState::Connecting:  return "Connecting";
    case WifiState::Connected:   return "Connected";
    case WifiState::ErrorWait:   return "ErrorWait";
    default:                     return "Unknown";
    }
}

void SystemMonitorModule::init(ConfigStore&, ServiceRegistry& services) {
    wifiSvc = services.get<WifiService>("wifi");
    cfgSvc  = services.get<ConfigStoreService>("config");
    logHub  = services.get<LogHubService>("loghub");

    LOGI("Starting SystemMonitorModule");
    logBootInfo();
}

void SystemMonitorModule::logBootInfo() {
    LOGI("Reset reason=%s", SystemStats::resetReasonStr());
    LOGI("CPU=%luMHz", (unsigned long)ESP.getCpuFreqMHz());
}

void SystemMonitorModule::logHeapAndWifi() {
    SystemStatsSnapshot snap{};
    SystemStats::collect(snap);

    LOGI("free=%lu min=%lu largest=%lu frag=%u%%",
                (unsigned long)snap.heap.freeBytes,
                (unsigned long)snap.heap.minFreeBytes,
                (unsigned long)snap.heap.largestFreeBlock,
                (unsigned int)snap.heap.fragPercent);

    if (wifiSvc) {
        WifiState st = wifiSvc->state(wifiSvc->ctx);
        bool con = wifiSvc->isConnected(wifiSvc->ctx);

        char ip[16] = "";
        wifiSvc->getIP(wifiSvc->ctx, ip, sizeof(ip));

        int rssi = -127;
        if (con) rssi = WiFi.RSSI(); ///< optional but very useful

        LOGI("WIFI state=%s connected=%d ip=%s rssi=%d",
                    wifiStateStr(st),
                    con ? 1 : 0,
                    ip,
                    rssi);
    }
}

void SystemMonitorModule::logTaskStacks() {
    if (!moduleManager) {
        LOGW("ModuleManager not set, task stats disabled");
        return;
    }

    /// Dump every 30s
    uint32_t now = millis();
    if (now - lastStackDumpMs < 30000) return;
    lastStackDumpMs = now;

    LOGI("Stack High Water Marks:");

    const uint8_t n = moduleManager->getCount();
    for (uint8_t i = 0; i < n; ++i) {
        Module* m = moduleManager->getModule(i);
        if (!m) continue;

        TaskHandle_t h = m->getTaskHandle();
        if (!h) continue;

        UBaseType_t hw = uxTaskGetStackHighWaterMark(h);

        /// Warn if stack is too low
        if (hw < 300) {
            LOGW("%s stackHW=%u (LOW!)", m->moduleId(), (unsigned)hw);
        } else {
            LOGI("%s stackHW=%u", m->moduleId(), (unsigned)hw);
        }
    }
}

void SystemMonitorModule::buildHealthJson(char* out, size_t outLen) {
    SystemStatsSnapshot snap{};
    SystemStats::collect(snap);

    /// wifi
    WifiState wst = WifiState::Disabled;
    bool wcon = false;
    char ip[16] = "";
    int rssi = -127;

    if (wifiSvc) {
        wst = wifiSvc->state(wifiSvc->ctx);
        wcon = wifiSvc->isConnected(wifiSvc->ctx);
        wifiSvc->getIP(wifiSvc->ctx, ip, sizeof(ip));
        if (wcon) rssi = WiFi.RSSI();
    }

    snprintf(out, outLen,
        "{"
            "\"upt_ms\":%lu,"
            "\"heap\":{"
                "\"free\":%lu,"
                "\"min\":%lu,"
                "\"largest\":%lu,"
                "\"frag\":%u"
            "}"
        "}",
        (unsigned long)snap.uptimeMs,
        (unsigned long)snap.heap.freeBytes,
        (unsigned long)snap.heap.minFreeBytes,
        (unsigned long)snap.heap.largestFreeBlock,
        (unsigned int)snap.heap.fragPercent
    );
}

void SystemMonitorModule::loop() {
    /// Every 10s basic health log
    logHeapAndWifi();

    /// Every 30s: stacks
    logTaskStacks();

    /// Every 60s: JSON health dump
    uint32_t now = millis();
    if (now - lastJsonDumpMs > 60000) {
        lastJsonDumpMs = now;
        char json[512];
        buildHealthJson(json, sizeof(json));
        LOGI("health=%s", json);
    }

    vTaskDelay(pdMS_TO_TICKS(10000));
}
