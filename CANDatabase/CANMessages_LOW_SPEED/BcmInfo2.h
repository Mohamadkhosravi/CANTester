/**
 * @file BcmInfo2.h
 * @brief BCM_INFO2 — CAN ID 0x088, DLC 8, 100 ms (BCM TX -> HVAC RX).
 * @note BCM_LightIntensity [40:8] EXCLUDED: overlaps
 *       BCM_InstantVehicleSpeed [32:12] at bits 40..43.
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{
    using ccl::msg::Direction;
    using ccl::msg::ICanMessage;
    class BcmInfo2 final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x088, kPeriodMs = 100; // was 0 (wrong)
        static constexpr uint8_t kDlc = 8;
        static constexpr Direction kDir = Direction::Rx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                uint8_t vehicleType : 1;           // 0
                uint8_t _rsv1_7 : 7;               // 1..7
                uint8_t engineState : 2;           // 8..9
                uint8_t startSwitchState : 2;      // 10..11
                uint8_t speedValidity : 4;         // 12..15
                uint8_t _rsv16_31[2];              // 16..31
                uint16_t instantVehicleSpeed : 12; // 32..43  f0.125
                uint16_t _rsv44_47 : 4;            // 44..47
                uint8_t _rsv48_63[2];              // 48..63
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

        uint8_t vehicleType() const { return f_.s.vehicleType; }
        void setVehicleType(uint8_t v) { f_.s.vehicleType = v & 0x1; }
        uint8_t engineState() const { return f_.s.engineState; }
        void setEngineState(uint8_t v) { f_.s.engineState = v & 0x3; }
        uint8_t startSwitchState() const { return f_.s.startSwitchState; }
        void setStartSwitchState(uint8_t v) { f_.s.startSwitchState = v & 0x3; }
        uint8_t speedValidity() const { return f_.s.speedValidity; }
        void setSpeedValidity(uint8_t v) { f_.s.speedValidity = v & 0xF; }
        uint16_t instantVehicleSpeed() const { return f_.s.instantVehicleSpeed; }
        void setInstantVehicleSpeed(uint16_t v) { f_.s.instantVehicleSpeed = v & 0xFFF; }

        float instantVehicleSpeedKph() const { return f_.s.instantVehicleSpeed * 0.125f; }
        void setInstantVehicleSpeedKph(float k) { f_.s.instantVehicleSpeed = (uint16_t)(k / 0.125f) & 0xFFF; }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}
