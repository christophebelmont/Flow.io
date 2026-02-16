/**
 * @file AlarmModule.cpp
 * @brief Implementation file.
 */

#include "AlarmModule.h"

#include "Core/CommandRegistry.h"
#include "Core/ErrorCodes.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <string.h>

#define LOG_TAG "AlarmMod"
#include "Core/ModuleLog.h"

static uint32_t clampEvalPeriodMs_(int32_t inMs)
{
    if (inMs < 25) return 25U;
    if (inMs > 5000) return 5000U;
    return (uint32_t)inMs;
}

static bool parseCmdArgsObject_(const CommandRequest& req, JsonObjectConst& outObj)
{
    static constexpr size_t CMD_DOC_CAPACITY = Limits::Alarm::JsonCmdBuf;
    static StaticJsonDocument<CMD_DOC_CAPACITY> doc;

    doc.clear();
    const char* json = req.args ? req.args : req.json;
    if (!json || json[0] == '\0') return false;

    const DeserializationError err = deserializeJson(doc, json);
    if (!err && doc.is<JsonObject>()) {
        outObj = doc.as<JsonObjectConst>();
        return true;
    }

    if (req.json && req.json[0] != '\0' && req.args != req.json) {
        doc.clear();
        const DeserializationError rootErr = deserializeJson(doc, req.json);
        if (rootErr || !doc.is<JsonObjectConst>()) return false;
        JsonVariantConst argsVar = doc["args"];
        if (argsVar.is<JsonObjectConst>()) {
            outObj = argsVar.as<JsonObjectConst>();
            return true;
        }
    }

    return false;
}

bool AlarmModule::delayReached_(uint32_t sinceMs, uint32_t delayMs, uint32_t nowMs)
{
    if (delayMs == 0U) return true;
    if (sinceMs == 0U) return false;
    return (uint32_t)(nowMs - sinceMs) >= delayMs;
}

const char* AlarmModule::condStateStr_(AlarmCondState s)
{
    if (s == AlarmCondState::True) return "true";
    if (s == AlarmCondState::False) return "false";
    return "unknown";
}

void AlarmModule::emitAlarmEvent_(EventId id, AlarmId alarmId) const
{
    if (!eventBus_) return;
    const AlarmPayload payload{(uint16_t)alarmId};
    (void)eventBus_->post(id, &payload, sizeof(payload));
}

int16_t AlarmModule::findSlotById_(AlarmId id) const
{
    for (uint16_t i = 0; i < Limits::Alarm::MaxAlarms; ++i) {
        if (!slots_[i].used) continue;
        if (slots_[i].id == id) return (int16_t)i;
    }
    return -1;
}

int16_t AlarmModule::findFreeSlot_() const
{
    for (uint16_t i = 0; i < Limits::Alarm::MaxAlarms; ++i) {
        if (!slots_[i].used) return (int16_t)i;
    }
    return -1;
}

bool AlarmModule::registerAlarm_(const AlarmRegistration& def, AlarmCondFn condFn, void* condCtx)
{
    if (!condFn) return false;
    if (def.id == AlarmId::None) return false;
    if (def.code[0] == '\0') return false;
    if (def.title[0] == '\0') return false;

    bool ok = false;
    portENTER_CRITICAL(&slotsMux_);
    if (findSlotById_(def.id) >= 0) {
        ok = false;
    } else {
        const int16_t idx = findFreeSlot_();
        if (idx >= 0) {
            AlarmSlot& s = slots_[(uint16_t)idx];
            s = AlarmSlot{};
            s.used = true;
            s.id = def.id;
            s.def = def;
            s.condFn = condFn;
            s.condCtx = condCtx;
            ok = true;
        }
    }
    portEXIT_CRITICAL(&slotsMux_);

    if (ok) {
        LOGI("Alarm registered id=%u code=%s", (unsigned)def.id, def.code);
    } else {
        LOGW("Alarm registration failed id=%u", (unsigned)def.id);
    }
    return ok;
}

