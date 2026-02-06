#pragma once
/**
 * @file MQTTModule.h
 * @brief MQTT client module.
 */
#include "Core/Module.h"
#include "Core/Services/Services.h"
#include <AsyncMqttClient.h>

/** @brief MQTT configuration values. */
struct MQTTConfig {
    bool enabled = true;
    char host[64] = "192.168.86.250";
    int32_t port = 1883;
    char user[32] = "";
    char pass[32] = "";
    char baseTopic[64] = "flowio";
};

/** @brief MQTT connection state. */
enum class MQTTState : uint8_t { Disabled, WaitingNetwork, Connecting, Connected, ErrorWait };

/**
 * @brief Active module that manages the MQTT client connection.
 */
class MQTTModule : public Module {
public:
    /** @brief Module id. */
    const char* moduleId() const override { return "mqtt"; }
    /** @brief Task name. */
    const char* taskName() const override { return "mqtt"; }

    /** @brief MQTT depends on log hub and WiFi. */
    uint8_t dependencyCount() const override { return 2; }
    const char* dependency(uint8_t i) const override {
        if (i == 0) return "loghub";
        if (i == 1) return "wifi";
        //if (i == 2) return "cmd";
        return nullptr;
    }

    /** @brief Initialize MQTT config/services. */
    void init(ConfigStore& cfg, I2CManager& i2c, ServiceRegistry& services) override;
    /** @brief MQTT task loop. */
    void loop() override;
    /** @brief Extra stack for MQTT processing. */
    uint16_t taskStackSize() const override { return 4096; }

private:
    MQTTConfig cfgData;
    MQTTState state = MQTTState::WaitingNetwork;
    uint32_t stateTs = 0;

    AsyncMqttClient client;

    const WifiService* wifiSvc = nullptr;
    const CommandService* cmdSvc = nullptr;
    const ConfigStoreService* cfgSvc = nullptr;
    const LogHubService* logHub = nullptr;
    EventBus* eventBus = nullptr;
    DataStore* dataStore = nullptr;

    char deviceId[24] = {0};
    char topicCmd[128] = {0};
    char topicAck[128] = {0};
    char topicStatus[128] = {0};
    char topicCfgSet[128] = {0};
    char topicCfgAck[128] = {0};
    static constexpr size_t CFG_TOPIC_MAX = 16;
    const char* cfgModules[CFG_TOPIC_MAX] = {nullptr};
    uint8_t cfgModuleCount = 0;
    char topicCfgBlocks[CFG_TOPIC_MAX][128] = {{0}};

    struct RxMsg {
        char topic[128];
        char payload[384];
    };
    QueueHandle_t rxQ = nullptr;
    char ackBuf[512] = {0};
    char replyBuf[256] = {0};
    char stateCfgBuf[768] = {0};

    ConfigVariable<char,0> hostVar {
        NVS_KEY("mq_host"),"host","mqtt",ConfigType::CharArray,
        (char*)cfgData.host,ConfigPersistence::Persistent,sizeof(cfgData.host)
    };
    ConfigVariable<int32_t,0> portVar {
        NVS_KEY("mq_port"),"port","mqtt",ConfigType::Int32,
        &cfgData.port,ConfigPersistence::Persistent,0
    };
    ConfigVariable<char,0> userVar {
        NVS_KEY("mq_user"),"user","mqtt",ConfigType::CharArray,
        (char*)cfgData.user,ConfigPersistence::Persistent,sizeof(cfgData.user)
    };
    ConfigVariable<char,0> passVar {
        NVS_KEY("mq_pass"),"pass","mqtt",ConfigType::CharArray,
        (char*)cfgData.pass,ConfigPersistence::Persistent,sizeof(cfgData.pass)
    };
    ConfigVariable<char,0> baseTopicVar {
        NVS_KEY("mq_base"),"baseTopic","mqtt",ConfigType::CharArray,
        (char*)cfgData.baseTopic,ConfigPersistence::Persistent,sizeof(cfgData.baseTopic)
    };
    ConfigVariable<bool,0> enabledVar {
        NVS_KEY("mq_en"),"enabled","mqtt",ConfigType::Bool,
        &cfgData.enabled,ConfigPersistence::Persistent,0
    };

    void setState(MQTTState s);
    void buildTopics();
    void connectMqtt();
    void processRx(const RxMsg& msg);
    void publishConfigBlocks(bool retained);

    const char* findJsonStringValue(const char* json, const char* key);
    const char* findJsonObjectStart(const char* json, const char* key);

    void onConnect(bool sessionPresent);
    void onDisconnect(AsyncMqttClientDisconnectReason reason);
    void onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties,
                   size_t len, size_t index, size_t total);

    static void onEventStatic(const Event& e, void* user);
    void onEvent(const Event& e);

    // ---- network warmup ----
    bool _netReady = false;
    uint32_t _netReadyTs = 0;

    // ---- retry backoff ----
    uint8_t _retryCount = 0;
    uint32_t _retryDelayMs = 2000; // 2s start
};
