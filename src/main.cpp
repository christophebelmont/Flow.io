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
#include "Core/I2CManager.h"

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

#include "Modules/Sensors/SensorsModule.h"
#include "Modules/Actuators/ActuatorsModule.h"
#include "Modules/EventBusModule/EventBusModule.h"
#include "Modules/CommandModule/CommandModule.h"

#include "Modules/Sensors/SensorsRuntime.h"
#include "Modules/Actuators/ActuatorsRuntime.h"
#include "Core/SystemStats.h"
#include <WiFi.h>

/// Only necessary services (global)
#include "Core/Services/iLogger.h"

static Preferences preferences;
static ConfigStore registry;

static ModuleManager moduleManager;
static ServiceRegistry services;
static I2CManager i2c;

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
static SensorsModule        sensorsModule;
static ActuatorsModule      actuatorsModule;

static OneWireBus oneWireWater(19);
static OneWireBus oneWireAir(18);
static ADS1115 adsPrimary(0x48, &Wire);
static ADS1115 adsSecondary(0x49, &Wire);

static char topicSensorsState[128] = {0};
static char topicNetworkState[128] = {0};
static char topicSystemState[128] = {0};
static char topicActuatorsState[128] = {0};

static bool buildSensorsSnapshot(MQTTModule* mqtt, char* out, size_t len) {
    if (!mqtt) return false;
    DataStore* ds = mqtt->dataStorePtr();
    if (!ds) return false;

    float ph = sensorsPh(*ds);
    float orp = sensorsOrp(*ds);
    float psi = sensorsPsi(*ds);
    float w = sensorsWaterTemp(*ds);
    float a = sensorsAirTemp(*ds);

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

static bool buildActuatorsSnapshot(MQTTModule* mqtt, char* out, size_t len) {
    if (!mqtt) return false;
    DataStore* ds = mqtt->dataStorePtr();
    if (!ds) return false;

    size_t used = 0;
    int wrote = snprintf(out, len, "{\"slots\":[");
    if (wrote < 0 || (size_t)wrote >= len) return false;
    used += (size_t)wrote;

    for (uint8_t i = 0; i < ACTUATOR_MAX; ++i) {
        ActuatorRuntime rt = actuatorRuntime(*ds, i);
        wrote = snprintf(out + used, len - used,
                         "%s{\"on\":%s,\"cmd\":%s,\"up\":%lu,\"tank\":%.2f}",
                         (i == 0 ? "" : ","),
                         rt.on ? "true" : "false",
                         rt.commanded ? "true" : "false",
                         (unsigned long)rt.uptimeMs,
                         rt.tankFillPct);
        if (wrote < 0 || (size_t)wrote >= (len - used)) return false;
        used += (size_t)wrote;
    }

    wrote = snprintf(out + used, len - used, "],\"ts\":%lu}", (unsigned long)millis());
    if (wrote < 0 || (size_t)wrote >= (len - used)) return false;
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
    moduleManager.add(&sensorsModule);
    moduleManager.add(&actuatorsModule);

    systemMonitorModule.setModuleManager(&moduleManager);
    moduleManager.add(&systemMonitorModule);

    sensorsModule.setOneWireBuses(&oneWireWater, &oneWireAir);
    sensorsModule.setAdsDevices(&adsPrimary, &adsSecondary);

    actuatorsModule.configureSlot(0, "filtration_pump", 32, ActuatorKind::Pump);
    actuatorsModule.configureSlot(1, "ph_injection_pump", 25, ActuatorKind::Pump);
    actuatorsModule.configureSlot(2, "chl_injection_pump", 26, ActuatorKind::Pump);
    actuatorsModule.configureSlot(3, "electrolyzer_relay", 13, ActuatorKind::Relay, true, 400, 1000);
    actuatorsModule.configureSlot(4, "pool_light", 27, ActuatorKind::Light);

    
    bool ok = moduleManager.initAll(registry, i2c, services);
    if (!ok) {
        while (true) delay(1000);
    }

    mqttModule.formatTopic(topicSensorsState, sizeof(topicSensorsState), "rt/sensors/state");
    mqttModule.formatTopic(topicNetworkState, sizeof(topicNetworkState), "rt/network/state");
    mqttModule.formatTopic(topicSystemState, sizeof(topicSystemState), "rt/system/state");
    mqttModule.formatTopic(topicActuatorsState, sizeof(topicActuatorsState), "rt/actuators/state");
    mqttModule.setSensorsPublisher(topicSensorsState, buildSensorsSnapshot);
    mqttModule.addRuntimePublisher(topicSensorsState, 60000, 0, false, buildSensorsSnapshot);
    mqttModule.addRuntimePublisher(topicNetworkState, 60000, 0, false, buildNetworkSnapshot);
    mqttModule.addRuntimePublisher(topicSystemState, 60000, 0, false, buildSystemSnapshot);
    mqttModule.addRuntimePublisher(topicActuatorsState, 15000, 0, false, buildActuatorsSnapshot);

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
