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
#include <QStackedWidget>
#include <windows.h>
#include <QTime>
#include "KT08Database.h"
#include <QMap>
// --- ساختارها و تعاریف ControlCAN API ---
#define VCI_USBCAN1 3
#define VCI_USBCAN2 4

#pragma pack(push, 1)
typedef struct {
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

typedef DWORD (WINAPI *pfnVCI_OpenDevice)(DWORD, DWORD, DWORD);
typedef DWORD (WINAPI *pfnVCI_CloseDevice)(DWORD, DWORD);
typedef DWORD (WINAPI *pfnVCI_InitCAN)(DWORD, DWORD, DWORD, VCI_INIT_CONFIG*);
typedef DWORD (WINAPI *pfnVCI_StartCAN)(DWORD, DWORD, DWORD);
typedef ULONG (WINAPI *pfnVCI_Transmit)(DWORD, DWORD, DWORD, VCI_CAN_OBJ*, ULONG);
typedef ULONG (WINAPI *pfnVCI_Receive)(DWORD, DWORD, DWORD, VCI_CAN_OBJ*, ULONG, INT);

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
    void onBusTypeChanged(int index);
    void onDataFormatChanged(int index);
    void sendLowSpeedCanFrames();
private:
    void setupUi();
    QMap<uint32_t, int> m_idToRowMap;
    QWidget* createHighSpeedTxPanel();
    QWidget* createLowSpeedTxPanel();
    QWidget* createHighSpeedRxPanel();
    QWidget* createLowSpeedRxPanel();
    QTableWidget *m_logTable;
    QComboBox *m_dataFormatCombo;

    enum DataFormat {
        HexFormat = 0,
        BinaryFormat = 1
    };
    DataFormat m_currentDataFormat = HexFormat;

    bool loadControlCanDll();
    bool openCanDevice();
    void closeCanDevice();
    void processReceivedFrame(uint32_t id, const std::array<uint8_t, 8>& data, uint8_t dlc);
    void sendCanFrame(uint32_t id, const std::array<uint8_t, 8>& data, uint8_t dlc);

    // توابع API
    pfnVCI_OpenDevice  fnOpenDevice{nullptr};
    pfnVCI_CloseDevice fnCloseDevice{nullptr};
    pfnVCI_InitCAN     fnInitCAN{nullptr};
    pfnVCI_StartCAN    fnStartCAN{nullptr};
    pfnVCI_Transmit    fnTransmit{nullptr};
    pfnVCI_Receive     fnReceive{nullptr};

    QLibrary m_canLib;

    DWORD m_devType{4};
    DWORD m_devIndex{0};
    DWORD m_canIndex{0};
    bool m_isConnected{false};

    QTimer m_txTimer;
    QTimer m_rxTimer;
    kt08::msg::Kt08Database m_db;

    //  UI
    QComboBox *m_devTypeCombo{nullptr};
    QComboBox *m_canChannelCombo{nullptr};
    QComboBox *m_busTypeCombo{nullptr};
    QPushButton *m_connectBtn{nullptr};
    QWidget* createLogConsolePanel();
    void applyCustomStyle();


    QStackedWidget *m_txStackedWidget{nullptr};
    QStackedWidget *m_rxStackedWidget{nullptr};

    //  High-Speed (Tx)
    QComboBox *m_startSwitchCombo{nullptr};
    QDoubleSpinBox *m_ambientTempSpin{nullptr};
    QDoubleSpinBox *m_coolantTempSpin{nullptr};
    QSpinBox *m_outGasPressureSpin{nullptr};
    QSpinBox *m_compressorSpeedSpin{nullptr};
    QCheckBox *m_acReqStatusCheck{nullptr};
    QTableWidget *m_hsRxTable{nullptr};

    QComboBox *m_lsStartSwitchCombo{nullptr};        // BcmInfo2
    QComboBox *m_lsEngineStateCombo{nullptr};        // BcmInfo2
    QDoubleSpinBox *m_lsVehicleSpeedSpin{nullptr};   // BcmInfo2 (Kph)

    QCheckBox *m_lsAcStateCheck{nullptr};            // BcmInfo4
    QCheckBox *m_lsRearHeaterCheck{nullptr};         // BcmInfo4
    QDoubleSpinBox *m_lsWaterTempSpin{nullptr};      // BcmInfo4 (°C)
    QDoubleSpinBox *m_lsAmbientTempSpin{nullptr};    // BcmInfo4 (°C)

    QSpinBox *m_lsCompressorRpmSpin{nullptr};        // BcmInfo19 (RPM)
    QSpinBox *m_lsOutGasPressureSpin{nullptr};       // BcmInfo19
    QSpinBox *m_lsInGasPressureSpin{nullptr};        // BcmInfo19
    QSpinBox *m_lsOutGasTempSpin{nullptr};           // BcmInfo19 (°C)
    QSpinBox *m_lsInGasTempSpin{nullptr};            // BcmInfo19 (°C)
    QCheckBox *m_lsShutOffValveCheck{nullptr};       // BcmInfo19

    // --- Low-Speed (BD-CAN) MMS Controls (0x170) ---
    QCheckBox *m_mmsAutoCheck = nullptr;
    QCheckBox *m_mmsAcCheck = nullptr;
    QCheckBox *m_mmsPowerCheck = nullptr;
    QCheckBox *m_mmsIntakeCheck = nullptr;
    QCheckBox *m_mmsTempIncCheck = nullptr;
    QCheckBox *m_mmsTempDecCheck = nullptr;
    QCheckBox *m_mmsBlowerIncCheck = nullptr;
    QCheckBox *m_mmsBlowerDecCheck = nullptr;
    QCheckBox *m_mmsFaceCheck = nullptr;
    QCheckBox *m_mmsFootCheck = nullptr;
    QCheckBox *m_mmsScreenCheck = nullptr;
    QCheckBox *m_mmsDefrostCheck = nullptr;
    QCheckBox *m_mmsRearHeaterCheck = nullptr;
    QCheckBox *m_mmsFootFaceCheck = nullptr;
    QCheckBox *m_mmsFootScreenCheck = nullptr;

    // --- Low-Speed (BD-CAN) BCM Controls ---
    QComboBox *m_lsVehicleTypeCombo = nullptr;


    // --- Low-Speed (BD-CAN) BCM Compressor & Gas Specs (0x1F0) ---

    // --- Low-Speed (BD-CAN) RX Telemetry Table ---


    //  Low-Speed (Rx)
    QTableWidget *m_lsRxTable{nullptr};
    QTableWidget *m_lsFaultTable{nullptr};

    void addLogEntry(const QString &dir, uint32_t id, const std::array<uint8_t, 8> &data, uint8_t dlc);
    QString formatBytes(const std::array<uint8_t, 8> &data, uint8_t dlc, DataFormat format);

};