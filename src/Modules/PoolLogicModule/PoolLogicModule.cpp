/**
 * @file PoolLogicModule.cpp
 * @brief Implementation file.
 */
#include "PoolLogicModule.h"
#include "Modules/IOModule/IORuntime.h"
#include <math.h>
#include <cstring>
#define LOG_TAG "PoolLogc"
#include "Core/ModuleLog.h"

void PoolLogicModule::init(ConfigStore& cfg, ServiceRegistry& services)
{
    cfgStore_ = &cfg;
    cfg.registerVar(enabledVar_);
    cfg.registerVar(tempLowVar_);
    cfg.registerVar(tempSetpointVar_);
    cfg.registerVar(startMinVar_);
    cfg.registerVar(stopMaxVar_);
    cfg.registerVar(waterTempIdxVar_);

    logHub_ = services.get<LogHubService>("loghub");
    const DataStoreService* dsSvc = services.get<DataStoreService>("datastore");
    dataStore_ = dsSvc ? dsSvc->store : nullptr;
    const EventBusService* ebSvc = services.get<EventBusService>("eventbus");
    eventBus_ = ebSvc ? ebSvc->bus : nullptr;
    schedSvc_ = services.get<TimeSchedulerService>("time.scheduler");

    if (eventBus_) {
        eventBus_->subscribe(EventId::SchedulerEventTriggered, &PoolLogicModule::onEventStatic_, this);
    }

    if (!enabled_) {
        LOGI("PoolLogic disabled");
        return;
    }

    ensureDailySlot_();
    LOGI("PoolLogic ready: daily recalc slot=%u at 15:00", (unsigned)SLOT_DAILY_RECALC);
}

void PoolLogicModule::onEventStatic_(const Event& e, void* user)
{
    if (!user) return;
    static_cast<PoolLogicModule*>(user)->onEvent_(e);
}

void PoolLogicModule::onEvent_(const Event& e)
{
    if (!enabled_) return;
    if (e.id != EventId::SchedulerEventTriggered) return;
    if (!e.payload || e.len < sizeof(SchedulerEventTriggeredPayload)) return;

    const SchedulerEventTriggeredPayload* p = (const SchedulerEventTriggeredPayload*)e.payload;
    if ((SchedulerEdge)p->edge != SchedulerEdge::Trigger) return;
    if (p->eventId != POOLLOGIC_EVENT_DAILY_RECALC) return;

    recalcAndApplyFiltrationWindow_();
}

void PoolLogicModule::ensureDailySlot_()
{
    if (!schedSvc_ || !schedSvc_->setSlot) {
        LOGW("time.scheduler service unavailable");
        return;
    }

    TimeSchedulerSlot recalc{};
    recalc.slot = SLOT_DAILY_RECALC;
    recalc.eventId = POOLLOGIC_EVENT_DAILY_RECALC;
    recalc.enabled = true;
    recalc.hasEnd = false;
    recalc.replayStartOnBoot = false;
    recalc.mode = TimeSchedulerMode::RecurringClock;
    recalc.weekdayMask = TIME_WEEKDAY_ALL;
    recalc.startHour = 15;
    recalc.startMinute = 0;
    recalc.endHour = 0;
    recalc.endMinute = 0;
    recalc.startEpochSec = 0;
    recalc.endEpochSec = 0;
    strncpy(recalc.label, "poollogic_daily_recalc", sizeof(recalc.label) - 1);
    recalc.label[sizeof(recalc.label) - 1] = '\0';

    if (!schedSvc_->setSlot(schedSvc_->ctx, &recalc)) {
        LOGW("Failed to set scheduler slot %u", (unsigned)SLOT_DAILY_RECALC);
    }
}

bool PoolLogicModule::computeFiltrationWindow_(float waterTemp, uint8_t& startHourOut, uint8_t& stopHourOut, uint8_t& durationOut)
{
    if (!isfinite(waterTemp)) return false;

    int duration = 2;
    if (waterTemp < waterTempLowThreshold_) {
        duration = 2;
    } else if (waterTemp < waterTempSetpoint_) {
        duration = (int)lroundf(waterTemp / 3.0f);
    } else {
        duration = (int)lroundf(waterTemp / 2.0f);
    }

    if (duration < 2) duration = 2;
    if (duration > 24) duration = 24;

    const int startMin = (int)filtrationStartMin_;
    int stopMax = (int)filtrationStopMax_;
    if (stopMax > 23) stopMax = 23;
    if (stopMax < 0) stopMax = 0;

    int start = 15 - (int)lroundf((float)duration / 2.0f);
    if (start < startMin) start = startMin;

    int stop = start + duration;
    if (stop > stopMax) stop = stopMax;

    if (stop <= start) {
        if (start < 23) {
            stop = start + 1;
        } else {
            start = 22;
            stop = 23;
        }
    }

    durationOut = (uint8_t)(stop - start);
    startHourOut = (uint8_t)start;
    stopHourOut = (uint8_t)stop;
    return true;
}

void PoolLogicModule::recalcAndApplyFiltrationWindow_()
{
    if (!dataStore_) {
        LOGW("No datastore available for water temperature");
        return;
    }
    if (!schedSvc_ || !schedSvc_->setSlot) {
        LOGW("No time.scheduler service available");
        return;
    }

    float waterTemp = 0.0f;
    if (!ioEndpointFloat(*dataStore_, waterTempIoIndex_, waterTemp)) {
        LOGW("Water temperature unavailable on io idx=%u", (unsigned)waterTempIoIndex_);
        return;
    }

    uint8_t startHour = 0;
    uint8_t stopHour = 0;
    uint8_t duration = 0;
    if (!computeFiltrationWindow_(waterTemp, startHour, stopHour, duration)) {
        LOGW("Invalid water temperature value");
        return;
    }

    TimeSchedulerSlot window{};
    window.slot = SLOT_FILTR_WINDOW;
    window.eventId = POOLLOGIC_EVENT_FILTRATION_WINDOW;
    window.enabled = true;
    window.hasEnd = true;
    window.replayStartOnBoot = true;
    window.mode = TimeSchedulerMode::RecurringClock;
    window.weekdayMask = TIME_WEEKDAY_ALL;
    window.startHour = startHour;
    window.startMinute = 0;
    window.endHour = stopHour;
    window.endMinute = 0;
    window.startEpochSec = 0;
    window.endEpochSec = 0;
    strncpy(window.label, "poollogic_filtration", sizeof(window.label) - 1);
    window.label[sizeof(window.label) - 1] = '\0';

    if (!schedSvc_->setSlot(schedSvc_->ctx, &window)) {
        LOGW("Failed to set filtration window slot=%u", (unsigned)SLOT_FILTR_WINDOW);
        return;
    }

    LOGI("Filtration duration: %uh (water=%.2fC)", (unsigned)duration, (double)waterTemp);
    LOGI("Start: %uh - Stop: %uh", (unsigned)startHour, (unsigned)stopHour);
}
