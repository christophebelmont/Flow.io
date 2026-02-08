/**
 * @file main.cpp
 * @brief Firmware entry point and module wiring.
 */
#include <Arduino.h>
#include <Preferences.h>    ///< Preference needs to be singleton-like global to work

/// Load Core Functions
#include "Core/LogHub.h"
#include "Core/LogSinkRegistry.h"

#include "Core/ConfigMigrations.h"
#include "Core/ConfigStore.h"
#include "Core/DataStore/DataStore.h"
#include "Core/ModuleManager.h"
#include "Core/ServiceRegistry.h"

/// Load Modules
// Network modules
#include "Modules/Network/WifiModule/WifiModule.h"
#include "Modules/Network/NTPModule/NTPModule.h"
#include "Modules/Network/MQTTModule/MQTTModule.h"
// Stores Modules
#include "Modules/Stores/ConfigStoreModule/ConfigStoreModule.h"
#include "Modules/Stores/DataStoreModule/DataStoreModule.h"
// System Modules
#include "Modules/System/SystemModule/SystemModule.h"
#include "Modules/System/SystemMonitorModule/SystemMonitorModule.h"
// Logs Modules
#include "Modules/Logs/LogHubModule/LogHubModule.h"
#include "Modules/Logs/LogSerialSinkModule/LogSerialSinkModule.h"
#include "Modules/Logs/LogDispatcherModule/LogDispatcherModule.h"

#include "Modules/IOModule/IOModule.h"
#include "Modules/IOModule/IOBus/OneWireBus.h"
#include "Modules/EventBusModule/EventBusModule.h"
#include "Modules/CommandModule/CommandModule.h"

#include "Modules/IOModule/IORuntime.h"
#include "Core/SystemStats.h"
#include <WiFi.h>

/// Only necessary services (global)
#include "Core/Services/iLogger.h"

static Preferences preferences;
static ConfigStore registry;

static ModuleManager moduleManager;
static ServiceRegistry services;

///static LoggerModule loggerModule;
static WifiModule           wifiModule;
static NTPModule            ntpModule;
static CommandModule        commandModule;
static ConfigStoreModule    configStoreModule;
static DataStoreModule      dataStoreModule;
static MQTTModule           mqttModule;
static SystemModule         systemModule;
static SystemMonitorModule  systemMonitorModule;
static LogSerialSinkModule  logSerialSinkModule;
static LogDispatcherModule  logDispatcherModule;
static LogHubModule         logHubModule;
static EventBusModule       eventBusModule;
static IOModule             ioModule;

static OneWireBus oneWireWater(19);
static OneWireBus oneWireAir(18);

static char topicSensorsState[128] = {0};
static char topicNetworkState[128] = {0};
static char topicSystemState[128] = {0};

static bool buildSensorsSnapshot(MQTTModule* mqtt, char* out, size_t len) {
    if (!mqtt) return false;
    DataStore* ds = mqtt->dataStorePtr();
    if (!ds) return false;

    float ph = ioPh(*ds);
    float orp = ioOrp(*ds);
    float psi = ioPsi(*ds);
    float w = ioWaterTemp(*ds);
    float a = ioAirTemp(*ds);

    snprintf(out, len,
             "{\"ph\":%.3f,\"orp\":%.3f,\"psi\":%.3f,\"waterTemp\":%.2f,\"airTemp\":%.2f,\"ts\":%lu}",
             ph, orp, psi, w, a, (unsigned long)millis());
    return true;
}

static bool buildNetworkSnapshot(MQTTModule* mqtt, char* out, size_t len) {
    if (!mqtt) return false;
    DataStore* ds = mqtt->dataStorePtr();
    if (!ds) return false;

    IpV4 ip4 = wifiIp(*ds);
    char ip[16];
    snprintf(ip, sizeof(ip), "%u.%u.%u.%u", ip4.b[0], ip4.b[1], ip4.b[2], ip4.b[3]);
    bool netReady = wifiReady(*ds);
    bool mqttOk = mqttReady(*ds);
    int rssi = (WiFi.isConnected()) ? WiFi.RSSI() : -127;

    snprintf(out, len,
             "{\"ready\":%s,\"ip\":\"%s\",\"rssi\":%d,\"mqtt\":%s,\"ts\":%lu}",
             netReady ? "true" : "false",
             ip,
             rssi,
             mqttOk ? "true" : "false",
             (unsigned long)millis());
    return true;
}

