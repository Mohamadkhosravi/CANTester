/**
 * @file HvacInfo2.h
 * @brief HVAC_INFO2 — CAN ID 0x146, DLC 8, 100 ms (HVAC TX -> BCM/MMS RX).
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{
    using ccl::msg::Direction;
    using ccl::msg::ICanMessage;
    class HvacInfo2 final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x146, kPeriodMs = 100;
        static constexpr uint8_t kDlc = 8;
        static constexpr Direction kDir = Direction::Tx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                uint8_t autoMode : 1;                 // 0
                uint8_t intakePattern : 1;            // 1
                uint8_t faceMode : 1;                 // 2
                uint8_t footMode : 1;                 // 3
                uint8_t screenMode : 1;               // 4
                uint8_t frontScreenDefrostState : 1;  // 5
                uint8_t powerState : 1;               // 6
                uint8_t acIndicator : 1;              // 7
                uint8_t temperatureSet : 6;           // 8..13  f0.5 o16
                uint8_t heaterRequest : 1;            // 14
                uint8_t _rsv15 : 1;                   // 15
                uint8_t blowerSpeed : 4;              // 16..19
                uint8_t blowerSwitchPressed : 3;      // 20..22
                uint8_t _rsv23 : 1;                   // 23
                uint8_t cabinTemperature;             // 24..31 f0.5 o-40
                uint8_t footFaceMode : 1;             // 32
                uint8_t footScreenMode : 1;           // 33
                uint8_t temperatureSwitchPressed : 3; // 34..36
                uint8_t _rsv37_39 : 3;                // 37..39
                uint8_t _rsv40_63[3];                 // 40..63
            } s;
        };
#pragma pack(pop)
        static_assert(sizeof(Frame) == 8, "frame must be 8 bytes");

        uint32_t id() const override { return kId; }
        uint8_t dlc() const override { return kDlc; }
        uint32_t periodMs() const override { return kPeriodMs; }
        Direction direction() const override { return kDir; }

        uint8_t encode(std::array<uint8_t, 8> &b) const override
        {
            std::memcpy(b.data(), f_.raw, kDlc);
            return kDlc;
        }
        void decode(const std::array<uint8_t, 8> &b, uint8_t len) override
        {
            std::memcpy(f_.raw, b.data(), len < kDlc ? len : kDlc);
        }

        uint8_t autoMode() const { return f_.s.autoMode; }
        void setAutoMode(uint8_t v) { f_.s.autoMode = v & 0x1; }
        uint8_t intakePattern() const { return f_.s.intakePattern; }
        void setIntakePattern(uint8_t v) { f_.s.intakePattern = v & 0x1; }
        uint8_t faceMode() const { return f_.s.faceMode; }
        void setFaceMode(uint8_t v) { f_.s.faceMode = v & 0x1; }
        uint8_t footMode() const { return f_.s.footMode; }
        void setFootMode(uint8_t v) { f_.s.footMode = v & 0x1; }
        uint8_t screenMode() const { return f_.s.screenMode; }
        void setScreenMode(uint8_t v) { f_.s.screenMode = v & 0x1; }
        uint8_t frontScreenDefrostState() const { return f_.s.frontScreenDefrostState; }
        void setFrontScreenDefrostState(uint8_t v) { f_.s.frontScreenDefrostState = v & 0x1; }
        uint8_t powerState() const { return f_.s.powerState; }
        void setPowerState(uint8_t v) { f_.s.powerState = v & 0x1; }
        uint8_t acIndicator() const { return f_.s.acIndicator; }
        void setAcIndicator(uint8_t v) { f_.s.acIndicator = v & 0x1; }
        uint8_t temperatureSet() const { return f_.s.temperatureSet; }
        void setTemperatureSet(uint8_t v) { f_.s.temperatureSet = v & 0x3F; }
        uint8_t heaterRequest() const { return f_.s.heaterRequest; }
        void setHeaterRequest(uint8_t v) { f_.s.heaterRequest = v & 0x1; }
        uint8_t blowerSpeed() const { return f_.s.blowerSpeed; }
        void setBlowerSpeed(uint8_t v) { f_.s.blowerSpeed = v & 0xF; }
        uint8_t blowerSwitchPressed() const { return f_.s.blowerSwitchPressed; }
        void setBlowerSwitchPressed(uint8_t v) { f_.s.blowerSwitchPressed = v & 0x7; }
        uint8_t cabinTemperature() const { return f_.s.cabinTemperature; }
        void setCabinTemperature(uint8_t v) { f_.s.cabinTemperature = v; }
        uint8_t footFaceMode() const { return f_.s.footFaceMode; }
        void setFootFaceMode(uint8_t v) { f_.s.footFaceMode = v & 0x1; }
        uint8_t footScreenMode() const { return f_.s.footScreenMode; }
        void setFootScreenMode(uint8_t v) { f_.s.footScreenMode = v & 0x1; }
        uint8_t temperatureSwitchPressed() const { return f_.s.temperatureSwitchPressed; }
        void setTemperatureSwitchPressed(uint8_t v) { f_.s.temperatureSwitchPressed = v & 0x7; }

        float temperatureSetC() const { return f_.s.temperatureSet * 0.5f + 16.0f; }
        void setTemperatureSetC(float c) { f_.s.temperatureSet = (uint8_t)((c - 16.0f) / 0.5f) & 0x3F; }
        float cabinTemperatureC() const { return f_.s.cabinTemperature * 0.5f - 40.0f; }
        void setCabinTemperatureC(float c) { f_.s.cabinTemperature = (uint8_t)((c + 40.0f) / 0.5f); }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}
