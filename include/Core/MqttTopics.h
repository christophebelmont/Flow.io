#pragma once
/**
 * @file MqttTopics.h
 * @brief Standard MQTT topic suffixes shared across modules.
 */

namespace MqttTopics {

/** @brief Command ingress suffix (`<base>/<device>/cmd`). */
constexpr char SuffixCmd[] = "cmd";
/** @brief Command/config acknowledgment suffix (`<base>/<device>/ack`). */
constexpr char SuffixAck[] = "ack";
/** @brief Device availability/status suffix  Last Will (`<base>/<device>/status`). */
constexpr char SuffixStatus[] = "status";
/** @brief Config patch ingress suffix (`<base>/<device>/cfg/set`). */
constexpr char SuffixCfgSet[] = "cfg/set";
/** @brief Config acknowledgment suffix (`<base>/<device>/cfg/ack`). */
constexpr char SuffixCfgAck[] = "cfg/ack";

}  // namespace MqttTopics

