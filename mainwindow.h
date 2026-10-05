#pragma once
#include <QMainWindow>
#include <QTimer>
#include <QTableWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QLibrary>
#include <QDir>
#include <windows.h>
#include "KT08Database.h"
#include <QCoreApplication>
// ۱. ساختارهای مربوط به ControlCAN API
#include <QDir>

#define VCI_USBCAN1 3
#define VCI_USBCAN2 4
#define VCI_USBCAN2A 4



#pragma pack(push, 1)



#pragma pack(pop)

typedef DWORD (*VCI_OpenDevice_t)(DWORD, DWORD, DWORD);
typedef DWORD (*VCI_CloseDevice_t)(DWORD, DWORD);
typedef DWORD (*VCI_InitCAN_t)(DWORD, DWORD, DWORD, void*);
typedef DWORD (*VCI_StartCAN_t)(DWORD, DWORD, DWORD);


#pragma pack(push, 1)

typedef struct
{
    USHORT hw_Version;
    USHORT fw_Version;
    USHORT dr_Version;
    USHORT in_Version;
    USHORT irq_Num;
    BYTE can_Num;
    CHAR str_Serial_Num[20];
    CHAR str_hw_Type[40];
    USHORT Reserved[4];
} VCI_BOARD_INFO;

#pragma pack(pop)





#pragma pack(push, 1)
struct VCI_INIT_CONFIG {
    DWORD AccCode;
    DWORD AccMask;
    DWORD Reserved;
    UCHAR Filter;
    UCHAR Timing0;
    UCHAR Timing1;
    UCHAR Mode;
};

struct VCI_CAN_OBJ {
    UINT ID;
    UINT TimeStamp;
    BYTE TimeFlag;
    BYTE SendType;
    BYTE RemoteFlag;
    BYTE ExternFlag;
    BYTE DataLen;
    BYTE Data[8];
    BYTE Reserved[3];
};
#pragma pack(pop)

typedef DWORD (WINAPI *pfnVCI_OpenDevice)(DWORD DevType, DWORD DevIndex, DWORD Reserved);
typedef DWORD (WINAPI *pfnVCI_CloseDevice)(DWORD DevType, DWORD DevIndex);
typedef DWORD (WINAPI *pfnVCI_InitCAN)(DWORD DevType, DWORD DevIndex, DWORD CANIndex, VCI_INIT_CONFIG* pInitConfig);
typedef DWORD (WINAPI *pfnVCI_StartCAN)(DWORD DevType, DWORD DevIndex, DWORD CANIndex);
typedef ULONG (WINAPI *pfnVCI_Transmit)(DWORD DevType, DWORD DevIndex, DWORD CANIndex, VCI_CAN_OBJ* pSend, ULONG Len);
typedef ULONG (WINAPI *pfnVCI_Receive)(DWORD DevType, DWORD DevIndex, DWORD CANIndex, VCI_CAN_OBJ* pReceive, ULONG Len, INT WaitTime);

typedef DWORD (WINAPI *pfnVCI_FindUsbDevice2)(VCI_BOARD_INFO* pInfo);

typedef DWORD (WINAPI *pfnVCI_ReadBoardInfo)(
    DWORD DevType,
    DWORD DevIndex,
    VCI_BOARD_INFO* pInfo);

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void toggleConnection();
    void sendCyclicMessages();
    void readCanData();

private:
    void setupUi();
    bool loadControlCanDll();
    bool openCanDevice();
    void closeCanDevice();
    void processReceivedFrame(uint32_t id, const std::array<uint8_t, 8>& data, uint8_t dlc);
    void sendCanFrame(uint32_t id, const std::array<uint8_t, 8>& data, uint8_t dlc);
    void on_btnConnect_clicked();

    pfnVCI_OpenDevice  fnOpenDevice{nullptr};
    pfnVCI_CloseDevice fnCloseDevice{nullptr};
    pfnVCI_InitCAN     fnInitCAN{nullptr};
    pfnVCI_StartCAN    fnStartCAN{nullptr};
    pfnVCI_Transmit    fnTransmit{nullptr};
    pfnVCI_Receive     fnReceive{nullptr};

    QLibrary m_canLib;

    DWORD m_devType{4};  // VCI_USBCAN2 = 4 , VCI_USBCAN1 = 3
    DWORD m_devIndex{0};
    DWORD m_canIndex{0};
    bool m_isConnected{false};

    QTimer m_txTimer;
    QTimer m_rxTimer;
    kt08::msg::Kt08Database m_db;
    QComboBox *m_devTypeCombo{nullptr};
    QComboBox *m_canChannelCombo{nullptr};
    QPushButton *m_connectBtn{nullptr};
    QComboBox *m_startSwitchCombo{nullptr};
    QDoubleSpinBox *m_ambientTempSpin{nullptr};
    QSpinBox *m_outGasPressureSpin{nullptr};
    QSpinBox *m_compressorSpeedSpin{nullptr};
    QCheckBox *m_acReqStatusCheck{nullptr};
    QDoubleSpinBox *m_coolantTempSpin{nullptr};

    QTableWidget *m_rxTable{nullptr};
};