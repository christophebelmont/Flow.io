/**
 * @file MQTTModule.cpp
 * @brief Implementation file.
 */
#include "MQTTModule.h"
#include "Core/Runtime.h"
#include "Core/SystemStats.h"
#include "Modules/Sensors/SensorsRuntime.h"
#include <WiFi.h>
#include <esp_system.h>
#include "Core/EventBus/EventPayloads.h"
#define LOG_TAG "MqttModu"
#include "Core/ModuleLog.h"

static uint32_t clampU32(uint32_t v, uint32_t minV, uint32_t maxV) {
    if (v < minV) return minV;
    if (v > maxV) return maxV;
    return v;
}

static uint32_t jitterMs(uint32_t baseMs, uint8_t pct) {
    if (baseMs == 0 || pct == 0) return baseMs;
    uint32_t span = (baseMs * pct) / 100U;
    uint32_t r = esp_random();
    uint32_t delta = r % (2U * span + 1U);
    int32_t signedDelta = (int32_t)delta - (int32_t)span;
    int32_t out = (int32_t)baseMs + signedDelta;
    if (out < 0) out = 0;
    return (uint32_t)out;
}


void MQTTModule::setState(MQTTState s) {
    state = s;
    stateTs = millis();
    if (dataStore) {
        setMqttReady(*dataStore, s == MQTTState::Connected);
    }
}

static void makeDeviceId(char* out, size_t len) {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(out, len, "ESP32-%02X%02X%02X", mac[3], mac[4], mac[5]);
}

void MQTTModule::buildTopics() {
    snprintf(topicCmd, sizeof(topicCmd), "%s/%s/cmd", cfgData.baseTopic, deviceId);
    snprintf(topicAck, sizeof(topicAck), "%s/%s/ack", cfgData.baseTopic, deviceId);
    snprintf(topicStatus, sizeof(topicStatus), "%s/%s/status", cfgData.baseTopic, deviceId);
    snprintf(topicCfgSet, sizeof(topicCfgSet), "%s/%s/cfg/set", cfgData.baseTopic, deviceId);
    snprintf(topicCfgAck, sizeof(topicCfgAck), "%s/%s/cfg/ack", cfgData.baseTopic, deviceId);
    snprintf(topicSensorsBase, sizeof(topicSensorsBase), "%s/%s/rt/sensors", cfgData.baseTopic, deviceId);
    snprintf(topicSensorsState, sizeof(topicSensorsState), "%s/%s/rt/sensors/state", cfgData.baseTopic, deviceId);
    snprintf(topicSensorsPh, sizeof(topicSensorsPh), "%s/%s/rt/sensors/ph", cfgData.baseTopic, deviceId);
    snprintf(topicSensorsOrp, sizeof(topicSensorsOrp), "%s/%s/rt/sensors/orp", cfgData.baseTopic, deviceId);
    snprintf(topicSensorsPsi, sizeof(topicSensorsPsi), "%s/%s/rt/sensors/psi", cfgData.baseTopic, deviceId);
    snprintf(topicSensorsWater, sizeof(topicSensorsWater), "%s/%s/rt/sensors/temp/water", cfgData.baseTopic, deviceId);
    snprintf(topicSensorsAir, sizeof(topicSensorsAir), "%s/%s/rt/sensors/temp/air", cfgData.baseTopic, deviceId);
    snprintf(topicNetworkState, sizeof(topicNetworkState), "%s/%s/rt/network/state", cfgData.baseTopic, deviceId);
    snprintf(topicSystemState, sizeof(topicSystemState), "%s/%s/rt/system/state", cfgData.baseTopic, deviceId);
    for (size_t i = 0; i < cfgModuleCount; ++i) {
        snprintf(topicCfgBlocks[i], sizeof(topicCfgBlocks[i]),
                 "%s/%s/cfg/%s", cfgData.baseTopic, deviceId, cfgModules[i]);
    }
}

void MQTTModule::connectMqtt() {
    buildTopics();
    client.setServer(cfgData.host, (uint16_t)cfgData.port);
    if (cfgData.user[0] != '\0') client.setCredentials(cfgData.user, cfgData.pass);
    client.setWill(topicStatus, 1, true, "{\"online\":false}");
    client.connect();
    setState(MQTTState::Connecting);
    LOGI("Connecting to %s:%ld", cfgData.host, cfgData.port);
}

void MQTTModule::onConnect(bool) {
    LOGI("Connected subscribe %s", topicCmd);
    client.subscribe(topicCmd, 0);
    client.subscribe(topicCfgSet, 1);
    client.publish(topicStatus, 1, true, "{\"online\":true}");
    publishConfigBlocks(true);
    publishSensors(true);

    _retryCount = 0;
    _retryDelayMs = 2000;
    setState(MQTTState::Connected);
}