bool AlarmModule::ack_(AlarmId id)
{
    bool postAck = false;
    bool postClear = false;
    char alarmCode[sizeof(slots_[0].def.code)] = {0};
    AlarmCondState ackCond = AlarmCondState::Unknown;
    uint32_t nowMs = millis();

    portENTER_CRITICAL(&slotsMux_);
    const int16_t idx = findSlotById_(id);
    if (idx >= 0) {
        AlarmSlot& s = slots_[(uint16_t)idx];
        if (s.active && s.def.latched && !s.acked) {
            strncpy(alarmCode, s.def.code, sizeof(alarmCode) - 1);
            alarmCode[sizeof(alarmCode) - 1] = '\0';
            s.acked = true;
            ackCond = s.lastCond;
            s.lastChangeMs = nowMs;
            postAck = true;

            if (s.lastCond == AlarmCondState::False && s.def.offDelayMs == 0U) {
                s.active = false;
                s.acked = false;
                s.offSinceMs = 0U;
                s.lastChangeMs = nowMs;
                postClear = true;
            }
        }
    }
    portEXIT_CRITICAL(&slotsMux_);

    if (postAck) {
        LOGD("Alarm ack request accepted id=%u code=%s cond=%s",
             (unsigned)id,
             alarmCode[0] ? alarmCode : "?",
             condStateStr_(ackCond));
        LOGI("Alarm acked id=%u code=%s", (unsigned)id, alarmCode[0] ? alarmCode : "?");
        emitAlarmEvent_(EventId::AlarmAcked, id);
    }
    if (postClear) {
        LOGI("Alarm cleared id=%u code=%s (ack path)", (unsigned)id, alarmCode[0] ? alarmCode : "?");
        emitAlarmEvent_(EventId::AlarmCleared, id);
    }
    return postAck || postClear;
}

uint8_t AlarmModule::ackAll_()
{
    AlarmId pending[Limits::Alarm::MaxAlarms]{};
    uint8_t pendingCount = 0;

    portENTER_CRITICAL(&slotsMux_);
    for (uint16_t i = 0; i < Limits::Alarm::MaxAlarms; ++i) {
        const AlarmSlot& s = slots_[i];
        if (!s.used || !s.active || !s.def.latched || s.acked) continue;
        if (pendingCount < Limits::Alarm::MaxAlarms) {
            pending[pendingCount++] = s.id;
        }
    }
    portEXIT_CRITICAL(&slotsMux_);

    uint8_t acked = 0;
    for (uint8_t i = 0; i < pendingCount; ++i) {
        if (ack_(pending[i])) ++acked;
    }
    return acked;
}

bool AlarmModule::isActive_(AlarmId id) const
{
    bool out = false;
    portENTER_CRITICAL(&slotsMux_);
    const int16_t idx = findSlotById_(id);
    if (idx >= 0) out = slots_[(uint16_t)idx].active;
    portEXIT_CRITICAL(&slotsMux_);
    return out;
}

bool AlarmModule::isAcked_(AlarmId id) const
{
    bool out = false;
    portENTER_CRITICAL(&slotsMux_);
    const int16_t idx = findSlotById_(id);
    if (idx >= 0) out = slots_[(uint16_t)idx].acked;
    portEXIT_CRITICAL(&slotsMux_);
    return out;
}

uint8_t AlarmModule::activeCount_() const
{
    uint8_t count = 0;
    portENTER_CRITICAL(&slotsMux_);
    for (uint16_t i = 0; i < Limits::Alarm::MaxAlarms; ++i) {
        if (slots_[i].used && slots_[i].active) ++count;
    }
    portEXIT_CRITICAL(&slotsMux_);
    return count;
}

AlarmSeverity AlarmModule::highestSeverity_() const
{
    AlarmSeverity highest = AlarmSeverity::Info;
    portENTER_CRITICAL(&slotsMux_);
    for (uint16_t i = 0; i < Limits::Alarm::MaxAlarms; ++i) {
        const AlarmSlot& s = slots_[i];
        if (!s.used || !s.active) continue;
        if ((uint8_t)s.def.severity > (uint8_t)highest) highest = s.def.severity;
    }
    portEXIT_CRITICAL(&slotsMux_);
    return highest;
}

bool AlarmModule::buildSnapshot_(char* out, size_t len) const
{
    if (!out || len == 0) return false;

    AlarmSlot snap[Limits::Alarm::MaxAlarms]{};
    portENTER_CRITICAL(&slotsMux_);
    memcpy(snap, slots_, sizeof(snap));
    portEXIT_CRITICAL(&slotsMux_);

    const uint8_t active = activeCount_();
    const AlarmSeverity highest = highestSeverity_();

    int wrote = snprintf(
        out,
        len,
        "{\"ok\":true,\"active_count\":%u,\"highest_severity\":%u,\"alarms\":[",
        (unsigned)active,
        (unsigned)((uint8_t)highest));
    if (wrote <= 0 || (size_t)wrote >= len) return false;

    size_t pos = (size_t)wrote;
    bool first = true;
    for (uint16_t i = 0; i < Limits::Alarm::MaxAlarms; ++i) {
        const AlarmSlot& s = snap[i];
        if (!s.used) continue;

        wrote = snprintf(
            out + pos,
            len - pos,
            "%s{\"id\":%u,\"code\":\"%s\",\"active\":%s,\"acked\":%s,"
            "\"severity\":%u,\"latched\":%s,\"cond\":\"%s\",\"active_since_ms\":%lu,\"last_change_ms\":%lu}",
            first ? "" : ",",
            (unsigned)s.id,
            s.def.code,
            s.active ? "true" : "false",
            s.acked ? "true" : "false",
            (unsigned)((uint8_t)s.def.severity),
            s.def.latched ? "true" : "false",
            condStateStr_(s.lastCond),
            (unsigned long)s.activeSinceMs,
            (unsigned long)s.lastChangeMs);
        if (wrote <= 0 || (size_t)wrote >= (len - pos)) return false;
        pos += (size_t)wrote;
        first = false;
    }

    if ((len - pos) < 3) return false;
    out[pos++] = ']';
    out[pos++] = '}';
    out[pos] = '\0';
    return true;
}

