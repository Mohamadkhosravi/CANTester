/**
 * @file BcmInfo19.h
 * @brief BCM_INFO19 — CAN ID 0x1E8, DLC 8, 100 ms (BCM TX -> HVAC RX).
 * @note Hybrid-only. CompressorActualSpeed factor 50 exceeds uint8 range
 *       on decode; physical helper returns uint16 rpm.
 */
#pragma once
#include "ICanMessage.h"
#include <cstring>

namespace kt08::msg
{
    using ccl::msg::Direction;
    using ccl::msg::ICanMessage;
    class BcmInfo19 final : public ICanMessage
    {
    public:
        static constexpr uint32_t kId = 0x1E8, kPeriodMs = 100;
        static constexpr uint8_t kDlc = 8;
        static constexpr Direction kDir = Direction::Rx;

#pragma pack(push, 1)
        union Frame
        {
            uint8_t raw[8];
            struct
            {
                uint8_t compressorActualSpeed;  // 0..7   f50
                uint8_t _rsv8_15;               // 8..15
                uint8_t outGasPressure;         // 16..23
                uint8_t inGasPressure;          // 24..31
                uint8_t outGasTemperature;      // 32..39 o-40
                uint8_t inGasTemperature;       // 40..47 o-40
                uint8_t shutOffValveStatus : 1; // 48
                uint8_t _rsv49_55 : 7;          // 49..55
                uint8_t _rsv56_63;              // 56..63
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

        uint8_t compressorActualSpeed() const { return f_.s.compressorActualSpeed; }
        void setCompressorActualSpeed(uint8_t v) { f_.s.compressorActualSpeed = v; }
        uint8_t outGasPressure() const { return f_.s.outGasPressure; }
        void setOutGasPressure(uint8_t v) { f_.s.outGasPressure = v; }
        uint8_t inGasPressure() const { return f_.s.inGasPressure; }
        void setInGasPressure(uint8_t v) { f_.s.inGasPressure = v; }
        uint8_t outGasTemperature() const { return f_.s.outGasTemperature; }
        void setOutGasTemperature(uint8_t v) { f_.s.outGasTemperature = v; }
        uint8_t inGasTemperature() const { return f_.s.inGasTemperature; }
        void setInGasTemperature(uint8_t v) { f_.s.inGasTemperature = v; }
        uint8_t shutOffValveStatus() const { return f_.s.shutOffValveStatus; }
        void setShutOffValveStatus(uint8_t v) { f_.s.shutOffValveStatus = v & 0x1; }

        uint16_t compressorActualSpeedRpm() const { return (uint16_t)f_.s.compressorActualSpeed * 50; }
        void setCompressorActualSpeedRpm(uint16_t r) { f_.s.compressorActualSpeed = (uint8_t)(r / 50); }
        int16_t outGasTemperatureC() const { return (int16_t)f_.s.outGasTemperature - 40; }
        void setOutGasTemperatureC(int16_t c) { f_.s.outGasTemperature = (uint8_t)(c + 40); }
        int16_t inGasTemperatureC() const { return (int16_t)f_.s.inGasTemperature - 40; }
        void setInGasTemperatureC(int16_t c) { f_.s.inGasTemperature = (uint8_t)(c + 40); }

        const Frame &frame() const { return f_; }

    private:
        Frame f_{};
    };
}
