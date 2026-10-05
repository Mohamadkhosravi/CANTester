/**
 * @file HvacPrmAns.h
 * @brief HVAC_PRMAns — CAN ID 0x6A6, DLC 8, Event (HVAC TX -> BCM RX).
 * @note 64-bit opaque parameter payload; raw byte access only.
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{
    using ccl::msg::Direction;
    using ccl::msg::ICanMessage;
    class HvacPrmAns final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x6A6, kPeriodMs = 0; // event
        static constexpr uint8_t kDlc = 8;
        static constexpr Direction kDir = Direction::Tx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                uint8_t parameterAnswer[8]; // 0..63
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

        const uint8_t *parameterAnswer() const { return f_.s.parameterAnswer; }
        void setParameterAnswer(const std::array<uint8_t, 8> &v)
        {
            std::memcpy(f_.s.parameterAnswer, v.data(), 8);
        }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}
