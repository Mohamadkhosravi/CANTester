#pragma once

#include <cstdint>
#include <array>
namespace ccl::msg
{
    // Runtime state for a single CAN message.
    // Config (id/dlc/period...) is compile-time and lives in each concrete class.
    struct MessageState
    {
        uint32_t txCounterMs{0};   // accumulates until period is reached (TX)
        uint64_t lastRxTicksMs{0}; // timestamp of last received frame (RX)
        bool valid{false};         // RX: false once timeout elapsed
        bool newData{false};       // RX: set on fresh frame, cleared by consumer
        bool txTriggered{false};   // TX: request a one-shot send
    };

    enum class Direction : uint8_t
    {
        Tx,
        Rx
    };

    class ICanMessage
    {
    public:
        virtual ~ICanMessage() = default;

        // Fill buffer from internal packed struct. Returns bytes written (== dlc).
        virtual uint8_t encode(std::array<uint8_t, 8> &buffer) const = 0;

        // Fill internal packed struct from a received frame.
        virtual void decode(const std::array<uint8_t, 8> &buffer, uint8_t len) = 0;

        // Compile-time config exposed polymorphically to the manager.
        virtual uint32_t id() const = 0;
        virtual uint8_t dlc() const = 0;
        virtual uint32_t periodMs() const = 0;
        virtual Direction direction() const = 0;
        virtual uint32_t timeoutMs() const
        {
            if (direction() != Direction::Rx)
                return 0;
            const uint32_t p = periodMs();
            return p ? p * 10 : 0;
        }

        bool isValid() const { return state_.valid; }
        bool hasNewData() const { return state_.newData; }
        void clearNewData() { state_.newData = false; }
        // Request a one-shot TX send on the next manager tick.
        void trigger() { state_.txTriggered = true; }

        MessageState &state() { return state_; }
        const MessageState &state() const { return state_; }

    protected:
        MessageState state_;
    };

}