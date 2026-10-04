#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    connect(&m_txTimer, &QTimer::timeout, this, &MainWindow::sendCyclicMessages);
    connect(&m_rxTimer, &QTimer::timeout, this, &MainWindow::readCanData);
}

MainWindow::~MainWindow()
{
    closeCanDevice();
}

void MainWindow::setupUi()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // ۱. پنل اتصال USB-CAN
    QGroupBox *connGroup = new QGroupBox("Waveshare USB-CAN Control (DLL Mode)", this);
    QHBoxLayout *connLayout = new QHBoxLayout(connGroup);

    m_devTypeCombo = new QComboBox(this);
    m_devTypeCombo->addItem("USB-CAN2 (Type 4)", 4);
    m_devTypeCombo->addItem("USB-CAN1 (Type 3)", 3);

    m_canChannelCombo = new QComboBox(this);
    m_canChannelCombo->addItem("CAN 1 (Channel 0)", 0);
    m_canChannelCombo->addItem("CAN 2 (Channel 1)", 1);

    m_connectBtn = new QPushButton("Connect", this);
    connect(m_connectBtn, &QPushButton::clicked, this, &MainWindow::toggleConnection);

    connLayout->addWidget(new QLabel("Device Type:"));
    connLayout->addWidget(m_devTypeCombo);
    connLayout->addWidget(new QLabel("Channel:"));
    connLayout->addWidget(m_canChannelCombo);
    connLayout->addWidget(m_connectBtn);
    mainLayout->addWidget(connGroup);

    // ۲. پنل شبیه‌سازی سیگنال‌های خودرو (Tx)
    QGroupBox *simGroup = new QGroupBox("ECU Signal Simulation (Tx to HVAC)", this);
    QGridLayout *simLayout = new QGridLayout(simGroup);

    m_startSwitchCombo = new QComboBox(this);
    m_startSwitchCombo->addItems({"0: OFF", "1: ACC", "2: IGN", "3: Start Default"});
    simLayout->addWidget(new QLabel("BCM_4 Start Switch (0x602):"), 0, 0);
    simLayout->addWidget(m_startSwitchCombo, 0, 1);

    m_ambientTempSpin = new QDoubleSpinBox(this);
    m_ambientTempSpin->setRange(-40.0, 86.5);
    m_ambientTempSpin->setValue(25.0);
    m_ambientTempSpin->setSingleStep(0.5);
    simLayout->addWidget(new QLabel("BCM_5 Ambient Temp °C (0x601):"), 1, 0);
    simLayout->addWidget(m_ambientTempSpin, 1, 1);

    m_coolantTempSpin = new QDoubleSpinBox(this);
    m_coolantTempSpin->setRange(-48.0, 141.5);
    m_coolantTempSpin->setValue(80.0);
    simLayout->addWidget(new QLabel("EMS_3 Coolant Temp °C (0x2F3):"), 2, 0);
    simLayout->addWidget(m_coolantTempSpin, 2, 1);

    m_outGasPressureSpin = new QSpinBox(this);
    m_outGasPressureSpin->setRange(0, 50);
    m_outGasPressureSpin->setValue(15);
    simLayout->addWidget(new QLabel("HCU_10 Out Gas Pressure Bar (0x215):"), 3, 0);
    simLayout->addWidget(m_outGasPressureSpin, 3, 1);

    m_compressorSpeedSpin = new QSpinBox(this);
    m_compressorSpeedSpin->setRange(0, 8000);
    m_compressorSpeedSpin->setValue(1200);
    simLayout->addWidget(new QLabel("HCU_10 Compressor Speed RPM:"), 4, 0);
    simLayout->addWidget(m_compressorSpeedSpin, 4, 1);

    m_acReqStatusCheck = new QCheckBox("HCU_10 AC Request Active", this);
    simLayout->addWidget(m_acReqStatusCheck, 5, 0, 1, 2);

    mainLayout->addWidget(simGroup);

    // ۳. جدول مانیتورینگ خروجی HVAC (Rx)
    QGroupBox *rxGroup = new QGroupBox("HVAC Output Monitor (Rx HVAC_1 0x086)", this);
    QVBoxLayout *rxLayout = new QVBoxLayout(rxGroup);

    m_rxTable = new QTableWidget(7, 2, this);
    m_rxTable->setHorizontalHeaderLabels({"Signal Name", "Live Value"});
    m_rxTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QStringList signalNames = {
        "Actual Evaporator Temp (°C)",
        "Target Evaporator Temp (°C)",
        "AC Request",
        "Rear Defrost Request",
        "Heater Request",
        "Target Cabin Temp Code",
        "Actual Cabin Temp (°C)"
    };

    for (int i = 0; i < signalNames.size(); ++i) {
        m_rxTable->setItem(i, 0, new QTableWidgetItem(signalNames[i]));
        m_rxTable->setItem(i, 1, new QTableWidgetItem("N/A"));
    }

    rxLayout->addWidget(m_rxTable);
    mainLayout->addWidget(rxGroup);

    setCentralWidget(centralWidget);
    setWindowTitle("KT08 HVAC CAN Analyzer — Dynamic ControlCAN");
    resize(600, 750);
}
bool MainWindow::loadControlCanDll()
{
    const QString dllPath =
        QCoreApplication::applicationDirPath()
        + "/ControlCAN.dll";

    m_canLib.setFileName(dllPath);

    qDebug() << "DLL path:" << dllPath;
    qDebug() << "DLL exists:" << QFileInfo::exists(dllPath);

    const bool loadOk = m_canLib.load();
    qDebug() << "Loaded DLL:" << m_canLib.fileName();
    qDebug() << "Load result:" << loadOk;
    qDebug() << "Is loaded:" << m_canLib.isLoaded();
    qDebug() << "Error:" << m_canLib.errorString();

    if (!loadOk) {
        QMessageBox::critical(
            this,
            "ControlCAN Load Error",
            QString("Path: %1\nError: %2")
                .arg(dllPath, m_canLib.errorString())
            );
        return false;
    }


    // نگاشت توابع از DLL
    fnOpenDevice  = (pfnVCI_OpenDevice)  m_canLib.resolve("VCI_OpenDevice");
    fnCloseDevice = (pfnVCI_CloseDevice) m_canLib.resolve("VCI_CloseDevice");
    fnInitCAN     = (pfnVCI_InitCAN)     m_canLib.resolve("VCI_InitCAN");
    fnStartCAN    = (pfnVCI_StartCAN)    m_canLib.resolve("VCI_StartCAN");
    fnTransmit    = (pfnVCI_Transmit)    m_canLib.resolve("VCI_Transmit");
    fnReceive     = (pfnVCI_Receive)     m_canLib.resolve("VCI_Receive");

    bool allResolved = (fnOpenDevice && fnCloseDevice && fnInitCAN && fnStartCAN && fnTransmit && fnReceive);
    qDebug() << "OpenDevice:" << ((fnOpenDevice )!= nullptr);
    qDebug() << "CloseDevice:" << (fnCloseDevice != nullptr);
    qDebug() << "InitCAN:" << (fnInitCAN != nullptr);
    qDebug() << "StartCAN:" << (fnStartCAN != nullptr);
    qDebug() << "Transmit:" << (fnTransmit != nullptr);
    qDebug() << "Receive:" << (fnReceive != nullptr);


    if (!allResolved) {
        QMessageBox::critical(
            this,
            "DLL Function Error",
            QString(
                "OpenDevice: %1\n"
                "CloseDevice: %2\n"
                "InitCAN: %3\n"
                "StartCAN: %4\n"
                "Transmit: %5\n"
                "Receive: %6"
                )
                .arg(fnOpenDevice != nullptr)
                .arg(fnCloseDevice != nullptr)
                .arg(fnInitCAN != nullptr)
                .arg(fnStartCAN != nullptr)
                .arg(fnTransmit != nullptr)
                .arg(fnReceive != nullptr)
            );
        return false;
    }

    return allResolved;
}

