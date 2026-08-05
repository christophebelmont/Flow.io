/**
 * @file DomainStatusServiceProvider.cpp
 * @brief Shared evaluator backing DomainStatusService.
 */

#include "Domain/DomainStatusServiceProvider.h"

#include "Modules/PoolDeviceModule/PoolDeviceModuleDataModel.h"

void DomainStatusServiceProvider::configure(const DomainSpec& domain,
                                            bool (*bindingPortExists)(uint16_t bindingPort))
{
    domain_ = &domain;
    bindingPortExistsFn_ = bindingPortExists;
}

void DomainStatusServiceProvider::bindServices(const IOServiceV2* ioSvc,
                                               const PoolDeviceService* poolSvc)
{
    ioSvc_ = ioSvc;
    poolSvc_ = poolSvc;
}

const DomainSlotPreset* DomainStatusServiceProvider::findDomainSlot_(DomainSlotId domainSlot) const
{
    if (!domain_) return nullptr;
    for (uint8_t i = 0U; i < domain_->domainSlotCount; ++i) {
        if (domain_->domainSlots[i].id == domainSlot) return &domain_->domainSlots[i];
    }
    return nullptr;
}

const DomainIoSlotBinding* DomainStatusServiceProvider::findBinding_(DomainSlotId domainSlot) const
{
    if (!domain_) return nullptr;
    for (uint8_t i = 0U; i < domain_->domainIoSlotBindingCount; ++i) {
        if (domain_->domainIoSlotBindings[i].domainSlot == domainSlot) {
            return &domain_->domainIoSlotBindings[i];
        }
    }
    return nullptr;
}

const PoolDevicePreset* DomainStatusServiceProvider::findPoolDevice_(DomainSlotId domainSlot) const
{
    if (!domain_) return nullptr;
    for (uint8_t i = 0U; i < domain_->poolDeviceCount; ++i) {
        if (domain_->poolDevices[i].commandSlot == domainSlot) return &domain_->poolDevices[i];
    }
    return nullptr;
}

bool DomainStatusServiceProvider::bindingPortExists_(uint16_t bindingPort) const
{
    return bindingPortExistsFn_ && bindingPortExistsFn_(bindingPort);
}

bool DomainStatusServiceProvider::slotStatus_(DomainSlotId domainSlot, DomainSlotStatus* outStatus) const
{
    if (!outStatus || !findDomainSlot_(domainSlot)) return false;

    *outStatus = DomainSlotStatus{};
    outStatus->domainSlot = domainSlot;

    const DomainIoSlotBinding* binding = findBinding_(domainSlot);
    if (!binding || binding->ioSlot == IO_SLOT_INVALID) {
        outStatus->errorReason = DomainSlotErrorReason::Unbound;
        return true;
    }

    outStatus->ioSlot = binding->ioSlot;
    outStatus->ioId = ioIdFromSlot(binding->ioSlot);
    if (!ioSvc_ || !ioSvc_->meta ||
        ioSvc_->meta(ioSvc_->ctx, outStatus->ioId, &outStatus->meta) != IO_OK) {
        outStatus->errorReason = DomainSlotErrorReason::NotConfigured;
        return true;
    }

    outStatus->hasMeta = 1U;
    outStatus->hasBindingPort = bindingPortExists_(outStatus->meta.bindingPort) ? 1U : 0U;

    const PoolDevicePreset* devicePreset = findPoolDevice_(domainSlot);
    if (devicePreset && poolSvc_ && poolSvc_->meta) {
        PoolDeviceSvcMeta poolMeta{};
        if (poolSvc_->meta(poolSvc_->ctx, devicePreset->id, &poolMeta) == POOLDEV_SVC_OK && poolMeta.used) {
            outStatus->hasPoolDevice = 1U;
            outStatus->poolMeta = poolMeta;
            if (poolSvc_->readActualOn) {
                (void)poolSvc_->readActualOn(poolSvc_->ctx,
                                             devicePreset->id,
                                             &outStatus->poolActualOn,
                                             &outStatus->poolActualTsMs);
            }
        }
    }

    if (!outStatus->hasBindingPort) {
        outStatus->errorReason = DomainSlotErrorReason::NoBinding;
        return true;
    }

    if (outStatus->meta.kind != IO_KIND_DIGITAL_OUT && ioSvc_->sensorStatus) {
        IoSensorStatus sensor{};
        const IoStatus sensorResult = ioSvc_->sensorStatus(ioSvc_->ctx, outStatus->ioId, &sensor);
        if (sensorResult == IO_OK && sensor.enabled == 0U) {
            outStatus->errorReason = DomainSlotErrorReason::Disabled;
            return true;
        }
        if (sensorResult == IO_OK && sensor.enabled != 0U && sensor.valid == 0U) {
            outStatus->state = DomainSlotRuntimeState::Error;
            outStatus->error = 1U;
            outStatus->errorReason = DomainSlotErrorReason::NoValidValue;
            return true;
        }
    }

    if (outStatus->hasPoolDevice) {
        if (!outStatus->poolMeta.enabled ||
            outStatus->poolMeta.blockReason == POOL_DEVICE_BLOCK_DISABLED) {
            outStatus->errorReason = DomainSlotErrorReason::Disabled;
            return true;
        }
        if (outStatus->poolMeta.blockReason != POOL_DEVICE_BLOCK_NONE) {
            outStatus->state = DomainSlotRuntimeState::Error;
            outStatus->error = 1U;
            outStatus->errorReason = DomainSlotErrorReason::PoolDeviceBlocked;
            return true;
        }
    }

    if (ioSvc_->readValue &&
        ioSvc_->readValue(ioSvc_->ctx, outStatus->ioId, &outStatus->value) == IO_OK &&
        outStatus->value.valid) {
        outStatus->state = DomainSlotRuntimeState::Active;
        outStatus->active = 1U;
        outStatus->hasValue = 1U;
        return true;
    }

    outStatus->state = DomainSlotRuntimeState::Error;
    outStatus->error = 1U;
    outStatus->errorReason = DomainSlotErrorReason::ReadFailed;
    return true;
}

bool DomainStatusServiceProvider::summary_(DomainStatusSummary* outSummary) const
{
    if (!outSummary || !domain_) return false;
    *outSummary = DomainStatusSummary{};
    outSummary->total = domain_->domainSlotCount;

    for (uint8_t i = 0U; i < domain_->domainSlotCount; ++i) {
        DomainSlotStatus status{};
        if (!slotStatus_(domain_->domainSlots[i].id, &status)) continue;
        if (status.state == DomainSlotRuntimeState::Active) ++outSummary->active;
        else if (status.state == DomainSlotRuntimeState::Error) ++outSummary->error;
        else ++outSummary->sleeping;
    }
    return true;
}

bool DomainStatusServiceProvider::hasDomainSlotError_() const
{
    DomainSlotStatus status{};
    return firstError_(&status);
}

bool DomainStatusServiceProvider::firstError_(DomainSlotStatus* outStatus) const
{
    if (!outStatus || !domain_) return false;
    for (uint8_t i = 0U; i < domain_->domainSlotCount; ++i) {
        DomainSlotStatus status{};
        if (!slotStatus_(domain_->domainSlots[i].id, &status) || !status.error) continue;
        *outStatus = status;
        return true;
    }
    *outStatus = DomainSlotStatus{};
    return false;
}