uint8_t AlarmModule::listIds_(AlarmId* out, uint8_t max) const
{
    if (!out || max == 0) return 0;
    uint8_t count = 0;
    portENTER_CRITICAL(&slotsMux_);
    for (uint16_t i = 0; i < Limits::Alarm::MaxAlarms && count < max; ++i) {
        if (!slots_[i].used) continue;
        out[count++] = slots_[i].id;
    }
    portEXIT_CRITICAL(&slotsMux_);
    return count;
}

bool AlarmModule::buildAlarmState_(AlarmId id, char* out, size_t len) const
{
    if (!out || len == 0) return false;

    AlarmSlot snap{};
    bool found = false;
    portENTER_CRITICAL(&slotsMux_);
    const int16_t idx = findSlotById_(id);
    if (idx >= 0) {
        snap = slots_[(uint16_t)idx];
        found = true;
    }
    portEXIT_CRITICAL(&slotsMux_);
    if (!found) return false;

    const int wrote = snprintf(
        out,
        len,
        "{\"id\":%u,\"a\":%u,\"k\":%u,\"c\":%u,\"s\":%u,\"lc\":%lu}",
        (unsigned)snap.id,
        snap.active ? 1u : 0u,
        snap.acked ? 1u : 0u,
        (unsigned)((uint8_t)snap.lastCond),
        (unsigned)((uint8_t)snap.def.severity),
        (unsigned long)snap.lastChangeMs);
    return (wrote > 0) && ((size_t)wrote < len);
}

bool AlarmModule::svcRegisterAlarm_(void* ctx, const AlarmRegistration* def, AlarmCondFn condFn, void* condCtx)
{
    AlarmModule* self = static_cast<AlarmModule*>(ctx);
    if (!self || !def) return false;
    return self->registerAlarm_(*def, condFn, condCtx);
}

bool AlarmModule::svcAck_(void* ctx, AlarmId id)
{
    AlarmModule* self = static_cast<AlarmModule*>(ctx);
    return self ? self->ack_(id) : false;
}

uint8_t AlarmModule::svcAckAll_(void* ctx)
{
    AlarmModule* self = static_cast<AlarmModule*>(ctx);
    return self ? self->ackAll_() : 0;
}

bool AlarmModule::svcIsActive_(void* ctx, AlarmId id)
{
    AlarmModule* self = static_cast<AlarmModule*>(ctx);
    return self ? self->isActive_(id) : false;
}

bool AlarmModule::svcIsAcked_(void* ctx, AlarmId id)
{
    AlarmModule* self = static_cast<AlarmModule*>(ctx);
    return self ? self->isAcked_(id) : false;
}

uint8_t AlarmModule::svcActiveCount_(void* ctx)
{
    AlarmModule* self = static_cast<AlarmModule*>(ctx);
    return self ? self->activeCount_() : 0;
}

AlarmSeverity AlarmModule::svcHighestSeverity_(void* ctx)
{
    AlarmModule* self = static_cast<AlarmModule*>(ctx);
    return self ? self->highestSeverity_() : AlarmSeverity::Info;
}

bool AlarmModule::svcBuildSnapshot_(void* ctx, char* out, size_t len)
{
    AlarmModule* self = static_cast<AlarmModule*>(ctx);
    return self ? self->buildSnapshot_(out, len) : false;
}

uint8_t AlarmModule::svcListIds_(void* ctx, AlarmId* out, uint8_t max)
{
    AlarmModule* self = static_cast<AlarmModule*>(ctx);
    return self ? self->listIds_(out, max) : 0;
}

