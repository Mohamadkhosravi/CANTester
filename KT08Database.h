#pragma once
#include "ICANDatabase.h"


#define CAN_LOW_SPEED 1
#define CAN_HIGH_SPEED 0

#define CAN_PROTOCOL_VERSION CAN_HIGH_SPEED

#if CAN_PROTOCOL_VERSION == CAN_LOW_SPEED

// HVAC TX
#include "HvacInfo1.h"
#include "HvacInfo2.h"
#include "HvacInfo3.h"
#include "HvacFlt.h"
#include "HvacSup.h"
#include "HvacPrmAns.h"

// BCM RX
#include "BcmNtwrk.h"
#include "BcmInfo2.h"
#include "BcmInfo4.h"
#include "BcmInfo5.h"
#include "BcmInfo19.h"
#include "BcmInfo20.h"
#include "BcmAtcAck.h"
#include "BcmAtcParReq.h"

// MMS RX
#include "MmsInfo3.h"

#include <array>
#include <cstdint>

namespace kt08::msg
{
    class Kt08Database final : public ccl::msg::ICanDatabase
    {
    public:
        Kt08Database();
        std::size_t count() const override { return kCount; }
        ccl::msg::ICanMessage *at(std::size_t i) const override { return lookup_[i].msg; }

        // پیاده‌سازی ICanDatabase — برای MessageManager
        ccl::msg::ICanMessage *find(std::uint32_t canId) const override;

        // دسترسی مستقیم برای لایه Application (بدون lookup)
        HvacInfo1 &hvacInfo1() { return hvacInfo1_; }
        HvacInfo2 &hvacInfo2() { return hvacInfo2_; }
        HvacInfo3 &hvacInfo3() { return hvacInfo3_; }
        HvacFlt &hvacFlt() { return hvacFlt_; }
        HvacSup &hvacSup() { return hvacSup_; }
        HvacPrmAns &hvacPrmAns() { return hvacPrmAns_; }
        // HvacDiagAns &hvacDiagAns() { return hvacDiagAns_; }

        BcmNtwrk &bcmNtwrk() { return bcmNtwrk_; }
        BcmInfo2 &bcmInfo2() { return bcmInfo2_; }
        BcmInfo4 &bcmInfo4() { return bcmInfo4_; }
        BcmInfo5 &bcmInfo5() { return bcmInfo5_; }
        BcmInfo19 &bcmInfo19() { return bcmInfo19_; }
        BcmInfo20 &bcmInfo20() { return bcmInfo20_; }
        BcmAtcAck &bcmAtcAck() { return bcmAtcAck_; }
        BcmAtcParReq &bcmAtcParReq() { return bcmAtcParReq_; }
        // BcmHvacDiagReq &bcmHvacDiagReq() { return bcmHvacDiagReq_; }

        MmsInfo3 &mmsInfo3() { return mmsInfo3_; }

    private:
        // --- HVAC TX ---
        HvacInfo1 hvacInfo1_;   // 0x086
        HvacInfo2 hvacInfo2_;   // 0x146
        HvacInfo3 hvacInfo3_;   // 0x186
        HvacFlt hvacFlt_;       // 0x626
        HvacSup hvacSup_;       // 0x686
        HvacPrmAns hvacPrmAns_; // 0x6A6
        // HvacDiagAns hvacDiagAns_; // 0x706

        // --- BCM RX ---
        BcmNtwrk bcmNtwrk_;         // 0x028
        BcmInfo2 bcmInfo2_;         // 0x088
        BcmInfo4 bcmInfo4_;         // 0x0C8
        BcmInfo5 bcmInfo5_;         // 0x0E8
        BcmInfo19 bcmInfo19_;       // 0x1E8
        BcmInfo20 bcmInfo20_;       // 0x2E9
        BcmAtcAck bcmAtcAck_;       // 0x62C
        BcmAtcParReq bcmAtcParReq_; // 0x6AC
        // BcmHvacDiagReq bcmHvacDiagReq_; // 0x7A8

        // --- MMS RX ---
        MmsInfo3 mmsInfo3_; // 0x170

        struct Entry
        {
            std::uint32_t id;
            ccl::msg::ICanMessage *msg;
        };

        // باید دقیقاً با تعداد entryهای فعال در constructor برابر باشد.
        // با فعال‌سازی HvacDiagAns (0x706) و BcmHvacDiagReq (0x7A8) → 17
        static constexpr std::size_t kCount = 15;
        std::array<Entry, kCount> lookup_;
    };

} // namespace kt08::msg
#elif CAN_PROTOCOL_VERSION == CAN_HIGH_SPEED

#include "BCM_4.h"
#include "BCM_5.h"
#include "HCU_1.h"
#include "HCU_10.h"
#include "EMS_3.h"
#include "HVAC_1.h"

namespace kt08::msg
{
    class Kt08Database final : public ccl::msg::ICanDatabase
    {
    public:
        Kt08Database();
        ~Kt08Database() override = default;

        std::size_t count() const override { return kCount; }
        ccl::msg::ICanMessage *at(std::size_t i) const override { return lookup_[i].msg; }
        ccl::msg::ICanMessage *find(std::uint32_t canId) const override;

        // Accessors for TX message
        Hvac1 &hvac1() { return hvac1_; }

        // Accessors for RX messages
        Hcu1 &hcu1() { return hcu1_; }
        Hcu10 &hcu10() { return hcu10_; }
        Ems3 &ems3() { return ems3_; }
        Bcm5 &bcm5() { return bcm5_; }
        Bcm4 &bcm4() { return bcm4_; }

    private:
        // --- HVAC TX ---
        Hvac1 hvac1_; // 0x086

        // --- BCM / HCU / EMS RX ---
        Hcu1 hcu1_;   // 0x201
        Hcu10 hcu10_; // 0x215
        Ems3 ems3_;   // 0x2F3
        Bcm5 bcm5_;   // 0x601
        Bcm4 bcm4_;   // 0x602

        struct Entry
        {
            std::uint32_t id;
            ccl::msg::ICanMessage *msg;
        };

        static constexpr std::size_t kCount = 6;
        std::array<Entry, kCount> lookup_;
    };
} // namespace kt08::msg

#endif