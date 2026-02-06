/**
 * @file NTPModule.cpp
 * @brief Implementation file.
 */
#include "NTPModule.h"
#include "Core/Runtime.h"
#include <time.h>
#include <cstdlib>
#define LOG_TAG "NtpModul"
#include "Core/ModuleLog.h"


static uint32_t clampU32(uint32_t v, uint32_t minV, uint32_t maxV) {
    if (v < minV) return minV;
    if (v > maxV) return maxV;
    return v;
}

void NTPModule::setState(NTPState s) {
    state = s;
    stateTs = millis();
    if (dataStore) {
        setTimeReady(*dataStore, s == NTPState::Synced);
    }
}

NTPState NTPModule::svcState(void* ctx) {
    return static_cast<NTPModule*>(ctx)->state;
}

bool NTPModule::svcIsSynced(void* ctx) {
    auto self = static_cast<NTPModule*>(ctx);
    return self->state == NTPState::Synced;
}

uint64_t NTPModule::svcEpoch(void*) {
    time_t now;
    time(&now);
    return (uint64_t)now;
}

bool NTPModule::svcFormatLocalTime(void*, char* out, size_t len) {
    struct tm t;
    if (!getLocalTime(&t, 50)) return false;
    snprintf(out, len, "%04d-%02d-%02d %02d:%02d:%02d",
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
             t.tm_hour, t.tm_min, t.tm_sec);
    return true;
}

void NTPModule::init(ConfigStore& cfg, I2CManager&, ServiceRegistry& services) {
    cfg.registerVar(server1Var);
    cfg.registerVar(server2Var);
    cfg.registerVar(tzVar);
    cfg.registerVar(enabledVar);

    logHub = services.get<LogHubService>("loghub");

    auto* ebSvc = services.get<EventBusService>("eventbus");
    eventBus = ebSvc ? ebSvc->bus : nullptr;

    const DataStoreService* dsSvc = services.get<DataStoreService>("datastore");
    dataStore = dsSvc ? dsSvc->store : nullptr;

    if (eventBus) {
        eventBus->subscribe(EventId::DataChanged, &NTPModule::onEventStatic, this);
    }

    cmdSvc = services.get<CommandService>("cmd");
    if (cmdSvc) {
        cmdSvc->registerHandler(cmdSvc->ctx, "ntp.resync", cmdResync, this);
    }

    static TimeService timeSvc{
        svcState,
        svcIsSynced,
        svcEpoch,
        svcFormatLocalTime,
        nullptr
    };
    timeSvc.ctx = this;
    services.add("time", &timeSvc);

    LOGI("Timeservice registered");

    /// init internal state
    _netReady = false;
    _netReadyTs = 0;
    _retryCount = 0;
    _retryDelayMs = 2000;

    setState(cfgData.enabled ? NTPState::WaitingNetwork : NTPState::Disabled);
}

void NTPModule::loop() {
    if (!cfgData.enabled) {
        if (state != NTPState::Disabled) setState(NTPState::Disabled);
        vTaskDelay(pdMS_TO_TICKS(2000));
        return;
    }

    switch (state) {

    case NTPState::WaitingNetwork:
        /// If network becomes ready, onEvent() will update _netReady and _netReadyTs.
        /// Here we just wait the warmup delay before syncing.
        if (_netReady) {
            constexpr uint32_t WARMUP_MS = 2000; ///< <-- adjust if needed
            if (millis() - _netReadyTs >= WARMUP_MS) {
                LOGI("Network warmup done -> start syncing");
                setState(NTPState::Syncing);
            }
        }
        break;

    case NTPState::Syncing: {
        LOGI("Syncing...");

        configTzTime(cfgData.tz, cfgData.server1, cfgData.server2);

        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 4000)) {
            char buf[32];
            svcFormatLocalTime(nullptr, buf, sizeof(buf));
            LOGI("Synced ok: %s", buf);

            /// reset backoff on success
            _retryCount = 0;
            _retryDelayMs = 2000;

            setState(NTPState::Synced);
        } else {
            LOGW("Sync failed -> retry in %lu ms", (unsigned long)_retryDelayMs);
            setState(NTPState::ErrorWait);
        }
        break;
    }

    case NTPState::ErrorWait:
        /// if network lost during error wait: go back waiting
        if (!_netReady) {
            setState(NTPState::WaitingNetwork);
            break;
        }

        if (millis() - stateTs >= _retryDelayMs) {
            /// exponential-ish backoff
            _retryCount++;
            uint32_t next = _retryDelayMs;

            /// Backoff sequence (simple and stable):
            /// 2s, 5s, 10s, 30s, 60s, 300s (max)
            if      (next < 5000)   next = 5000;
            else if (next < 10000)  next = 10000;
            else if (next < 30000)  next = 30000;
            else if (next < 60000)  next = 60000;
            else                    next = 300000;

            _retryDelayMs = clampU32(next, 2000, 300000);
            setState(NTPState::Syncing);
        }
        break;

    case NTPState::Synced:
        /// Periodic resync every 6h if network still available
        if (_netReady && (millis() - stateTs > 6UL * 3600UL * 1000UL)) {
            setState(NTPState::Syncing);
        }
        break;

    case NTPState::Disabled:
        setState(NTPState::WaitingNetwork);
        break;
    }

    vTaskDelay(pdMS_TO_TICKS(250));
}

void NTPModule::forceResync() {
    if (!cfgData.enabled) return;
    if (!_netReady) {
        setState(NTPState::WaitingNetwork);
        return;
    }

    /// reset retry backoff for manual resync
    _retryCount = 0;
    _retryDelayMs = 2000;
    _netReadyTs = millis(); ///< restart warmup
    setState(NTPState::WaitingNetwork);
}

bool NTPModule::cmdResync(void* userCtx,
                          const CommandRequest&,
                          char* reply,
                          size_t replyLen)
{
    NTPModule* self = (NTPModule*)userCtx;
    self->forceResync();
    snprintf(reply, replyLen, "{\"ok\":true}");
    return true;
}

void NTPModule::onEventStatic(const Event& e, void* user)
{
    static_cast<NTPModule*>(user)->onEvent(e);
}

void NTPModule::onEvent(const Event& e)
{
    if (e.id != EventId::DataChanged) return;

    const DataChangedPayload* p = (const DataChangedPayload*)e.payload;
    if (!p) return;

    if (p->id != DATAKEY_WIFI_READY) return;

    if (!dataStore) return;

    bool ready = wifiReady(*dataStore);
    if (ready == _netReady) return;

    _netReady = ready;
    _netReadyTs = millis();

    if (_netReady) {
        /// Do not immediately sync; wait warmup in loop()
        LOGI("DataStore networkReady=true -> warmup");
        if (state == NTPState::Synced) return; ///< do not disturb if already synced
        setState(NTPState::WaitingNetwork);
    } else {
        LOGI("DataStore networkReady=false -> stop and wait");
        setState(NTPState::WaitingNetwork);
    }
}
