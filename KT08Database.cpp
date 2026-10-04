// Messages/Kt08Database.cpp
#include "KT08Database.h"
#include <algorithm>
#include <cassert>

#if CAN_PROTOCOL_VERSION == CAN_LOW_SPEED
namespace kt08::msg
{
    Kt08Database::Kt08Database()
        : lookup_{{
              // مرتب‌شده صعودی بر اساس ID — شرط lower_bound / binary_search
              {BcmNtwrk::kId, &bcmNtwrk_},         // 0x028
              {HvacInfo1::kId, &hvacInfo1_},       // 0x086
              {BcmInfo2::kId, &bcmInfo2_},         // 0x088
              {BcmInfo4::kId, &bcmInfo4_},         // 0x0C8
              {BcmInfo5::kId, &bcmInfo5_},         // 0x0E8
              {HvacInfo2::kId, &hvacInfo2_},       // 0x146
              {MmsInfo3::kId, &mmsInfo3_},         // 0x170
              {HvacInfo3::kId, &hvacInfo3_},       // 0x186
              {BcmInfo19::kId, &bcmInfo19_},       // 0x1E8
              {BcmInfo20::kId, &bcmInfo20_},       // 0x2E9
              {HvacFlt::kId, &hvacFlt_},           // 0x626
              {BcmAtcAck::kId, &bcmAtcAck_},       // 0x62C
              {HvacSup::kId, &hvacSup_},           // 0x686
              {HvacPrmAns::kId, &hvacPrmAns_},     // 0x6A6
              {BcmAtcParReq::kId, &bcmAtcParReq_}, // 0x6AC
              // {HvacDiagAns::kId, &hvacDiagAns_},      // 0x706
              // {BcmHvacDiagReq::kId, &bcmHvacDiagReq_}, // 0x7A8
          }}
    {
        // گارد دیباگی: تضمین مرتب بودن آرایه (پیش‌شرط binary search)
        assert(std::is_sorted(
                   lookup_.begin(), lookup_.end(),
                   [](const Entry &a, const Entry &b)
                   { return a.id < b.id; }) &&
               "Kt08Database lookup_ must be sorted ascending by id");
    }

    ccl::msg::ICanMessage *Kt08Database::find(std::uint32_t canId) const
    {
        // binary search روی آرایه مرتب — O(log 15) ≈ 4 مقایسه
        auto it = std::lower_bound(
            lookup_.begin(), lookup_.end(), canId,
            [](const Entry &e, std::uint32_t id)
            { return e.id < id; });

        if (it != lookup_.end() && it->id == canId)
        {
            return it->msg;
        }
        return nullptr;
    }

} // namespace kt08::msg
#elif CAN_PROTOCOL_VERSION == CAN_HIGH_SPEED

namespace kt08::msg
{
    Kt08Database::Kt08Database()
        : lookup_{{

              {Hvac1::kId, &hvac1_}, // 0x086
              {Hcu1::kId, &hcu1_},   // 0x201
              {Hcu10::kId, &hcu10_}, // 0x215
              {Ems3::kId, &ems3_},   // 0x2F3
              {Bcm5::kId, &bcm5_},   // 0x601
              {Bcm4::kId, &bcm4_}    // 0x602
          }}
    {
        assert(std::is_sorted(
                   lookup_.begin(), lookup_.end(),
                   [](const Entry &a, const Entry &b)
                   { return a.id < b.id; }) &&
               "Kt08Database C-CAN lookup_ must be sorted ascending by id");
    }

    ccl::msg::ICanMessage *Kt08Database::find(std::uint32_t canId) const
    {
        auto it = std::lower_bound(
            lookup_.begin(), lookup_.end(), canId,
            [](const Entry &e, std::uint32_t id)
            { return e.id < id; });

        if (it != lookup_.end() && it->id == canId)
        {
            return it->msg;
        }
        return nullptr;
    }

} //

#endif