void MainWindow::toggleConnection()
{
    if (m_isConnected) {
        closeCanDevice();
        m_connectBtn->setText("Connect");
        m_devTypeCombo->setEnabled(true);
        m_canChannelCombo->setEnabled(true);
    } else {
        if (!loadControlCanDll()) {
            return;
        }

        m_devType = m_devTypeCombo->currentData().toUInt();
        m_canIndex = m_canChannelCombo->currentData().toUInt();

        if (openCanDevice()) {
            m_isConnected = true;
            m_connectBtn->setText("Disconnect");
            m_devTypeCombo->setEnabled(false);
            m_canChannelCombo->setEnabled(false);

            m_txTimer.start(100);
            m_rxTimer.start(10);
        } else {
            QMessageBox::critical(this, "Error", "Failed to initialize Waveshare USB-CAN device!");
        }
    }
}

bool MainWindow::openCanDevice()
{
    const DWORD openResult =
        fnOpenDevice(m_devType, m_devIndex, 0);

    qDebug() << "Device type:" << m_devType;
    qDebug() << "Device index:" << m_devIndex;
    qDebug() << "VCI_OpenDevice result:" << Qt::hex << openResult;
    const DWORD winError = GetLastError();

    qDebug() << "VCI_OpenDevice result:" << openResult;
    qDebug() << "Windows error:" << winError;
    if (openResult != 1) {
        QMessageBox::critical(
            this, "VCI_OpenDevice Failed",
            QString("Return code: %1 (0x%2)\n"
                    "Device Type: %3\n"
                    "Device Index: %4")
                .arg(openResult)
                .arg(openResult, 0, 16)
                .arg(m_devType)
                .arg(m_devIndex)
            );
        return false;
    }

    VCI_INIT_CONFIG config{};
    config.AccCode = 0x00000000;
    config.AccMask = 0xFFFFFFFF;
    config.Filter  = 1;
    config.Timing0 = 0x00;
    config.Timing1 = 0x1C;
    config.Mode    = 0;

    const DWORD initResult =
        fnInitCAN(m_devType, m_devIndex, m_canIndex, &config);

    if (initResult != 1) {
        fnCloseDevice(m_devType, m_devIndex);

        QMessageBox::critical(
            this, "VCI_InitCAN Failed",
            QString("Return code: %1 (0x%2)\n"
                    "Device Type: %3\n"
                    "Device Index: %4\n"
                    "CAN Channel: %5")
                .arg(initResult)
                .arg(initResult, 0, 16)
                .arg(m_devType)
                .arg(m_devIndex)
                .arg(m_canIndex)
            );
        return false;
    }

    const DWORD startResult =
        fnStartCAN(m_devType, m_devIndex, m_canIndex);

    if (startResult != 1) {
        fnCloseDevice(m_devType, m_devIndex);

        QMessageBox::critical(
            this, "VCI_StartCAN Failed",
            QString("Return code: %1 (0x%2)\n"
                    "CAN Channel: %3")
                .arg(startResult)
                .arg(startResult, 0, 16)
                .arg(m_canIndex)
            );
        return false;
    }

    return true;
}


