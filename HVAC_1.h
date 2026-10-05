/**
 * @file Hvac1.h
 * @brief HVAC_1 — CAN ID 0x086, DLC 8, Period 100ms (HVAC TX -> Vehicles RX).
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{
    using ccl::msg::Direction;
    using ccl::msg::ICanMessage;

    class Hvac1 final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x086;
        static constexpr uint8_t kDlc = 8;
        static constexpr uint32_t kPeriodMs = 100;
        static constexpr ccl::msg::Direction kDir = ccl::msg::Direction::Tx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                uint8_t actualEvaporatorTemp; // Byte 0 (Res: 0.5, Offset: -40)
                uint8_t targetEvaporatorTemp; // Byte 1 (Res: 0.5, Offset: -40)

                uint8_t acRequest : 1;                // Bit 16
                uint8_t rearDefrostRequestSwitch : 1; // Bit 17
                uint8_t heaterRequest : 1;            // Bit 18
                uint8_t targetCabinTemp : 5;          // Bit 19..23

                uint8_t actualCabinTemp; // Byte 3 (Res: 0.5, Offset: -40)

                uint8_t _rsv32_63[4]; // Bytes 4..7
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

        // Getters
        float actualEvaporatorTempDegC() const { return (f_.s.actualEvaporatorTemp * 0.5f) - 40.0f; }
        float targetEvaporatorTempDegC() const { return (f_.s.targetEvaporatorTemp * 0.5f) - 40.0f; }
        bool acRequest() const { return f_.s.acRequest != 0; }
        bool rearDefrostRequestSwitch() const { return f_.s.rearDefrostRequestSwitch != 0; }
        bool heaterRequest() const { return f_.s.heaterRequest != 0; }
        uint8_t targetCabinTemp() const { return f_.s.targetCabinTemp; }
        float actualCabinTempDegC() const { return (f_.s.actualCabinTemp * 0.5f) - 40.0f; }

        // Setters
        void setActualEvaporatorTempDegC(float tempC){ f_.s.actualEvaporatorTemp = static_cast<uint8_t>((tempC + 40.0f) / 0.5f);}
        void setTargetEvaporatorTempDegC(float tempC){f_.s.targetEvaporatorTemp = static_cast<uint8_t>((tempC + 40.0f) / 0.5f);}
        void setAcRequest(bool v) { f_.s.acRequest = v ? 1 : 0; }
        void setRearDefrostRequestSwitch(bool v) { f_.s.rearDefrostRequestSwitch = v ? 1 : 0; }
        void setHeaterRequest(bool v) { f_.s.heaterRequest = v ? 1 : 0; }
        void setTargetCabinTemp(uint8_t v) { f_.s.targetCabinTemp = v & 0x1F; }
        void setActualCabinTempDegC(float tempC){f_.s.actualCabinTemp = static_cast<uint8_t>((tempC + 40.0f) / 0.5f);}
        // Getres






        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}