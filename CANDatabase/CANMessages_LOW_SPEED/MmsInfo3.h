/**
 * @file MmsInfo3.h
 * @brief MMS_INFO3 — CAN ID 0x170, DLC 8, Event (MMS TX -> HVAC RX).
 * @note Event pattern (3x Pressed + 3x Not-Pressed @10ms) is a Manager
 *       concern, not encoded here. kPeriodMs = 0 (event sentinel).
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>
using namespace ccl::msg;
namespace kt08::msg
{
    class MmsInfo3 final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x170, kPeriodMs = 0; // event
        static constexpr uint8_t kDlc = 8;
        static constexpr Direction kDir = Direction::Rx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                uint8_t autoButtonState : 1;               // 0
                uint8_t intakePatternButtonState : 1;      // 1
                uint8_t faceModeButtonState : 1;           // 2
                uint8_t footModeButtonState : 1;           // 3
                uint8_t screenModeButtonState : 1;         // 4
                uint8_t frontScreenDefrostButtonState : 1; // 5
                uint8_t rearScreenHeaterButtonState : 1;   // 6
                uint8_t powerButtonState : 1;              // 7
                uint8_t acButtonState : 1;                 // 8
                uint8_t temperatureIncButtonState : 1;     // 9
                uint8_t temperatureDecButtonState : 1;     // 10
                uint8_t blowerIncButtonState : 1;          // 11
                uint8_t blowerDecButtonState : 1;          // 12
                uint8_t footFaceModeButtonState : 1;       // 13
                uint8_t footScreenModeButtonState : 1;     // 14
                uint8_t _rsv15 : 1;                        // 15
                uint8_t _rsv16_63[6];                      // 16..63
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

        uint8_t autoButtonState() const { return f_.s.autoButtonState; }
        uint8_t intakePatternButtonState() const { return f_.s.intakePatternButtonState; }
        uint8_t faceModeButtonState() const { return f_.s.faceModeButtonState; }
        uint8_t footModeButtonState() const { return f_.s.footModeButtonState; }
        uint8_t screenModeButtonState() const { return f_.s.screenModeButtonState; }
        uint8_t frontScreenDefrostButtonState() const { return f_.s.frontScreenDefrostButtonState; }
        uint8_t rearScreenHeaterButtonState() const { return f_.s.rearScreenHeaterButtonState; }
        uint8_t powerButtonState() const { return f_.s.powerButtonState; }
        uint8_t acButtonState() const { return f_.s.acButtonState; }
        uint8_t temperatureIncButtonState() const { return f_.s.temperatureIncButtonState; }
        uint8_t temperatureDecButtonState() const { return f_.s.temperatureDecButtonState; }
        uint8_t blowerIncButtonState() const { return f_.s.blowerIncButtonState; }
        uint8_t blowerDecButtonState() const { return f_.s.blowerDecButtonState; }
        uint8_t footFaceModeButtonState() const { return f_.s.footFaceModeButtonState; }
        uint8_t footScreenModeButtonState() const { return f_.s.footScreenModeButtonState; }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}