void MQTTModule::onDisconnect(AsyncMqttClientDisconnectReason) {
    LOGW("Disconnected");
    setState(MQTTState::ErrorWait);
}

void MQTTModule::onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties,
                           size_t len, size_t, size_t total) {
    if (!rxQ) return;
    if (len != total) return;

    RxMsg m{};
    strncpy(m.topic, topic, sizeof(m.topic) - 1);
    size_t copyLen = len;
    if (copyLen >= sizeof(m.payload)) copyLen = sizeof(m.payload) - 1;
    memcpy(m.payload, payload, copyLen);
    m.payload[copyLen] = '\0';

    xQueueSend(rxQ, &m, 0);
}

const char* MQTTModule::findJsonStringValue(const char* json, const char* key) {
    static char pat[48];
    snprintf(pat, sizeof(pat), "\"%s\":\"", key);
    const char* p = strstr(json, pat);
    if (!p) return nullptr;
    return p + strlen(pat);
}

const char* MQTTModule::findJsonObjectStart(const char* json, const char* key) {
    static char pat[48];
    snprintf(pat, sizeof(pat), "\"%s\":{", key);
    const char* p = strstr(json, pat);
    if (!p) return nullptr;
    return p + strlen(pat) - 1;
}

void MQTTModule::publishConfigBlocks(bool retained) {
    if (!cfgSvc || !cfgSvc->toJsonModule) return;
    for (size_t i = 0; i < cfgModuleCount; ++i) {
        bool truncated = false;
        bool any = cfgSvc->toJsonModule(cfgSvc->ctx, cfgModules[i], stateCfgBuf, sizeof(stateCfgBuf), &truncated);
        if (!any) continue;
        if (truncated) {
            LOGW("cfg/%s truncated (buffer=%u)", cfgModules[i], (unsigned)sizeof(stateCfgBuf));
        }
        client.publish(topicCfgBlocks[i], 1, retained, stateCfgBuf);
    }
}

void MQTTModule::publishNetwork()
{
    if (!dataStore) return;
    if (state != MQTTState::Connected) return;

    char ip[16] = {0};
    IpV4 ip4 = wifiIp(*dataStore);
    snprintf(ip, sizeof(ip), "%u.%u.%u.%u", ip4.b[0], ip4.b[1], ip4.b[2], ip4.b[3]);

    bool netReady = wifiReady(*dataStore);
    bool mqttOk = mqttReady(*dataStore);
    int rssi = (WiFi.isConnected()) ? WiFi.RSSI() : -127;

    snprintf(networkBuf, sizeof(networkBuf),
             "{\"ready\":%s,\"ip\":\"%s\",\"rssi\":%d,\"mqtt\":%s,\"ts\":%lu}",
             netReady ? "true" : "false",
             ip,
             rssi,
             mqttOk ? "true" : "false",
             (unsigned long)millis());

    client.publish(topicNetworkState, 0, false, networkBuf);
}

void MQTTModule::publishSystem()
{
    if (state != MQTTState::Connected) return;

    SystemStatsSnapshot snap{};
    SystemStats::collect(snap);

    snprintf(systemBuf, sizeof(systemBuf),
             "{\"upt_ms\":%lu,\"heap\":{\"free\":%lu,\"min\":%lu,\"largest\":%lu,\"frag\":%u},\"ts\":%lu}",
             (unsigned long)snap.uptimeMs,
             (unsigned long)snap.heap.freeBytes,
             (unsigned long)snap.heap.minFreeBytes,
             (unsigned long)snap.heap.largestFreeBlock,
             (unsigned int)snap.heap.fragPercent,
             (unsigned long)millis());

    client.publish(topicSystemState, 0, false, systemBuf);
}

