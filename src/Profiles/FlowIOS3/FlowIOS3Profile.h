#pragma once

#include "App/FirmwareProfile.h"
#include "Core/SystemLimits.h"
#include "Modules/AlarmModule/AlarmModule.h"
#include "Modules/CommandModule/CommandModule.h"
#include "Modules/EventBusModule/EventBusModule.h"
#include "Modules/HMIModule/HMIModule.h"
#include "Modules/IOModule/IOBus/OneWireBus.h"
#include "Modules/IOModule/IOModule.h"
#include "Modules/Logs/LogDispatcherModule/LogDispatcherModule.h"
#include "Modules/Logs/LogHubModule/LogHubModule.h"
#include "Modules/Logs/LogSerialSinkModule/LogSerialSinkModule.h"
#include "Modules/Network/HAModule/HAModule.h"
#include "Modules/Network/FirmwareUpdateModule/FirmwareUpdateModule.h"
#include "Modules/Network/HmiUdpServerModule/HmiUdpServerModule.h"
#include "Modules/Network/MQTTModule/MQTTModule.h"
#include "Modules/Network/TimeModule/TimeModule.h"
#include "Modules/Network/WebInterfaceModule/WebInterfaceModule.h"
#include "Modules/Network/WifiProvisioningModule/WifiProvisioningModule.h"
#include "Modules/Network/WifiModule/WifiModule.h"
#include "Modules/PoolDeviceModule/PoolDeviceModule.h"
#include "Modules/PoolLogicModule/PoolLogicModule.h"
#include "Modules/Stores/ConfigStoreModule/ConfigStoreModule.h"
#include "Modules/Stores/DataStoreModule/DataStoreModule.h"
#include "Modules/System/StatusLedModule/StatusLedModule.h"
#include "Modules/System/SystemModule/SystemModule.h"
#include "Modules/System/SystemMonitorModule/SystemMonitorModule.h"

class DataStore;
struct HAService;

namespace Profiles {
namespace FlowIOS3 {

struct ModuleInstances {
    explicit ModuleInstances(const BoardSpec& board);

    WifiModule wifiModule;
    TimeModule timeModule{};
    WifiProvisioningModule wifiProvisioningModule{};
    WebInterfaceModule webInterfaceModule;
    FirmwareUpdateModule firmwareUpdateModule;
    CommandModule commandModule{};
    ConfigStoreModule configStoreModule{};
    DataStoreModule dataStoreModule{};
    MQTTModule mqttModule{};
    HAModule haModule{};
    StatusLedModule statusLedModule{};
    SystemModule systemModule{};
    SystemMonitorModule systemMonitorModule{};
    LogSerialSinkModule logSerialSinkModule{};
    LogDispatcherModule logDispatcherModule{};
    LogHubModule logHubModule{};
    EventBusModule eventBusModule{};
    AlarmModule alarmModule{};
    HmiUdpServerModule hmiUdpServerModule{};
    HMIModule hmiModule{};
    IOModule ioModule;
    PoolDeviceModule poolDeviceModule{};
    PoolLogicModule poolLogicModule{};
    OneWireBus oneWireWater{3};
    OneWireBus oneWireAir{2};
    DataStore* ioDataStore = nullptr;
    const HAService* haService = nullptr;
    char topicNetworkState[Limits::TopicBuf] = {0};
    char topicSystemState[Limits::TopicBuf] = {0};
};

ModuleInstances& moduleInstances();
const FirmwareProfile& profile();
void setupProfile(AppContext& ctx);
void loopProfile(AppContext& ctx);

}  // namespace FlowIOS3
}  // namespace Profiles
