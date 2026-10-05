/**
 * @file HvacSup.h
 * @brief HVAC_SUP — CAN ID 0x686, DLC 8, 1000 ms (HVAC TX).
 * @note Supervision/status. Sparse single-bit flags at bits
 *       3, 35, 38, 43, 46 (little-endian byte/bit mapping).
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{
    using ccl::msg::Direction;
    using ccl::msg::ICanMessage;
    class HvacSup final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x686, kPeriodMs = 1000;
        static constexpr uint8_t kDlc = 8;
        static constexpr Direction kDir = Direction::Tx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                uint8_t _rsv0_2 : 3;          // 0..2
                uint8_t diagMod : 1;          // 3
                uint8_t _rsv4_7 : 4;          // 4..7
                uint8_t _rsv8_31[3];          // 8..31
                uint8_t _rsv32_34 : 3;        // 32..34
                uint8_t bcmDynamicStates : 1; // 35
                uint8_t _rsv36_37 : 2;        // 36..37
                uint8_t mmsDynamicStates : 1; // 38
                uint8_t _rsv39 : 1;           // 39
                uint8_t _rsv40_42 : 3;        // 40..42
                uint8_t bcmStaticStates : 1;  // 43
                uint8_t _rsv44_45 : 2;        // 44..45
                uint8_t mmsStaticStates : 1;  // 46
                uint8_t _rsv47 : 1;           // 47
                uint8_t _rsv48_63[2];         // 48..63
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

        uint8_t diagMod() const { return f_.s.diagMod; }
        void setDiagMod(uint8_t v) { f_.s.diagMod = v & 0x1; }
        uint8_t bcmDynamicStates() const { return f_.s.bcmDynamicStates; }
        void setBcmDynamicStates(uint8_t v) { f_.s.bcmDynamicStates = v & 0x1; }
        uint8_t mmsDynamicStates() const { return f_.s.mmsDynamicStates; }
        void setMmsDynamicStates(uint8_t v) { f_.s.mmsDynamicStates = v & 0x1; }
        uint8_t bcmStaticStates() const { return f_.s.bcmStaticStates; }
        void setBcmStaticStates(uint8_t v) { f_.s.bcmStaticStates = v & 0x1; }
        uint8_t mmsStaticStates() const { return f_.s.mmsStaticStates; }
        void setMmsStaticStates(uint8_t v) { f_.s.mmsStaticStates = v & 0x1; }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}
