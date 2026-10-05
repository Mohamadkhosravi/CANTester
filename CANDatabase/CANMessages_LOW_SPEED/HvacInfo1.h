/**
 * @file HvacInfo1.h
 * @brief HVAC_INFO1 — CAN ID 0x086, DLC 8, 100 ms (HVAC TX -> BCM RX).
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{
    using ccl::msg::Direction;
    using ccl::msg::ICanMessage;
    class HvacInfo1 final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x086, kPeriodMs = 100;
        static constexpr uint8_t kDlc = 8;
        static constexpr Direction kDir = Direction::Tx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                uint8_t evaporatorTargetTemperature; // 0..7   f0.5 o-20
                uint8_t evaporatorActualTemperature; // 8..15  f0.5 o-20
                uint8_t _rsv16_39[3];                // 16..39
                uint8_t acRequest : 1;               // 40
                uint8_t rearScreenHeaterSwitch : 1;  // 41
                uint8_t _rsv42_47 : 6;               // 42..47
                uint8_t _rsv48_63[2];                // 48..63
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

        uint8_t evaporatorTargetTemperature() const { return f_.s.evaporatorTargetTemperature; }
        void setEvaporatorTargetTemperature(uint8_t v) { f_.s.evaporatorTargetTemperature = v; }
        uint8_t evaporatorActualTemperature() const { return f_.s.evaporatorActualTemperature; }
        void setEvaporatorActualTemperature(uint8_t v) { f_.s.evaporatorActualTemperature = v; }
        uint8_t acRequest() const { return f_.s.acRequest; }
        void setAcRequest(uint8_t v) { f_.s.acRequest = v & 0x1; }
        uint8_t rearScreenHeaterSwitch() const { return f_.s.rearScreenHeaterSwitch; }
        void setRearScreenHeaterSwitch(uint8_t v) { f_.s.rearScreenHeaterSwitch = v & 0x1; }

        // physical helpers: phys = raw * factor + offset
        float evaporatorTargetTemperatureC() const { return f_.s.evaporatorTargetTemperature * 0.5f - 20.0f; }
        void setEvaporatorTargetTemperatureC(float c) { f_.s.evaporatorTargetTemperature = (uint8_t)((c + 20.0f) / 0.5f); }
        float evaporatorActualTemperatureC() const { return f_.s.evaporatorActualTemperature * 0.5f - 20.0f; }
        void setEvaporatorActualTemperatureC(float c) { f_.s.evaporatorActualTemperature = (uint8_t)((c + 20.0f) / 0.5f); }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}