bool MQTTModule::publishSensors(bool force)
{
    if (!cfgData.sensorsPublish) return false;
    if (!dataStore) return false;
    if (state != MQTTState::Connected) return false;

    uint32_t now = millis();
    uint32_t minMs = (cfgData.sensorsMinPeriodMs < 0) ? 0 : (uint32_t)cfgData.sensorsMinPeriodMs;
    if (!force && (uint32_t)(now - _lastSensorsPublishMs) < minMs) return false;
    _lastSensorsPublishMs = now;

    int qos = cfgData.sensorsQos;
    if (qos < 0) qos = 0;
    if (qos > 2) qos = 2;
    bool retain = cfgData.sensorsRetain;

    float ph = sensorsPh(*dataStore);
    float orp = sensorsOrp(*dataStore);
    float psi = sensorsPsi(*dataStore);
    float w = sensorsWaterTemp(*dataStore);
    float a = sensorsAirTemp(*dataStore);

    char valBuf[24] = {0};
    snprintf(valBuf, sizeof(valBuf), "%.3f", ph);
    (void)valBuf;

    snprintf(sensorsBuf, sizeof(sensorsBuf),
             "{\"ph\":%.3f,\"orp\":%.3f,\"psi\":%.3f,\"waterTemp\":%.2f,\"airTemp\":%.2f,\"ts\":%lu}",
             ph, orp, psi, w, a, (unsigned long)now);
    client.publish(topicSensorsState, qos, retain, sensorsBuf);
    return true;
}
void MQTTModule::processRx(const RxMsg& msg) {
    if (strcmp(msg.topic, topicCmd) == 0) {
        const char* cmdVal = findJsonStringValue(msg.payload, "cmd");
        if (!cmdVal || !cmdSvc) return;

        const char* cmdEnd = strchr(cmdVal, '"');
        if (!cmdEnd) return;

        char cmd[64];
        size_t clen = (size_t)(cmdEnd - cmdVal);
        if (clen >= sizeof(cmd)) clen = sizeof(cmd) - 1;
        memcpy(cmd, cmdVal, clen); cmd[clen] = '\0';

        const char* argsObj = findJsonObjectStart(msg.payload, "args");

        bool ok = cmdSvc->execute(cmdSvc->ctx, cmd, msg.payload, argsObj, replyBuf, sizeof(replyBuf));
        snprintf(ackBuf, sizeof(ackBuf), "{\"ok\":%s,\"cmd\":\"%s\",\"reply\":%s}",
                 ok ? "true" : "false", cmd, replyBuf);
        client.publish(topicAck, 0, false, ackBuf);
        return;
    }

    if (strcmp(msg.topic, topicCfgSet) == 0) {
        if (!cfgSvc || !cfgSvc->applyJson) return;
        bool ok = cfgSvc->applyJson(cfgSvc->ctx, msg.payload);
        snprintf(ackBuf, sizeof(ackBuf), "{\"ok\":%s}", ok ? "true" : "false");
        client.publish(topicCfgAck, 1, false, ackBuf);

        if (ok) publishConfigBlocks(true);
        return;
    }
}

void MQTTModule::init(ConfigStore& cfg, I2CManager&, ServiceRegistry& services) {
    cfg.registerVar(hostVar);
    cfg.registerVar(portVar);
    cfg.registerVar(userVar);
    cfg.registerVar(passVar);
    cfg.registerVar(baseTopicVar);
    cfg.registerVar(enabledVar);
    cfg.registerVar(sensorsPubVar);
    cfg.registerVar(sensorsRetainVar);
    cfg.registerVar(sensorsQosVar);
    cfg.registerVar(sensorsMinVar);

    wifiSvc = services.get<WifiService>("wifi");
    cmdSvc = services.get<CommandService>("cmd");
    cfgSvc = services.get<ConfigStoreService>("config");
    logHub = services.get<LogHubService>("loghub");

    auto* ebSvc = services.get<EventBusService>("eventbus");
    eventBus = ebSvc ? ebSvc->bus : nullptr;

    const DataStoreService* dsSvc = services.get<DataStoreService>("datastore");
    dataStore = dsSvc ? dsSvc->store : nullptr;

    if (eventBus) {
        eventBus->subscribe(EventId::DataChanged, &MQTTModule::onEventStatic, this);
        eventBus->subscribe(EventId::ConfigChanged, &MQTTModule::onEventStatic, this);
        eventBus->subscribe(EventId::DataSnapshotAvailable, &MQTTModule::onEventStatic, this);
    }

    makeDeviceId(deviceId, sizeof(deviceId));
    buildTopics();

    rxQ = xQueueCreate(8, sizeof(RxMsg));

    client.onConnect([this](bool sp){ this->onConnect(sp); });
    client.onDisconnect([this](AsyncMqttClientDisconnectReason r){ this->onDisconnect(r); });
    client.onMessage([this](char* t, char* p, AsyncMqttClientMessageProperties pr, size_t l, size_t i, size_t tot){
        this->onMessage(t, p, pr, l, i, tot);
    });

    if (cfgSvc && cfgSvc->listModules) {
        cfgModuleCount = cfgSvc->listModules(cfgSvc->ctx, cfgModules, CFG_TOPIC_MAX);
    } else {
        cfgModuleCount = 0;
    }

    LOGI("Init id=%s topic=%s cfgModules=%u", deviceId, topicCmd, (unsigned)cfgModuleCount);

    _netReady = dataStore ? wifiReady(*dataStore) : false;
    _netReadyTs = millis();
    _retryCount = 0;
    _retryDelayMs = 2000;

    setState(cfgData.enabled ? MQTTState::WaitingNetwork : MQTTState::Disabled);
}

