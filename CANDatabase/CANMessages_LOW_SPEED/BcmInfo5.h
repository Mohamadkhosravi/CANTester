/**
 * @file BcmInfo5.h
 * @brief BCM_INFO5 — CAN ID 0x0E8, DLC 8, 100 ms (BCM TX -> HVAC RX).
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{
    using ccl::msg::Direction;
    using ccl::msg::ICanMessage;
    class BcmInfo5 final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x0E8, kPeriodMs = 100;
        static constexpr uint8_t kDlc = 8;
        static constexpr Direction kDir = Direction::Rx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                uint8_t _rsv0_6 : 7;                // 0..6
                uint8_t sideLampsState : 1;         // 7
                uint8_t _rsv8_39[4];                // 8..39
                uint8_t _rsv40_42 : 3;              // 40..42
                uint8_t ambianceBackLightLevel : 3; // 43..45
                uint8_t _rsv46_47 : 2;              // 46..47
                uint8_t _rsv48_63[2];               // 48..63
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

        uint8_t sideLampsState() const { return f_.s.sideLampsState; }
        void setSideLampsState(uint8_t v) { f_.s.sideLampsState = v & 0x1; }
        uint8_t ambianceBackLightLevel() const { return f_.s.ambianceBackLightLevel; }
        void setAmbianceBackLightLevel(uint8_t v) { f_.s.ambianceBackLightLevel = v & 0x7; }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}
