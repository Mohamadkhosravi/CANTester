/**
 * @file Bcm4.h
 * @brief BCM_4 — CAN ID 0x602, DLC 8, Period 100ms (BCM TX -> HVAC RX).
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{
    using ccl::msg::Direction;
    using ccl::msg::ICanMessage;

    class Bcm4 final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x602;
        static constexpr uint8_t kDlc = 8;
        static constexpr uint32_t kPeriodMs = 100;
        static constexpr ccl::msg::Direction kDir = ccl::msg::Direction::Rx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                // Bytes 0..2
                uint8_t _rsv0_23[3];

                // Byte 3 (Bit 24..31)
                uint8_t startSwitchStatus : 2; // Bit 24..25 (0: OFF, 1: ACC, 2: IGN, 3: Start Default)
                uint8_t _rsv26_31 : 6;         // Bit 26..31

                // Bytes 4..7
                uint8_t _rsv32_63[4];
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

        // Getters & Setters
        uint8_t startSwitchStatus() const { return f_.s.startSwitchStatus; }
        void setStartSwitchStatus(uint8_t v) { f_.s.startSwitchStatus = v & 0x03; }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}