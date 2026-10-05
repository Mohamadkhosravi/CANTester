#include "CANDatabase/KT08Database.h"
#include <algorithm>
#include <cassert>

namespace kt08::msg
{
    Kt08Database::Kt08Database()

        : m_hsLookup{{
            {Hvac1::kId, &m_hvac1}, // 0x086
            {Hcu1::kId,  &m_hcu1},  // 0x201
            {Hcu10::kId, &m_hcu10}, // 0x215
            {Ems3::kId,  &m_ems3},  // 0x2F3
            {Bcm5::kId,  &m_bcm5},  // 0x601
            {Bcm4::kId,  &m_bcm4}   // 0x602
          }},

          m_lsLookup{{
            {BcmNtwrk::kId,     &m_bcmNtwrk},     // 0x028
            {HvacInfo1::kId,    &m_hvacInfo1},    // 0x086
            {BcmInfo2::kId,     &m_bcmInfo2},     // 0x088
            {BcmInfo4::kId,     &m_bcmInfo4},     // 0x0C8
            {BcmInfo5::kId,     &m_bcmInfo5},     // 0x0E8
            {HvacInfo2::kId,    &m_hvacInfo2},    // 0x146
            {MmsInfo3::kId,     &m_mmsInfo3},     // 0x170
            {HvacInfo3::kId,    &m_hvacInfo3},    // 0x186
            {BcmInfo19::kId,    &m_bcmInfo19},    // 0x1E8
            {BcmInfo20::kId,    &m_bcmInfo20},    // 0x2E9
            {HvacFlt::kId,      &m_hvacFlt},      // 0x626
            {BcmAtcAck::kId,    &m_bcmAtcAck},    // 0x62C
            {HvacSup::kId,      &m_hvacSup},      // 0x686
            {HvacPrmAns::kId,   &m_hvacPrmAns},   // 0x6A6
            {BcmAtcParReq::kId, &m_bcmAtcParReq}  // 0x6AC
          }}
    {

        assert(std::is_sorted(m_hsLookup.begin(), m_hsLookup.end(),
                               [](const Entry &a, const Entry &b) { return a.id < b.id; }) &&
               "Kt08Database HighSpeed lookup must be sorted");

        assert(std::is_sorted(m_lsLookup.begin(), m_lsLookup.end(),
                               [](const Entry &a, const Entry &b) { return a.id < b.id; }) &&
               "Kt08Database LowSpeed lookup must be sorted");
    }

    ccl::msg::ICanMessage* Kt08Database::find(std::uint32_t canId) const
    {
        if (m_busType == CanBusType::HighSpeed)
        {
            auto it = std::lower_bound(
                m_hsLookup.begin(), m_hsLookup.end(), canId,
                [](const Entry &e, std::uint32_t id) { return e.id < id; });

            if (it != m_hsLookup.end() && it->id == canId) {
                return it->msg;
            }
        }
        else // LowSpeed
        {
            auto it = std::lower_bound(
                m_lsLookup.begin(), m_lsLookup.end(), canId,
                [](const Entry &e, std::uint32_t id) { return e.id < id; });

            if (it != m_lsLookup.end() && it->id == canId) {
                return it->msg;
            }
        }

        return nullptr;
    }

} // namespace kt08::msg