/**
 * @file Hcu10.h
 * @brief HCU_10 — CAN ID 0x215, DLC 8, Period 100ms (HCU TX -> HVAC RX).
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{
    using ccl::msg::Direction;
    using ccl::msg::ICanMessage;

    class Hcu10 final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x215;
        static constexpr uint8_t kDlc = 8;
        static constexpr uint32_t kPeriodMs = 100;
        static constexpr ccl::msg::Direction kDir = ccl::msg::Direction::Rx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                uint8_t outGasPressure;         // Bar
                uint8_t inGasPressure;          // Bar
                uint8_t outGasTemp;             // °C (Offset: -40)
                uint8_t inGasTemp;              // °C (Offset: -40)
                uint16_t compressorActualSpeed; // RPM

                uint8_t acRequestStatus : 1;    // Bit 48
                uint8_t shutOffValveStatus : 1; // Bit 49
                uint8_t _rsv50_55 : 6;          // Bit 50..55

                uint8_t _rsv56_63; // Byte 7
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
        uint8_t outGasPressure() const { return f_.s.outGasPressure; }
        uint8_t inGasPressure() const { return f_.s.inGasPressure; }
        int16_t outGasTempDegC() const { return static_cast<int16_t>(f_.s.outGasTemp) - 40; }
        int16_t inGasTempDegC() const { return static_cast<int16_t>(f_.s.inGasTemp) - 40; }
        uint16_t compressorActualSpeed() const { return f_.s.compressorActualSpeed; }
        bool acRequestStatus() const { return f_.s.acRequestStatus != 0; }
        bool shutOffValveStatus() const { return f_.s.shutOffValveStatus != 0; }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}