/**
 * @file HvacInfo3.h
 * @brief HVAC_INFO3 — CAN ID 0x186, DLC 8, 100 ms (HVAC TX -> MMS RX).
 * @note VolumeDec/VolumeSwitchPressed start at bit 8 (byte 1); no
 *       cross-byte straddle since fields align within bytes 0 and 1.
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{
    using ccl::msg::Direction;
    using ccl::msg::ICanMessage;
    class HvacInfo3 final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x186, kPeriodMs = 100;
        static constexpr uint8_t kDlc = 8;
        static constexpr Direction kDir = Direction::Tx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                uint8_t centralLockSwitch : 3;   // 0..2
                uint8_t volumeIncSwitch : 3;     // 3..5
                uint8_t _rsv6_7 : 2;             // 6..7
                uint8_t volumeDecSwitch : 3;     // 8..10
                uint8_t volumeSwitchPressed : 3; // 11..13
                uint8_t _rsv14_15 : 2;           // 14..15
                uint8_t _rsv16_63[6];            // 16..63
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

        uint8_t centralLockSwitch() const { return f_.s.centralLockSwitch; }
        void setCentralLockSwitch(uint8_t v) { f_.s.centralLockSwitch = v & 0x7; }
        uint8_t volumeIncSwitch() const { return f_.s.volumeIncSwitch; }
        void setVolumeIncSwitch(uint8_t v) { f_.s.volumeIncSwitch = v & 0x7; }
        uint8_t volumeDecSwitch() const { return f_.s.volumeDecSwitch; }
        void setVolumeDecSwitch(uint8_t v) { f_.s.volumeDecSwitch = v & 0x7; }
        uint8_t volumeSwitchPressed() const { return f_.s.volumeSwitchPressed; }
        void setVolumeSwitchPressed(uint8_t v) { f_.s.volumeSwitchPressed = v & 0x7; }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}
