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
#include "Modules/EventBusModule/EventBusModule.h"
#include "Modules/CommandModule/CommandModule.h"

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

    systemMonitorModule.setModuleManager(&moduleManager);
    moduleManager.add(&systemMonitorModule);

    
    bool ok = moduleManager.initAll(registry, i2c, services);
    if (!ok) {
        while (true) delay(1000);
    }

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