void MainWindow::closeCanDevice()
{
    m_txTimer.stop();
    m_rxTimer.stop();

    if (m_isConnected) {
        if (fnCloseDevice) {
            fnCloseDevice(m_devType, m_devIndex);
        }
        m_isConnected = false;
    }
}

void MainWindow::sendCyclicMessages()
{
    if (!m_isConnected) return;

    std::array<uint8_t, 8> buffer{};

    // BCM_4
    m_db.bcm4().setStartSwitchStatus(m_startSwitchCombo->currentIndex());
    m_db.bcm4().encode(buffer);
    sendCanFrame(m_db.bcm4().kId, buffer, m_db.bcm4().kDlc);

    // BCM_5
    m_db.bcm5().setAmbientTempDegC(m_ambientTempSpin->value());
    m_db.bcm5().encode(buffer);
    sendCanFrame(m_db.bcm5().kId, buffer, m_db.bcm5().kDlc);

    // EMS_3
    //m_db.ems3().setEngineCoolantTempDegC(m_coolantTempSpin->value());
    m_db.ems3().encode(buffer);
    sendCanFrame(m_db.ems3().kId, buffer, m_db.ems3().kDlc);

    // HCU_10
  //  m_db.hcu10().setOutGasPressure(m_outGasPressureSpin->value());
  //  m_db.hcu10().setCompressorActualSpeed(m_compressorSpeedSpin->value());
  //  m_db.hcu10().setAcRequestStatus(m_acReqStatusCheck->isChecked());
    m_db.hcu10().encode(buffer);
    sendCanFrame(m_db.hcu10().kId, buffer, m_db.hcu10().kDlc);
}

void MainWindow::sendCanFrame(uint32_t id, const std::array<uint8_t, 8>& data, uint8_t dlc)
{
    VCI_CAN_OBJ sendFrame;
    memset(&sendFrame, 0, sizeof(sendFrame));

    sendFrame.ID = id;
    sendFrame.SendType = 0;
    sendFrame.RemoteFlag = 0;
    sendFrame.ExternFlag = 0;
    sendFrame.DataLen = dlc;

    for (int i = 0; i < dlc; ++i) {
        sendFrame.Data[i] = data[i];
    }

    fnTransmit(m_devType, m_devIndex, m_canIndex, &sendFrame, 1);
}

void MainWindow::readCanData()
{
    if (!m_isConnected) return;

    VCI_CAN_OBJ rxFrames[50];
    ULONG len = fnReceive(m_devType, m_devIndex, m_canIndex, rxFrames, 50, 0);

    for (ULONG i = 0; i < len; ++i) {
        if (rxFrames[i].RemoteFlag == 0) {
            std::array<uint8_t, 8> data{};
            for (int j = 0; j < rxFrames[i].DataLen; ++j) {
                data[j] = rxFrames[i].Data[j];
            }
            processReceivedFrame(rxFrames[i].ID, data, rxFrames[i].DataLen);
        }
    }
}

void MainWindow::processReceivedFrame(uint32_t id, const std::array<uint8_t, 8>& data, uint8_t dlc)
{
    if (id == kt08::msg::Hvac1::kId) {
        m_db.hvac1().decode(data, dlc);

        m_rxTable->item(0, 1)->setText(QString::number(m_db.hvac1().actualEvaporatorTempDegC(), 'f', 1));
        m_rxTable->item(1, 1)->setText(QString::number(m_db.hvac1().targetEvaporatorTempDegC(), 'f', 1));
        m_rxTable->item(2, 1)->setText(m_db.hvac1().acRequest() ? "ON" : "OFF");
        m_rxTable->item(3, 1)->setText(m_db.hvac1().rearDefrostRequestSwitch() ? "ON" : "OFF");
        m_rxTable->item(4, 1)->setText(m_db.hvac1().heaterRequest() ? "ON" : "OFF");
        m_rxTable->item(5, 1)->setText(QString::number(m_db.hvac1().targetCabinTemp()));
        m_rxTable->item(6, 1)->setText(QString::number(m_db.hvac1().actualCabinTempDegC(), 'f', 1));
    }
}