void MQTTModule::loop() {
    if (!cfgData.enabled) {
        if (state != MQTTState::Disabled) {
            client.disconnect();
            setState(MQTTState::Disabled);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
        return;
    }

    switch (state) {
    case MQTTState::Disabled: setState(MQTTState::WaitingNetwork); break;
    case MQTTState::WaitingNetwork:
        if (!_netReady) break;
        if (millis() - _netReadyTs >= 2000) connectMqtt();
        break;
    case MQTTState::Connecting:
        if (millis() - stateTs > 10000) {
            LOGW("Connect timeout");
            client.disconnect();
            setState(MQTTState::ErrorWait);
        }
        break;
    case MQTTState::Connected: {
        RxMsg m;
        while (xQueueReceive(rxQ, &m, 0) == pdTRUE) processRx(m);
        if (_pendingSensorsPublish) {
            if (publishSensors(false)) {
                _pendingSensorsPublish = false;
            }
        }
        uint32_t now = millis();
        if (now - _lastNetworkPublishMs >= 60000) {
            _lastNetworkPublishMs = now;
            publishNetwork();
        }
        if (now - _lastSystemPublishMs >= 60000) {
            _lastSystemPublishMs = now;
            publishSystem();
        }
        break;
    }
    case MQTTState::ErrorWait:
        if (!_netReady) {
            setState(MQTTState::WaitingNetwork);
            break;
        }
        if (millis() - stateTs >= _retryDelayMs) {
            _retryCount++;
            uint32_t next = _retryDelayMs;

            if      (next < 5000)   next = 5000;
            else if (next < 10000)  next = 10000;
            else if (next < 30000)  next = 30000;
            else if (next < 60000)  next = 60000;
            else                    next = 300000;

            next = clampU32(next, 2000, 300000);
            _retryDelayMs = jitterMs(next, 15);
            setState(MQTTState::WaitingNetwork);
        }
        break;
    }

    vTaskDelay(pdMS_TO_TICKS(50));
}

void MQTTModule::onEventStatic(const Event& e, void* user)
{
    static_cast<MQTTModule*>(user)->onEvent(e);
}

void MQTTModule::onEvent(const Event& e)
{
    if (e.id == EventId::DataChanged) {
        const DataChangedPayload* p = (const DataChangedPayload*)e.payload;
        if (!p) return;
        if (p->id != DATAKEY_WIFI_READY) return;
        if (!dataStore) return;

        bool ready = wifiReady(*dataStore);
        if (ready == _netReady) return;

        _netReady = ready;
        _netReadyTs = millis();

        if (_netReady) {
            LOGI("DataStore networkReady=true -> warmup");
            if (state != MQTTState::Connected) setState(MQTTState::WaitingNetwork);
        } else {
            LOGI("DataStore networkReady=false -> disconnect and wait");
            client.disconnect();
            setState(MQTTState::WaitingNetwork);
        }
        return;
    }

    if (e.id == EventId::DataSnapshotAvailable) {
        const DataSnapshotPayload* p = (const DataSnapshotPayload*)e.payload;
        if (!p) return;
        if ((p->dirtyFlags & DIRTY_SENSORS) != 0) {
            _pendingSensorsPublish = true;
        }
        return;
    }

    if (e.id == EventId::ConfigChanged) {
        const ConfigChangedPayload* p = (const ConfigChangedPayload*)e.payload;
        if (!p) return;

        const char* key = p->nvsKey;
        if (!key || key[0] == '\0') return;

        if (strcmp(key, "mq_base") == 0 ||
            strcmp(key, "mq_host") == 0 ||
            strcmp(key, "mq_port") == 0 ||
            strcmp(key, "mq_user") == 0 ||
            strcmp(key, "mq_pass") == 0 ||
            strcmp(key, "mq_sen") == 0 ||
            strcmp(key, "mq_sret") == 0 ||
            strcmp(key, "mq_sqos") == 0 ||
            strcmp(key, "mq_smin") == 0)
        {
            LOGI("MQTT config changed (%s) -> reconnect", key);
            client.disconnect();
            _netReadyTs = millis();
            setState(MQTTState::WaitingNetwork);
        }
        return;
    }
}
