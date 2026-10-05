/**
 * @file HvacFlt.h
 * @brief HVAC_FLT — CAN ID 0x626, DLC 8, Event (HVAC TX).
 * @note FaultDTC is 24-bit; stored little-endian across bytes 1..3.
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{
    using ccl::msg::Direction;
    using ccl::msg::ICanMessage;
    class HvacFlt final : public ccl::msg::ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x626, kPeriodMs = 0; // event
        static constexpr uint8_t kDlc = 8;
        static constexpr Direction kDir = Direction::Tx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                uint8_t faultHeader;    // 0..7
                uint32_t faultDtc : 24; // 8..31
                uint8_t _rsv32_63[3];   // 32..63
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

        uint8_t faultHeader() const { return f_.s.faultHeader; }
        void setFaultHeader(uint8_t v) { f_.s.faultHeader = v; }
        uint32_t faultDtc() const { return f_.s.faultDtc; }
        void setFaultDtc(uint32_t v) { f_.s.faultDtc = v & 0xFFFFFF; }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}
