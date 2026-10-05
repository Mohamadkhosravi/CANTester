/**
 * @file BcmAtcAck.h
 * @brief BCM_ATCAck — CAN ID 0x62C, DLC 1, Event (BCM TX -> MMS/HVAC RX).
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{

    class BcmAtcAck final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x62C, kPeriodMs = 0; // event
        static constexpr uint8_t kDlc = 1;
        static constexpr ccl::msg::Direction kDir = ccl::msg::Direction::Rx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                uint8_t ack;         // 0..7
                uint8_t _rsv8_63[7]; // 8..63
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

        uint8_t ack() const { return f_.s.ack; }
        void setAck(uint8_t v) { f_.s.ack = v; }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}
