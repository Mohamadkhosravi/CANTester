#pragma once

#include "ICANDatabase.h"

// ---  HighSpeed ---
#include "CANDatabase/CANMessages_HIGH_SPEED/BCM_4.h"
#include "CANDatabase/CANMessages_HIGH_SPEED/BCM_5.h"
#include "CANDatabase/CANMessages_HIGH_SPEED/EMS_3.h"
#include "CANDatabase/CANMessages_HIGH_SPEED/HCU_1.h"
#include "CANDatabase/CANMessages_HIGH_SPEED/HCU_10.h"
#include "CANDatabase/CANMessages_HIGH_SPEED/HVAC_1.h"

// ---  LowSpeed ---
#include "CANDatabase/CANMessages_LOW_SPEED/HvacInfo1.h"
#include "CANDatabase/CANMessages_LOW_SPEED/HvacInfo2.h"
#include "CANDatabase/CANMessages_LOW_SPEED/HvacInfo3.h"
#include "CANDatabase/CANMessages_LOW_SPEED/HvacFlt.h"
#include "CANDatabase/CANMessages_LOW_SPEED/HvacSup.h"
#include "CANDatabase/CANMessages_LOW_SPEED/HvacPrmAns.h"

#include "CANDatabase/CANMessages_LOW_SPEED/BcmNtwrk.h"
#include "CANDatabase/CANMessages_LOW_SPEED/BcmInfo2.h"
#include "CANDatabase/CANMessages_LOW_SPEED/BcmInfo4.h"
#include "CANDatabase/CANMessages_LOW_SPEED/BcmInfo5.h"
#include "CANDatabase/CANMessages_LOW_SPEED/BcmInfo19.h"
#include "CANDatabase/CANMessages_LOW_SPEED/BcmInfo20.h"
#include "CANDatabase/CANMessages_LOW_SPEED/BcmAtcAck.h"
#include "CANDatabase/CANMessages_LOW_SPEED/BcmAtcParReq.h"

#include "CANDatabase/CANMessages_LOW_SPEED/MmsInfo3.h"

#include <array>
#include <cstdint>

// تعریف Enum برای نوع شبکه
enum class CanBusType {
    HighSpeed, // C-CAN (500 Kbps)
    LowSpeed   // B-CAN (125 Kbps)
};

namespace kt08::msg {

class Kt08Database {
public:
    struct Entry {
        std::uint32_t id;
        ccl::msg::ICanMessage* msg;
    };

    Kt08Database();
    void setBusType(CanBusType type) { m_busType = type; }
    CanBusType busType() const { return m_busType; }
    ccl::msg::ICanMessage* find(std::uint32_t canId) const;

    //  High-Speed ---
    Hvac1& hvac1() { return m_hvac1; }
    Hcu1&  hcu1()  { return m_hcu1; }
    Hcu10& hcu10() { return m_hcu10; }
    Ems3&  ems3()  { return m_ems3; }
    Bcm4&  bcm4()  { return m_bcm4; }
    Bcm5&  bcm5()  { return m_bcm5; }

    // --- Low-Speed ---
    BcmNtwrk&     bcmNtwrk()     { return m_bcmNtwrk; }
    HvacInfo1&    hvacInfo1()    { return m_hvacInfo1; }
    BcmInfo2&     bcmInfo2()     { return m_bcmInfo2; }
    BcmInfo4&     bcmInfo4()     { return m_bcmInfo4; }
    BcmInfo5&     bcmInfo5()     { return m_bcmInfo5; }
    HvacInfo2&    hvacInfo2()    { return m_hvacInfo2; }
    MmsInfo3&     mmsInfo3()     { return m_mmsInfo3; }
    HvacInfo3&    hvacInfo3()    { return m_hvacInfo3; }
    BcmInfo19&    bcmInfo19()    { return m_bcmInfo19; }
    BcmInfo20&    bcmInfo20()    { return m_bcmInfo20; }
    HvacFlt&      hvacFlt()      { return m_hvacFlt; }
    BcmAtcAck&    bcmAtcAck()    { return m_bcmAtcAck; }
    HvacSup&      hvacSup()      { return m_hvacSup; }
    HvacPrmAns&   hvacPrmAns()   { return m_hvacPrmAns; }
    BcmAtcParReq& bcmAtcParReq() { return m_bcmAtcParReq; }

private:
    CanBusType m_busType{CanBusType::HighSpeed};

    //  High-Speed
    Hvac1 m_hvac1{};
    Hcu1  m_hcu1{};
    Hcu10 m_hcu10{};
    Ems3  m_ems3{};
    Bcm4  m_bcm4{};
    Bcm5  m_bcm5{};

    //  Low-Speed
    BcmNtwrk     m_bcmNtwrk{};
    HvacInfo1    m_hvacInfo1{};
    BcmInfo2     m_bcmInfo2{};
    BcmInfo4     m_bcmInfo4{};
    BcmInfo5     m_bcmInfo5{};
    HvacInfo2    m_hvacInfo2{};
    MmsInfo3     m_mmsInfo3{};
    HvacInfo3    m_hvacInfo3{};
    BcmInfo19    m_bcmInfo19{};
    BcmInfo20    m_bcmInfo20{};
    HvacFlt      m_hvacFlt{};
    BcmAtcAck    m_bcmAtcAck{};
    HvacSup      m_hvacSup{};
    HvacPrmAns   m_hvacPrmAns{};
    BcmAtcParReq m_bcmAtcParReq{};

    std::array<Entry, 6>  m_hsLookup;
    std::array<Entry, 15> m_lsLookup;
};

} // namespace kt08::msg