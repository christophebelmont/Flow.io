#pragma once
/**
 * @file NTPModule.h
 * @brief NTP time synchronization module.
 */
#include "Core/Module.h"
#include "Core/Services/Services.h"
#include <WiFi.h>
/** @brief NTP configuration values. */
struct NTPConfig {
    char server1[40] = "pool.ntp.org";
    char server2[40] = "time.nist.gov";
    char tz[64]      = "CET-1CEST,M3.5.0/2,M10.5.0/3";
    bool enabled = true;
};

/**
 * @brief Active module that synchronizes time over NTP.
 */
class NTPModule : public Module {
public:
    /** @brief Module id. */
    const char* moduleId() const override { return "ntp"; }
    /** @brief Task name. */
    const char* taskName() const override { return "ntp"; }

    /** @brief Depends on log hub and datastore. */
    uint8_t dependencyCount() const override { return 2; }
    const char* dependency(uint8_t i) const override {
        if (i == 0) return "loghub";
        if (i == 1) return "datastore";
        return nullptr;
    }

    /** @brief Initialize NTP config/services. */
    void init(ConfigStore& cfg, ServiceRegistry& services) override;
    /** @brief NTP task loop. */
    void loop() override;

    /** @brief Force a resync attempt. */
    void forceResync();

private:
    NTPConfig cfgData;

    //const WifiService* wifiSvc = nullptr;
    const CommandService* cmdSvc = nullptr;
    const LogHubService* logHub = nullptr;
    EventBus* eventBus = nullptr;
    DataStore* dataStore = nullptr;

    static void onEventStatic(const Event& e, void* user);
    void onEvent(const Event& e);

    static bool cmdResync(void* userCtx, const CommandRequest& req, char* reply, size_t replyLen);

    NTPState state = NTPState::WaitingNetwork;
    uint32_t stateTs = 0;

    ConfigVariable<char,0> server1Var {
        NVS_KEY("ntp_s1"),"server1","ntp",ConfigType::CharArray,
        (char*)cfgData.server1,ConfigPersistence::Persistent,sizeof(cfgData.server1)
    };
    ConfigVariable<char,0> server2Var {
        NVS_KEY("ntp_s2"),"server2","ntp",ConfigType::CharArray,
        (char*)cfgData.server2,ConfigPersistence::Persistent,sizeof(cfgData.server2)
    };
    ConfigVariable<char,0> tzVar {
        NVS_KEY("ntp_tz"),"tz","ntp",ConfigType::CharArray,
        (char*)cfgData.tz,ConfigPersistence::Persistent,sizeof(cfgData.tz)
    };
    ConfigVariable<bool,0> enabledVar {
        NVS_KEY("ntp_en"),"enabled","ntp",ConfigType::Bool,
        &cfgData.enabled,ConfigPersistence::Persistent,0
    };

    void setState(NTPState s);

    static NTPState svcState(void* ctx);
    static bool svcIsSynced(void* ctx);
    static uint64_t svcEpoch(void* ctx);
    static bool svcFormatLocalTime(void* ctx, char* out, size_t len);

    // ---- network warmup ----
    bool _netReady = false;
    uint32_t _netReadyTs = 0;

    // ---- retry backoff ----
    uint8_t _retryCount = 0;
    uint32_t _retryDelayMs = 2000; // 2s start
};
