/**
 * @file BcmInfo4.h
 * @brief BCM_INFO4 — CAN ID 0x0C8, DLC 8, 100 ms (BCM TX -> HVAC RX).
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{
    using ccl::msg::Direction;
    using ccl::msg::ICanMessage;
    class BcmInfo4 final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x0C8, kPeriodMs = 100;
        static constexpr uint8_t kDlc = 8;
        static constexpr Direction kDir = Direction::Rx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                uint8_t _rsv0_5 : 6;               // 0..5
                uint8_t acState : 1;               // 6
                uint8_t rearScreenHeaterState : 1; // 7
                uint8_t engineWaterTemperature;    // 8..15  f0.75 o-48
                uint8_t _rsv16_39[3];              // 16..39
                uint8_t ambientTemperature;        // 40..47 f0.5  o-40
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

        uint8_t acState() const { return f_.s.acState; }
        void setAcState(uint8_t v) { f_.s.acState = v & 0x1; }
        uint8_t rearScreenHeaterState() const { return f_.s.rearScreenHeaterState; }
        void setRearScreenHeaterState(uint8_t v) { f_.s.rearScreenHeaterState = v & 0x1; }
        uint8_t engineWaterTemperature() const { return f_.s.engineWaterTemperature; }
        void setEngineWaterTemperature(uint8_t v) { f_.s.engineWaterTemperature = v; }
        uint8_t ambientTemperature() const { return f_.s.ambientTemperature; }
        void setAmbientTemperature(uint8_t v) { f_.s.ambientTemperature = v; }

        float engineWaterTemperatureC() const { return f_.s.engineWaterTemperature * 0.75f - 48.0f; }
        void setEngineWaterTemperatureC(float c) { f_.s.engineWaterTemperature = (uint8_t)((c + 48.0f) / 0.75f); }
        float ambientTemperatureC() const { return f_.s.ambientTemperature * 0.5f - 40.0f; }
        void setAmbientTemperatureC(float c) { f_.s.ambientTemperature = (uint8_t)((c + 40.0f) / 0.5f); }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}
