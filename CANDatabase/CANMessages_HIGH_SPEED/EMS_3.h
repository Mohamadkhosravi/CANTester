/**
 * @file Ems3.h
 * @brief EMS_3 — CAN ID 0x2F3, DLC 8, Period 100ms (EMS TX -> HVAC RX).
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{
    using ccl::msg::Direction;
    using ccl::msg::ICanMessage;
    class Ems3 final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x2F3;
        static constexpr uint8_t kDlc = 8;
        static constexpr uint32_t kPeriodMs = 100;
        static constexpr ccl::msg::Direction kDir = ccl::msg::Direction::Rx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                // Byte 0: Engine Coolant Temp (Res: 0.75, Offset: -48, 0xFF: Invalid)
                uint8_t engineCoolantTemp;

                // Bytes 1..7
                uint8_t _rsv8_63[7];
            } s;
        };
#pragma pack(pop)
        static_assert(sizeof(Frame) == 8, "frame must be 8 bytes");

        uint32_t id() const override { return kId; }
        uint8_t dlc() const override { return kDlc; }
        uint32_t periodMs() const override { return kPeriodMs; }
        ccl::msg::Direction direction() const override { return kDir; }

        uint8_t encode(std::array<uint8_t, 8> &b) const override
        {
            std::memcpy(b.data(), f_.raw, kDlc);
            return kDlc;
        }
        void decode(const std::array<uint8_t, 8> &b, uint8_t len) override
        {
            std::memcpy(f_.raw, b.data(), len < kDlc ? len : kDlc);
        }

        // Physical Temp (°C)
        float engineCoolantTempDegC() const { return (f_.s.engineCoolantTemp * 0.75f) - 48.0f; }
        void setEngineCoolantTempDegC(float tempDegC) {
            f_.s.engineCoolantTemp = static_cast<uint16_t>((tempDegC + 48.0f) / 0.75f);
        }
        bool isValid() const { return f_.s.engineCoolantTemp != 0xFF; }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}