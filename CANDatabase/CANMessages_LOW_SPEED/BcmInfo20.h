/**
 * @file BcmInfo20.h
 * @brief BCM_INFO20 — CAN ID 0x2E9, DLC 8, 100 ms (BCM TX -> MMS/HVAC RX).
 * @note Hybrid-only. Single signal PTReady at bit 23 (byte 2, bit 7).
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{
    using ccl::msg::Direction;
    using ccl::msg::ICanMessage;
    class BcmInfo20 final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x2E9, kPeriodMs = 100;
        static constexpr uint8_t kDlc = 8;
        static constexpr Direction kDir = Direction::Rx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                uint8_t _rsv0_15[2];   // 0..15
                uint8_t _rsv16_22 : 7; // 16..22
                uint8_t ptReady : 1;   // 23
                uint8_t _rsv24_63[5];  // 24..63
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

        uint8_t ptReady() const { return f_.s.ptReady; }
        void setPtReady(uint8_t v) { f_.s.ptReady = v & 0x1; }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}
