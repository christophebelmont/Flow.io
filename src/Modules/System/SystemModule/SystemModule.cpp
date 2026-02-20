/**
 * @file SystemModule.cpp
 * @brief Implementation file.
 */
#include "SystemModule.h"
#include "Core/ErrorCodes.h"
#include <WiFi.h>
#include <esp_system.h>
#include <esp_wifi.h>
#define LOG_TAG "SysModul"
#include "Core/ModuleLog.h"

static bool wipeWifiPersistent_(esp_err_t* outErr)
{
    // Keep WiFi driver initialized while clearing persisted station/AP data.
    WiFi.mode(WIFI_MODE_STA);
    delay(20);
    (void)WiFi.disconnect(false, true); // keep radio on, erase AP credentials

    esp_err_t err = esp_wifi_restore();
    if (err == ESP_ERR_WIFI_NOT_INIT) {
        // Retry once after forcing STA mode in case the driver was not up yet.
        WiFi.mode(WIFI_MODE_STA);
        delay(20);
        err = esp_wifi_restore();
    }

    // Best-effort shutdown after erase.
    (void)WiFi.disconnect(true, true);

    if (outErr) *outErr = err;
    return (err == ESP_OK || err == ESP_ERR_WIFI_NOT_INIT);
}

static bool writeOkReply_(char* reply, size_t replyLen, const char* json, const char* where)
{
    if (!reply || replyLen == 0 || !json) return false;
    const int wrote = snprintf(reply, replyLen, "%s", json);
    if (wrote > 0 && (size_t)wrote < replyLen) return true;
    if (!writeErrorJson(reply, replyLen, ErrorCode::Failed, where)) {
        snprintf(reply, replyLen, "{\"ok\":false}");
    }
    return false;
}

bool SystemModule::cmdPing(void*, const CommandRequest&, char* reply, size_t replyLen) {
    return writeOkReply_(reply, replyLen, "{\"ok\":true,\"pong\":true}", "system.ping");
}

bool SystemModule::cmdReboot(void*, const CommandRequest&, char* reply, size_t replyLen) {
    if (!writeOkReply_(reply, replyLen, "{\"ok\":true,\"msg\":\"rebooting\"}", "system.reboot")) {
        return false;
    }
    delay(200); ///< laisser le temps à MQTT de publier l'ACK
    esp_restart();
    return true;
}

bool SystemModule::cmdFactoryReset(void* userCtx, const CommandRequest&, char* reply, size_t replyLen) {
    SystemModule* self = static_cast<SystemModule*>(userCtx);
    if (!self || !self->cfgSvc || !self->cfgSvc->erase) {
        if (!writeErrorJson(reply, replyLen, ErrorCode::NotReady, "system.factory_reset")) {
            snprintf(reply, replyLen, "{\"ok\":false}");
        }
        return false;
    }

    const bool flowCfgCleared = self->cfgSvc->erase(self->cfgSvc->ctx);

    // Also wipe WiFi driver persisted credentials/settings from default NVS storage.
    esp_err_t wifiRestoreErr = ESP_OK;
    const bool wifiCfgCleared = wipeWifiPersistent_(&wifiRestoreErr);
    if (wifiRestoreErr == ESP_ERR_WIFI_NOT_INIT) {
        LOGW("WiFi driver not initialized during factory reset; restore skipped");
    }

    if (!flowCfgCleared || !wifiCfgCleared) {
        LOGE("Factory reset failed flow_cfg=%d wifi_cfg=%d wifi_err=%d",
             (int)flowCfgCleared,
             (int)wifiCfgCleared,
             (int)wifiRestoreErr);
        if (!writeErrorJson(reply, replyLen, ErrorCode::Failed, "system.factory_reset")) {
            snprintf(reply, replyLen, "{\"ok\":false}");
        }
        return false;
    }

    if (!writeOkReply_(reply, replyLen, "{\"ok\":true,\"msg\":\"factory_reset\"}", "system.factory_reset")) {
        return false;
    }

    LOGI("Factory reset done flow_cfg=%d wifi_cfg=%d wifi_err=%d",
         (int)flowCfgCleared,
         (int)wifiCfgCleared,
         (int)wifiRestoreErr);

    delay(300); ///< laisser le temps à MQTT/Web de publier l'ACK
    esp_restart();
    return true;
}

void SystemModule::init(ConfigStore& cfg, ServiceRegistry& services) {
    (void)cfg;

    logHub = services.get<LogHubService>("loghub");
    cmdSvc = services.get<CommandService>("cmd");
    cfgSvc = services.get<ConfigStoreService>("config");

    cmdSvc->registerHandler(cmdSvc->ctx, "system.ping", cmdPing, this);
    cmdSvc->registerHandler(cmdSvc->ctx, "system.reboot", cmdReboot, this);
    cmdSvc->registerHandler(cmdSvc->ctx, "system.factory_reset", cmdFactoryReset, this);

    LOGI("Commands registered: system.ping system.reboot system.factory_reset");
}