static bool buildSystemSnapshot(MQTTModule* mqtt, char* out, size_t len) {
    if (!mqtt) return false;

    SystemStatsSnapshot snap{};
    SystemStats::collect(snap);

    snprintf(out, len,
             "{\"upt_ms\":%lu,\"heap\":{\"free\":%lu,\"min\":%lu,\"largest\":%lu,\"frag\":%u},\"ts\":%lu}",
             (unsigned long)snap.uptimeMs,
             (unsigned long)snap.heap.freeBytes,
             (unsigned long)snap.heap.minFreeBytes,
             (unsigned long)snap.heap.largestFreeBlock,
             (unsigned int)snap.heap.fragPercent,
             (unsigned long)millis());
    return true;
}

void setup() {
    Serial.begin(115200);
    delay(50);
    preferences.begin("flowio", false);
    registry.setPreferences(preferences);
    registry.runMigrations(CURRENT_CFG_VERSION, steps, MIGRATION_COUNT);

    moduleManager.add(&logHubModule);
    moduleManager.add(&logDispatcherModule);
    moduleManager.add(&logSerialSinkModule);
    moduleManager.add(&eventBusModule);

    moduleManager.add(&configStoreModule);
    moduleManager.add(&dataStoreModule);
    moduleManager.add(&commandModule);
    moduleManager.add(&wifiModule);
    moduleManager.add(&ntpModule);
    moduleManager.add(&mqttModule);
    moduleManager.add(&systemModule);
    moduleManager.add(&ioModule);

    systemMonitorModule.setModuleManager(&moduleManager);
    moduleManager.add(&systemMonitorModule);

    ioModule.setOneWireBuses(&oneWireWater, &oneWireAir);

    IOAnalogDefinition phDef{};
    snprintf(phDef.id, sizeof(phDef.id), "ph");
    phDef.source = IO_SRC_ADS_INTERNAL_SINGLE;
    phDef.channel = 0;
    phDef.precision = 1;
    ioModule.defineAnalogInput(phDef);

    IOAnalogDefinition orpDef{};
    snprintf(orpDef.id, sizeof(orpDef.id), "orp");
    orpDef.source = IO_SRC_ADS_INTERNAL_SINGLE;
    orpDef.channel = 1;
    orpDef.precision = 0;
    ioModule.defineAnalogInput(orpDef);

    IOAnalogDefinition psiDef{};
    snprintf(psiDef.id, sizeof(psiDef.id), "psi");
    psiDef.source = IO_SRC_ADS_INTERNAL_SINGLE;
    psiDef.channel = 2;
    psiDef.precision = 1;
    ioModule.defineAnalogInput(psiDef);

    IOAnalogDefinition waterDef{};
    snprintf(waterDef.id, sizeof(waterDef.id), "water_temp");
    waterDef.source = IO_SRC_DS18_WATER;
    waterDef.channel = 0;
    waterDef.precision = 1;
    waterDef.minValid = -55.0f;
    waterDef.maxValid = 125.0f;
    ioModule.defineAnalogInput(waterDef);

    IOAnalogDefinition airDef{};
    snprintf(airDef.id, sizeof(airDef.id), "air_temp");
    airDef.source = IO_SRC_DS18_AIR;
    airDef.channel = 0;
    airDef.precision = 1;
    airDef.minValid = -55.0f;
    airDef.maxValid = 125.0f;
    ioModule.defineAnalogInput(airDef);

    
    bool ok = moduleManager.initAll(registry, services);
    if (!ok) {
        while (true) delay(1000);
    }

    mqttModule.formatTopic(topicSensorsState, sizeof(topicSensorsState), "rt/sensors/state");
    mqttModule.formatTopic(topicNetworkState, sizeof(topicNetworkState), "rt/network/state");
    mqttModule.formatTopic(topicSystemState, sizeof(topicSystemState), "rt/system/state");
    mqttModule.setSensorsPublisher(topicSensorsState, buildSensorsSnapshot);
    mqttModule.addRuntimePublisher(topicSensorsState, 60000, 0, false, buildSensorsSnapshot);
    mqttModule.addRuntimePublisher(topicNetworkState, 60000, 0, false, buildNetworkSnapshot);
    mqttModule.addRuntimePublisher(topicSystemState, 60000, 0, false, buildSystemSnapshot);

    Serial.print(
        "\x1b[34m"
        " _____  _                   ___  ___          \n"
        "|  ___|| |  ___ __      __ |_ _|/ _ \\         \n"
        "| |_   | | / _ \\\\ \\ /\\ / /  | || | | |  _____ \n"
        "|  _|  | || (_) |\\ V  V /_  | || |_| | |_____|\n"
        "|_|    |_| \\___/  \\_/\\_/(_)|___|\\___/         \n"
        "\x1b[0m"
    );

}

void loop() {
}