bool AlarmModule::svcBuildAlarmState_(void* ctx, AlarmId id, char* out, size_t len)
{
    AlarmModule* self = static_cast<AlarmModule*>(ctx);
    return self ? self->buildAlarmState_(id, out, len) : false;
}

bool AlarmModule::cmdList_(void* userCtx, const CommandRequest&, char* reply, size_t replyLen)
{
    AlarmModule* self = static_cast<AlarmModule*>(userCtx);
    if (!self) return false;
    if (!self->buildSnapshot_(reply, replyLen)) {
        if (!writeErrorJson(reply, replyLen, ErrorCode::InternalAckOverflow, "alarms.list")) {
            snprintf(reply, replyLen, "{\"ok\":false}");
        }
        return false;
    }
    return true;
}

bool AlarmModule::handleCmdAck_(const CommandRequest& req, char* reply, size_t replyLen)
{
    JsonObjectConst args;
    if (!parseCmdArgsObject_(req, args)) {
        if (!writeErrorJson(reply, replyLen, ErrorCode::MissingArgs, "alarms.ack")) {
            snprintf(reply, replyLen, "{\"ok\":false}");
        }
        return false;
    }
    if (!args.containsKey("id")) {
        if (!writeErrorJson(reply, replyLen, ErrorCode::MissingValue, "alarms.ack.id")) {
            snprintf(reply, replyLen, "{\"ok\":false}");
        }
        return false;
    }
    if (!args["id"].is<uint16_t>() && !args["id"].is<uint32_t>() && !args["id"].is<int32_t>()) {
        if (!writeErrorJson(reply, replyLen, ErrorCode::InvalidEventId, "alarms.ack.id")) {
            snprintf(reply, replyLen, "{\"ok\":false}");
        }
        return false;
    }

    const uint32_t idRaw = args["id"].as<uint32_t>();
    const AlarmId id = (AlarmId)((uint16_t)idRaw);
    if (!ack_(id)) {
        if (!writeErrorJson(reply, replyLen, ErrorCode::Failed, "alarms.ack")) {
            snprintf(reply, replyLen, "{\"ok\":false}");
        }
        return false;
    }

    snprintf(reply, replyLen, "{\"ok\":true,\"id\":%u}", (unsigned)((uint16_t)id));
    return true;
}

bool AlarmModule::cmdAck_(void* userCtx, const CommandRequest& req, char* reply, size_t replyLen)
{
    AlarmModule* self = static_cast<AlarmModule*>(userCtx);
    if (!self) return false;
    return self->handleCmdAck_(req, reply, replyLen);
}

bool AlarmModule::cmdAckAll_(void* userCtx, const CommandRequest&, char* reply, size_t replyLen)
{
    AlarmModule* self = static_cast<AlarmModule*>(userCtx);
    if (!self) return false;
    const uint8_t acked = self->ackAll_();
    snprintf(reply, replyLen, "{\"ok\":true,\"acked\":%u}", (unsigned)acked);
    return true;
}

void AlarmModule::init(ConfigStore& cfg, ServiceRegistry& services)
{
    constexpr uint8_t kCfgModuleId = (uint8_t)ConfigModuleId::Alarms;
    constexpr uint16_t kCfgBranchId = (uint16_t)ConfigBranchId::Alarms;
    cfg.registerVar(enabledVar_, kCfgModuleId, kCfgBranchId);
    cfg.registerVar(evalPeriodVar_, kCfgModuleId, kCfgBranchId);

    logHub_ = services.get<LogHubService>("loghub");
    const EventBusService* eb = services.get<EventBusService>("eventbus");
    eventBus_ = eb ? eb->bus : nullptr;
    cmdSvc_ = services.get<CommandService>("cmd");

    (void)services.add("alarms", &alarmSvc_);

    if (cmdSvc_ && cmdSvc_->registerHandler) {
        cmdSvc_->registerHandler(cmdSvc_->ctx, "alarms.list", &AlarmModule::cmdList_, this);
        cmdSvc_->registerHandler(cmdSvc_->ctx, "alarms.ack", &AlarmModule::cmdAck_, this);
        cmdSvc_->registerHandler(cmdSvc_->ctx, "alarms.ack_all", &AlarmModule::cmdAckAll_, this);
    }

    LOGI("Alarm service registered");
    (void)logHub_;
}

void AlarmModule::onConfigLoaded(ConfigStore&, ServiceRegistry&)
{
    evalPeriodMsCfg_ = (int32_t)clampEvalPeriodMs_(evalPeriodMsCfg_);
}

