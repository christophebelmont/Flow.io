#pragma once

#include "Domain/Pool/PoolIds.h"
#include "Modules/IOModule/IODrivers/Ads1115Driver.h"
#include "Modules/IOModule/IOModuleTypes.h"

namespace Profiles {
namespace Waveshare {
namespace IoLayout {

enum : PhysicalPortId {
    PortAdsInternal0 = 100, // ADS1115 interne, entree single-ended A0.
    PortAdsInternal1 = 101, // ADS1115 interne, entree single-ended A1.
    PortAdsInternal2 = 102, // ADS1115 interne, entree single-ended A2.
    PortAdsInternal3 = 103, // ADS1115 interne, entree single-ended A3.
    PortAdsExternal0 = 110, // ADS1115 externe, paire differentielle 0.
    PortAdsExternal1 = 111, // ADS1115 externe, paire differentielle 1.
    PortOneWire1 = 120, // DS18B20 bus 1.
    PortOneWire2 = 121, // DS18B20 bus 2.
    PortSht40Temp = 130, // SHT40: temperature.
    PortSht40Humidity = 131, // SHT40: humidite.
    PortBmp280Temp = 132, // BMP280: temperature.
    PortBmp280Pressure = 133, // BMP280: pression.
    PortBme680Temp = 134, // BME680: temperature.
    PortBme680Humidity = 135, // BME680: humidite.
    PortBme680Pressure = 136, // BME680: pression.
    PortBme680Gas = 137, // BME680: resistance gaz.
    PortIna226ShuntMv = 138, // INA226: tension shunt (mV).
    PortIna226BusV = 139, // INA226: tension bus (V).
    PortIna226CurrentMa = 140, // INA226: courant (mA).
    PortIna226PowerMw = 141, // INA226: puissance (mW).
    PortIna226LoadV = 142, // INA226: tension charge (V).
    PortDin0 = 200, // DIN0.
    PortDin1 = 201, // DIN1.
    PortDin2 = 202, // DIN2.
    PortDin3 = 203, // DIN3.
    PortDin4 = 204, // DIN4.
    PortDin5 = 205, // DIN5.
    PortDin6 = 206, // DIN6.
    PortDin7 = 207, // DIN7.
    PortExio1 = 300, // TCA9554 sortie bit 0.
    PortExio2 = 301, // TCA9554 sortie bit 1.
    PortExio3 = 302, // TCA9554 sortie bit 2.
    PortExio4 = 303, // TCA9554 sortie bit 3.
    PortExio5 = 304, // TCA9554 sortie bit 4.
    PortExio6 = 305, // TCA9554 sortie bit 5.
    PortExio7 = 306, // TCA9554 sortie bit 6.
    PortExio8 = 307, // TCA9554 sortie bit 7.
    PortMcpOut1 = 400, // MCP23017 sortie bit 0.
    PortMcpOut2 = 401, // MCP23017 sortie bit 1.
    PortMcpOut3 = 402, // MCP23017 sortie bit 2.
    PortMcpOut4 = 403, // MCP23017 sortie bit 3.
    PortMcpOut5 = 404, // MCP23017 sortie bit 4.
    PortMcpOut6 = 405, // MCP23017 sortie bit 5.
    PortMcpOut7 = 406, // MCP23017 sortie bit 6.
    PortMcpOut8 = 407, // MCP23017 sortie bit 7.
    PortMcpOut9 = 408, // MCP23017 sortie bit 8.
    PortMcpOut10 = 409, // MCP23017 sortie bit 9.
    PortMcpOut11 = 410, // MCP23017 sortie bit 10.
    PortMcpOut12 = 411, // MCP23017 sortie bit 11.
    PortMcpOut13 = 412, // MCP23017 sortie bit 12.
    PortMcpOut14 = 413, // MCP23017 sortie bit 13.
    PortMcpOut15 = 414, // MCP23017 sortie bit 14.
    PortMcpOut16 = 415 // MCP23017 sortie bit 15.
};

inline constexpr IOBindingPortSpec kBindingPorts[] = {
    // {portId, kind, param0, param1}
    {PortAdsInternal0, IO_PORT_KIND_ADS_INTERNAL_SINGLE, 0, 0}, // ADS1115 interne canal 0.
    {PortAdsInternal1, IO_PORT_KIND_ADS_INTERNAL_SINGLE, 1, 0}, // ADS1115 interne canal 1.
    {PortAdsInternal2, IO_PORT_KIND_ADS_INTERNAL_SINGLE, 2, 0}, // ADS1115 interne canal 2.
    {PortAdsInternal3, IO_PORT_KIND_ADS_INTERNAL_SINGLE, 3, 0}, // ADS1115 interne canal 3.
    {PortAdsExternal0, IO_PORT_KIND_ADS_EXTERNAL_DIFF, 0, 0}, // ADS1115 externe paire 0.
    {PortAdsExternal1, IO_PORT_KIND_ADS_EXTERNAL_DIFF, 1, 0}, // ADS1115 externe paire 1.
    {PortOneWire1, IO_PORT_KIND_DS18_WATER, 20, 0}, // DS18B20 bus 1 GPIO20.
    {PortOneWire2, IO_PORT_KIND_DS18_AIR, 19, 0}, // DS18B20 bus 2 GPIO19.
    {PortSht40Temp, IO_PORT_KIND_SHT40, 0, 0}, // SHT40 temperature.
    {PortSht40Humidity, IO_PORT_KIND_SHT40, 1, 0}, // SHT40 humidite.
    {PortBmp280Temp, IO_PORT_KIND_BMP280, 0, 0}, // BMP280 temperature.
    {PortBmp280Pressure, IO_PORT_KIND_BMP280, 1, 0}, // BMP280 pression.
    {PortBme680Temp, IO_PORT_KIND_BME680, 0, 0}, // BME680 temperature.
    {PortBme680Humidity, IO_PORT_KIND_BME680, 1, 0}, // BME680 humidite.
    {PortBme680Pressure, IO_PORT_KIND_BME680, 2, 0}, // BME680 pression.
    {PortBme680Gas, IO_PORT_KIND_BME680, 3, 0}, // BME680 gaz.
    {PortIna226ShuntMv, IO_PORT_KIND_INA226, 0, 0}, // INA226 shunt.
    {PortIna226BusV, IO_PORT_KIND_INA226, 1, 0}, // INA226 bus.
    {PortIna226CurrentMa, IO_PORT_KIND_INA226, 2, 0}, // INA226 courant.
    {PortIna226PowerMw, IO_PORT_KIND_INA226, 3, 0}, // INA226 puissance.
    {PortIna226LoadV, IO_PORT_KIND_INA226, 4, 0}, // INA226 tension charge.
#if defined(FLOW_BOARD_WAVESHARE_ESP32_S3)
    {PortDin0, IO_PORT_KIND_GPIO_INPUT, 4, 0}, // DIN0 GPIO4.
    {PortDin1, IO_PORT_KIND_GPIO_INPUT, 5, 0}, // DIN1 GPIO5.
    {PortDin2, IO_PORT_KIND_GPIO_INPUT, 6, 0}, // DIN2 GPIO6.
    {PortDin3, IO_PORT_KIND_GPIO_INPUT, 7, 0}, // DIN3 GPIO7.
    {PortDin4, IO_PORT_KIND_GPIO_INPUT, 8, 0}, // DIN4 GPIO8.
    {PortDin5, IO_PORT_KIND_GPIO_INPUT, 9, 0}, // DIN5 GPIO9.
    {PortDin6, IO_PORT_KIND_GPIO_INPUT, 10, 0}, // DIN6 GPIO10.
    {PortDin7, IO_PORT_KIND_GPIO_INPUT, 11, 0}, // DIN7 GPIO11.
#else
    {PortDin0, IO_PORT_KIND_GPIO_INPUT, 4, 0}, // Entree digitale 1 (GPIO4).
    {PortDin1, IO_PORT_KIND_GPIO_INPUT, 5, 0}, // Entree digitale 2 (GPIO5).
    {PortDin2, IO_PORT_KIND_GPIO_INPUT, 6, 0}, // Entree digitale 3 (GPIO6).
    {PortDin3, IO_PORT_KIND_GPIO_INPUT, 7, 0}, // Entree digitale 4 (GPIO7).
    {PortDin4, IO_PORT_KIND_GPIO_INPUT, 8, 0}, // Entree digitale 5 (GPIO8).
    {PortDin5, IO_PORT_KIND_GPIO_INPUT, 9, 0}, // Entree digitale 6 (GPIO9).
    {PortDin6, IO_PORT_KIND_GPIO_INPUT, 10, 0}, // Entree digitale 7 (GPIO10).
    {PortDin7, IO_PORT_KIND_GPIO_INPUT, 11, 0}, // Entree digitale 8 (GPIO11).
#endif
    {PortExio1, IO_PORT_KIND_TCA9554_OUTPUT, 0, 0}, // TCA9554 bit 0.
    {PortExio2, IO_PORT_KIND_TCA9554_OUTPUT, 1, 0}, // TCA9554 bit 1.
    {PortExio3, IO_PORT_KIND_TCA9554_OUTPUT, 2, 0}, // TCA9554 bit 2.
    {PortExio4, IO_PORT_KIND_TCA9554_OUTPUT, 3, 0}, // TCA9554 bit 3.
    {PortExio5, IO_PORT_KIND_TCA9554_OUTPUT, 4, 0}, // TCA9554 bit 4.
    {PortExio6, IO_PORT_KIND_TCA9554_OUTPUT, 5, 0}, // TCA9554 bit 5.
    {PortExio7, IO_PORT_KIND_TCA9554_OUTPUT, 6, 0}, // TCA9554 bit 6.
    {PortExio8, IO_PORT_KIND_TCA9554_OUTPUT, 7, 0}, // TCA9554 bit 7.
    {PortMcpOut1, IO_PORT_KIND_MCP23017_OUTPUT, 0, 0}, // MCP23017 bit 0.
    {PortMcpOut2, IO_PORT_KIND_MCP23017_OUTPUT, 1, 0}, // MCP23017 bit 1.
    {PortMcpOut3, IO_PORT_KIND_MCP23017_OUTPUT, 2, 0}, // MCP23017 bit 2.
    {PortMcpOut4, IO_PORT_KIND_MCP23017_OUTPUT, 3, 0}, // MCP23017 bit 3.
    {PortMcpOut5, IO_PORT_KIND_MCP23017_OUTPUT, 4, 0}, // MCP23017 bit 4.
    {PortMcpOut6, IO_PORT_KIND_MCP23017_OUTPUT, 5, 0}, // MCP23017 bit 5.
    {PortMcpOut7, IO_PORT_KIND_MCP23017_OUTPUT, 6, 0}, // MCP23017 bit 6.
    {PortMcpOut8, IO_PORT_KIND_MCP23017_OUTPUT, 7, 0}, // MCP23017 bit 7.
    {PortMcpOut9, IO_PORT_KIND_MCP23017_OUTPUT, 8, 0}, // MCP23017 bit 8.
    {PortMcpOut10, IO_PORT_KIND_MCP23017_OUTPUT, 9, 0}, // MCP23017 bit 9.
    {PortMcpOut11, IO_PORT_KIND_MCP23017_OUTPUT, 10, 0}, // MCP23017 bit 10.
    {PortMcpOut12, IO_PORT_KIND_MCP23017_OUTPUT, 11, 0}, // MCP23017 bit 11.
    {PortMcpOut13, IO_PORT_KIND_MCP23017_OUTPUT, 12, 0}, // MCP23017 bit 12.
    {PortMcpOut14, IO_PORT_KIND_MCP23017_OUTPUT, 13, 0}, // MCP23017 bit 13.
    {PortMcpOut15, IO_PORT_KIND_MCP23017_OUTPUT, 14, 0}, // MCP23017 bit 14.
    {PortMcpOut16, IO_PORT_KIND_MCP23017_OUTPUT, 15, 0}, // MCP23017 bit 15.
};

constexpr PhysicalPortId analogPortFromLegacy(uint8_t source, uint8_t channel)
{
    switch (source) {
        case IO_SRC_ADS_INTERNAL_SINGLE:
            return (channel == 0U) ? PortAdsInternal0 :
                   (channel == 1U) ? PortAdsInternal1 :
                   (channel == 2U) ? PortAdsInternal2 :
                                     PortAdsInternal3;
        case IO_SRC_ADS_EXTERNAL_DIFF:
            return (channel == 0U) ? PortAdsExternal0 : PortAdsExternal1;
        case IO_SRC_DS18_WATER:
            return PortOneWire1;
        case IO_SRC_DS18_AIR:
            return PortOneWire2;
        default:
            return IO_PORT_INVALID;
    }
}

struct AnalogRoleDefault {
    DomainSlotId domainSlot; // Besoin fonctionnel de la sonde.
    PhysicalPortId bindingPort; // Port physique associe.
    float c0; // Coefficient de calibration offset/intercept.
    float c1; // Coefficient de calibration gain/slope.
    int32_t precision; // Precision d'affichage (nb de decimales).
};

inline constexpr AnalogRoleDefault kAnalogRoleDefaults[] = {
    // {domainSlot, bindingPort, c0, c1, precision}
    {PoolIds::SensorOrp, analogPortFromLegacy(FLOW_WIRDEF_IO_A0S, FLOW_WIRDEF_IO_A0C), FLOW_WIRDEF_IO_A00, FLOW_WIRDEF_IO_A01, FLOW_WIRDEF_IO_A0P}, // ORP.
    {PoolIds::SensorPh, analogPortFromLegacy(FLOW_WIRDEF_IO_A1S, FLOW_WIRDEF_IO_A1C), FLOW_WIRDEF_IO_A10, FLOW_WIRDEF_IO_A11, FLOW_WIRDEF_IO_A1P}, // pH.
    {PoolIds::SensorPsi, analogPortFromLegacy(FLOW_WIRDEF_IO_A2S, FLOW_WIRDEF_IO_A2C), FLOW_WIRDEF_IO_A20, FLOW_WIRDEF_IO_A21, FLOW_WIRDEF_IO_A2P}, // Pression.
    {PoolIds::SensorSpareAnalog, analogPortFromLegacy(FLOW_WIRDEF_IO_A3S, FLOW_WIRDEF_IO_A3C), FLOW_WIRDEF_IO_A30, FLOW_WIRDEF_IO_A31, FLOW_WIRDEF_IO_A3P}, // Entree analogique reservee.
    {PoolIds::SensorWaterTemp, analogPortFromLegacy(FLOW_WIRDEF_IO_A4S, FLOW_WIRDEF_IO_A4C), FLOW_WIRDEF_IO_A40, FLOW_WIRDEF_IO_A41, FLOW_WIRDEF_IO_A4P}, // Temperature eau.
    {PoolIds::SensorAirTemp, analogPortFromLegacy(FLOW_WIRDEF_IO_A5S, FLOW_WIRDEF_IO_A5C), FLOW_WIRDEF_IO_A50, FLOW_WIRDEF_IO_A51, FLOW_WIRDEF_IO_A5P}, // Temperature air.
};

struct DigitalInputRoleDefault {
    DomainSlotId domainSlot; // Besoin fonctionnel de l'entree.
    PhysicalPortId bindingPort; // Port physique associe.
    uint8_t mode; // Mode de lecture (etat/counter).
    uint8_t edgeMode; // Type de front pris en compte.
    uint32_t debounceUs; // Debounce en microsecondes.
};

inline constexpr DigitalInputRoleDefault kDigitalInputRoleDefaults[] = {
    // {role, bindingPort, mode, edgeMode, debounceUs}
#if defined(FLOW_BOARD_WAVESHARE_ESP32_S3)
    {PoolIds::SensorPoolLevel, PortDin2, IO_DIGITAL_INPUT_STATE, IO_EDGE_RISING, 0U}, // Capteur niveau piscine (GPIO6).
    {PoolIds::SensorPhLevel, PortDin0, IO_DIGITAL_INPUT_STATE, IO_EDGE_RISING, 0U}, // Capteur niveau pH (GPIO4).
    {PoolIds::SensorChlorineLevel, PortDin1, IO_DIGITAL_INPUT_STATE, IO_EDGE_RISING, 0U}, // Capteur niveau desinfectant (GPIO5).
    {PoolIds::SensorWaterCounter, PortDin3, IO_DIGITAL_INPUT_COUNTER, IO_EDGE_RISING, 100000U}, // Compteur impulsions eau (GPIO7, 100 ms debounce).
#else
    {PoolIds::SensorPoolLevel, PortDin0, IO_DIGITAL_INPUT_STATE, IO_EDGE_RISING, 0U}, // Capteur niveau piscine.
    {PoolIds::SensorPhLevel, PortDin1, IO_DIGITAL_INPUT_STATE, IO_EDGE_RISING, 0U}, // Capteur niveau pH.
    {PoolIds::SensorChlorineLevel, PortDin2, IO_DIGITAL_INPUT_STATE, IO_EDGE_RISING, 0U}, // Capteur niveau chlore.
    {PoolIds::SensorWaterCounter, PortDin3, IO_DIGITAL_INPUT_COUNTER, IO_EDGE_RISING, 100000U}, // Compteur impulsions eau (100 ms debounce).
#endif
};

struct DigitalOutputRoleDefault {
    DomainSlotId domainSlot; // Besoin fonctionnel de la sortie.
    PhysicalPortId bindingPort; // Port physique associe.
    bool activeHigh; // Polarite de commande logique.
    bool retainOnWarmReboot; // Conserve le latch expander sur reboot ESP32 chaud.
    bool momentary; // True si sortie impulsionnelle.
    uint16_t pulseMs; // Duree d'impulsion en ms.
};

inline constexpr DigitalOutputRoleDefault kDigitalOutputRoleDefaults[] = {
    // {domainSlot, bindingPort, activeHigh, retainOnWarmReboot, momentary, pulseMs}
    {PoolIds::ActuatorFiltrationPump, PortExio1, true, true, false, 0U}, // Pompe filtration.
    {PoolIds::ActuatorPhPump, PortExio2, true, false, false, 0U}, // Pompe pH.
    {PoolIds::ActuatorChlorinePump, PortExio3, true, false, false, 0U}, // Pompe chlore.
    {PoolIds::ActuatorRobot, PortExio4, true, false, false, 0U}, // Robot.
    {PoolIds::ActuatorFillPump, PortExio5, true, false, false, 0U}, // Pompe de remplissage.
    {PoolIds::ActuatorChlorineGenerator, PortExio6, true, false, false, 0U}, // Electrolyseur.
    {PoolIds::ActuatorLights, PortExio7, true, false, false, 0U}, // Eclairage.
    {PoolIds::ActuatorWaterHeater, PortExio8, true, false, false, 0U}, // Chauffage.
};

inline constexpr const AnalogRoleDefault* analogDefaultForDomainSlot(DomainSlotId domainSlot)
{
    for (const AnalogRoleDefault& entry : kAnalogRoleDefaults) {
        if (entry.domainSlot == domainSlot) return &entry;
    }
    return nullptr;
}

inline constexpr const DigitalInputRoleDefault* digitalInputDefaultForDomainSlot(DomainSlotId domainSlot)
{
    for (const DigitalInputRoleDefault& entry : kDigitalInputRoleDefaults) {
        if (entry.domainSlot == domainSlot) return &entry;
    }
    return nullptr;
}

inline constexpr const DigitalOutputRoleDefault* digitalOutputDefaultForDomainSlot(DomainSlotId domainSlot)
{
    for (const DigitalOutputRoleDefault& entry : kDigitalOutputRoleDefaults) {
        if (entry.domainSlot == domainSlot) return &entry;
    }
    return nullptr;
}

}  // namespace IoLayout
}  // namespace Waveshare
}  // namespace Profiles
