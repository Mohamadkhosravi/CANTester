#pragma once
#include <cstdint>
#include "ICanMessage.h"
namespace ccl::msg
{

    class ICanDatabase
    {
    public:
        virtual ~ICanDatabase() = default;

        /** @return پیام متناظر با @p canId یا nullptr اگر ناشناخته باشد. */
        virtual ICanMessage *find(std::uint32_t canId) const = 0;

        /** برای پیمایش در MessageManager::process — بدون افشای storage داخلی. */
        virtual std::size_t count() const = 0;
        virtual ICanMessage *at(std::size_t index) const = 0;
    };

} // namespace ccl::msg