void AlarmModule::evaluateOnce_(uint32_t nowMs)
{
    for (uint16_t i = 0; i < Limits::Alarm::MaxAlarms; ++i) {
        AlarmId id = AlarmId::None;
        AlarmCondFn condFn = nullptr;
        void* condCtx = nullptr;
        bool used = false;

        portENTER_CRITICAL(&slotsMux_);
        if (slots_[i].used) {
            used = true;
            id = slots_[i].id;
            condFn = slots_[i].condFn;
            condCtx = slots_[i].condCtx;
        }
        portEXIT_CRITICAL(&slotsMux_);

        if (!used || !condFn) continue;

        const AlarmCondState cond = condFn(condCtx, nowMs);
        bool postRaised = false;
        bool postCleared = false;
        bool postCondTrue = false;
        bool postCondFalse = false;
        char alarmCode[sizeof(slots_[0].def.code)] = {0};

        portENTER_CRITICAL(&slotsMux_);
        AlarmSlot& s = slots_[i];
        if (s.used && s.id == id && s.condFn == condFn && s.condCtx == condCtx) {
            const AlarmCondState prevCond = s.lastCond;
            if (prevCond != cond) {
                if (cond == AlarmCondState::True) {
                    postCondTrue = true;
                    strncpy(alarmCode, s.def.code, sizeof(alarmCode) - 1);
                    alarmCode[sizeof(alarmCode) - 1] = '\0';
                } else if (cond == AlarmCondState::False) {
                    postCondFalse = true;
                    strncpy(alarmCode, s.def.code, sizeof(alarmCode) - 1);
                    alarmCode[sizeof(alarmCode) - 1] = '\0';
                }
            }
            s.lastCond = cond;

            if (cond == AlarmCondState::True) {
                s.offSinceMs = 0U;
                if (!s.active) {
                    if (s.onSinceMs == 0U) s.onSinceMs = nowMs;
                    if (delayReached_(s.onSinceMs, s.def.onDelayMs, nowMs)) {
                        s.active = true;
                        s.acked = false;
                        s.activeSinceMs = nowMs;
                        s.lastChangeMs = nowMs;
                        s.onSinceMs = 0U;
                        postRaised = true;
                        strncpy(alarmCode, s.def.code, sizeof(alarmCode) - 1);
                        alarmCode[sizeof(alarmCode) - 1] = '\0';
                    }
                } else {
                    s.onSinceMs = 0U;
                }
            } else if (cond == AlarmCondState::False) {
                s.onSinceMs = 0U;
                if (s.active) {
                    const bool canClear = (!s.def.latched) || s.acked;
                    if (canClear) {
                        if (s.offSinceMs == 0U) s.offSinceMs = nowMs;
                        if (delayReached_(s.offSinceMs, s.def.offDelayMs, nowMs)) {
                            s.active = false;
                            s.acked = false;
                            s.offSinceMs = 0U;
                            s.lastChangeMs = nowMs;
                            postCleared = true;
                            strncpy(alarmCode, s.def.code, sizeof(alarmCode) - 1);
                            alarmCode[sizeof(alarmCode) - 1] = '\0';
                        }
                    } else {
                        s.offSinceMs = 0U;
                    }
                } else {
                    s.offSinceMs = 0U;
                }
            } else {
                // Unknown sensor/state: cancel transition timers, keep stable alarm state.
                s.onSinceMs = 0U;
                s.offSinceMs = 0U;
            }
        }
        portEXIT_CRITICAL(&slotsMux_);

        if (postCondTrue) {
            LOGD("Alarm cond=true id=%u code=%s", (unsigned)id, alarmCode[0] ? alarmCode : "?");
            emitAlarmEvent_(EventId::AlarmConditionChanged, id);
        }
        if (postCondFalse) {
            LOGD("Alarm cond=false id=%u code=%s", (unsigned)id, alarmCode[0] ? alarmCode : "?");
            emitAlarmEvent_(EventId::AlarmConditionChanged, id);
        }
        if (postRaised) {
            LOGW("Alarm raised id=%u code=%s cond=%s", (unsigned)id, alarmCode[0] ? alarmCode : "?", condStateStr_(cond));
            emitAlarmEvent_(EventId::AlarmRaised, id);
        }
        if (postCleared) {
            LOGI("Alarm cleared id=%u code=%s cond=%s", (unsigned)id, alarmCode[0] ? alarmCode : "?", condStateStr_(cond));
            emitAlarmEvent_(EventId::AlarmCleared, id);
        }
    }
}

void AlarmModule::loop()
{
    if (!enabled_) {
        vTaskDelay(pdMS_TO_TICKS(500));
        return;
    }

    evaluateOnce_(millis());
    vTaskDelay(pdMS_TO_TICKS(clampEvalPeriodMs_(evalPeriodMsCfg_)));
}
