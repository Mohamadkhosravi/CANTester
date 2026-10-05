#pragma once
#include <cstdint>
#include "ICanMessage.h"
namespace ccl::msg
{

    class ICanDatabase
    {
    public:
        virtual ~ICanDatabase() = default;


        virtual ICanMessage *find(std::uint32_t canId) const = 0;

        virtual std::size_t count() const = 0;
        virtual ICanMessage *at(std::size_t index) const = 0;
    };

} // namespace ccl::msg