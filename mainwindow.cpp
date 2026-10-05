#include "mainwindow.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>
#include <QHeaderView>

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

    QGroupBox *connGroup = new QGroupBox("Waveshare USB-CAN Control", this);
    QHBoxLayout *connLayout = new QHBoxLayout(connGroup);

    m_devTypeCombo = new QComboBox(this);
    m_devTypeCombo->addItem("USB-CAN2 (Type 4)", 4);
    m_devTypeCombo->addItem("USB-CAN1 (Type 3)", 3);

    m_canChannelCombo = new QComboBox(this);
    m_canChannelCombo->addItem("CAN 1 (Channel 0)", 0);
    m_canChannelCombo->addItem("CAN 2 (Channel 1)", 1);

    m_busTypeCombo = new QComboBox(this);
    m_busTypeCombo->addItem("High-Speed C-CAN (500 Kbps)", static_cast<int>(CanBusType::HighSpeed));
    m_busTypeCombo->addItem("Low-Speed B-CAN (125 Kbps)", static_cast<int>(CanBusType::LowSpeed));

    connect(m_busTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onBusTypeChanged);

    m_connectBtn = new QPushButton("Connect", this);
    connect(m_connectBtn, &QPushButton::clicked, this, &MainWindow::toggleConnection);

    connLayout->addWidget(new QLabel("Device:"));
    connLayout->addWidget(m_devTypeCombo);
    connLayout->addWidget(new QLabel("Channel:"));
    connLayout->addWidget(m_canChannelCombo);
    connLayout->addWidget(new QLabel("Bus Type:"));
    connLayout->addWidget(m_busTypeCombo);
    connLayout->addWidget(m_connectBtn);

    mainLayout->addWidget(connGroup);

    m_txStackedWidget = new QStackedWidget(this);
    m_txStackedWidget->addWidget(createHighSpeedTxPanel());
    m_txStackedWidget->addWidget(createLowSpeedTxPanel());

    m_rxStackedWidget = new QStackedWidget(this);
    m_rxStackedWidget->addWidget(createHighSpeedRxPanel());
    m_rxStackedWidget->addWidget(createLowSpeedRxPanel());

    mainLayout->addWidget(m_txStackedWidget);
    mainLayout->addWidget(m_rxStackedWidget);

    setCentralWidget(centralWidget);
    setWindowTitle("KT08 HVAC CAN Analyzer");
    resize(720, 850);
}

// *** اصلاح حیاتی ۱: ری‌اینشالایز کردن CAN در صورت تغییر Bus Type در زمان اتصال ***
void MainWindow::onBusTypeChanged(int index)
{
    m_txStackedWidget->setCurrentIndex(index);
    m_rxStackedWidget->setCurrentIndex(index);
    m_db.setBusType(static_cast<CanBusType>(m_busTypeCombo->currentData().toInt()));

    if (m_isConnected) {
        // بازراه‌اندازی دستگاه برای اعمال Baud Rate جدید
        closeCanDevice();
        if (!openCanDevice()) {
            QMessageBox::warning(this, "CAN Warning", "Failed to re-initialize CAN device with new Baud Rate!");
        } else {
            m_txTimer.start(100);
            m_rxTimer.start(10);
        }
    }
}

// Tx High-Speed
QWidget* MainWindow::createHighSpeedTxPanel()
{
    QGroupBox *simGroup = new QGroupBox("High-Speed ECU Simulation (Tx to HVAC)", this);
    QGridLayout *simLayout = new QGridLayout(simGroup);

    m_startSwitchCombo = new QComboBox(this);
    m_startSwitchCombo->addItems({"0: OFF", "1: ACC", "2: IGN", "3: Start Default"});

    m_ambientTempSpin = new QDoubleSpinBox(this);
    m_ambientTempSpin->setRange(-40.0, 86.5);
    m_ambientTempSpin->setValue(25.0);

    m_coolantTempSpin = new QDoubleSpinBox(this);
    m_coolantTempSpin->setRange(-48.0, 141.5);
    m_coolantTempSpin->setValue(80.0);

    m_outGasPressureSpin = new QSpinBox(this);
    m_outGasPressureSpin->setRange(0, 50);
    m_outGasPressureSpin->setValue(15);

    m_compressorSpeedSpin = new QSpinBox(this);
    m_compressorSpeedSpin->setRange(0, 8000);
    m_compressorSpeedSpin->setValue(1200);

    m_acReqStatusCheck = new QCheckBox("HCU_10 AC Request Active", this);

    simLayout->addWidget(new QLabel("BCM_4 Start Switch (0x602):"), 0, 0);
    simLayout->addWidget(m_startSwitchCombo, 0, 1);
    simLayout->addWidget(new QLabel("BCM_5 Ambient Temp °C (0x601):"), 1, 0);
    simLayout->addWidget(m_ambientTempSpin, 1, 1);
    simLayout->addWidget(new QLabel("EMS_3 Coolant Temp °C (0x2F3):"), 2, 0);
    simLayout->addWidget(m_coolantTempSpin, 2, 1);
    simLayout->addWidget(new QLabel("HCU_10 Out Gas Pressure Bar (0x215):"), 3, 0);
    simLayout->addWidget(m_outGasPressureSpin, 3, 1);
    simLayout->addWidget(new QLabel("HCU_10 Compressor Speed RPM:"), 4, 0);
    simLayout->addWidget(m_compressorSpeedSpin, 4, 1);
    simLayout->addWidget(m_acReqStatusCheck, 5, 0, 1, 2);

    return simGroup;
}

QWidget* MainWindow::createLowSpeedTxPanel()
{
    QGroupBox *simGroup = new QGroupBox("Low-Speed BCM Simulation (Tx to HVAC)", this);
    QGridLayout *simLayout = new QGridLayout(simGroup);

    // 1. BcmInfo2 Controls
    m_lsStartSwitchCombo = new QComboBox(this);
    m_lsStartSwitchCombo->addItems({"OFF (0)", "ACC (1)", "IGN (2)", "START (3)"});
    m_lsEngineStateCombo = new QComboBox(this);
    m_lsEngineStateCombo->addItems({"Stop (0)", "Crank (1)", "Running (2)", "Fault (3)"});
    m_lsVehicleSpeedSpin = new QDoubleSpinBox(this);
    m_lsVehicleSpeedSpin->setRange(0.0, 250.0);
    m_lsVehicleSpeedSpin->setValue(0.0);

    // 2. BcmInfo4 Controls
    m_lsAcStateCheck = new QCheckBox("AC State Active", this);
    m_lsRearHeaterCheck = new QCheckBox("Rear Screen Heater Active", this);
    m_lsWaterTempSpin = new QDoubleSpinBox(this);
    m_lsWaterTempSpin->setRange(-48.0, 140.0);
    m_lsWaterTempSpin->setValue(85.0);
    m_lsAmbientTempSpin = new QDoubleSpinBox(this);
    m_lsAmbientTempSpin->setRange(-40.0, 85.0);
    m_lsAmbientTempSpin->setValue(25.0);

    // 3. BcmInfo19 Controls
    m_lsCompressorRpmSpin = new QSpinBox(this);
    m_lsCompressorRpmSpin->setRange(0, 12750);
    m_lsCompressorRpmSpin->setSingleStep(50);
    m_lsCompressorRpmSpin->setValue(1000);

    m_lsOutGasPressureSpin = new QSpinBox(this);
    m_lsOutGasPressureSpin->setRange(0, 255);
    m_lsInGasPressureSpin = new QSpinBox(this);
    m_lsInGasPressureSpin->setRange(0, 255);

    m_lsOutGasTempSpin = new QSpinBox(this);
    m_lsOutGasTempSpin->setRange(-40, 215);
    m_lsInGasTempSpin = new QSpinBox(this);
    m_lsInGasTempSpin->setRange(-40, 215);

    m_lsShutOffValveCheck = new QCheckBox("Shut Off Valve Open", this);

    int row = 0;
    simLayout->addWidget(new QLabel("<b>BcmInfo2:</b>"), row++, 0);
    simLayout->addWidget(new QLabel("Start Switch State:"), row, 0);
    simLayout->addWidget(m_lsStartSwitchCombo, row++, 1);
    simLayout->addWidget(new QLabel("Engine State:"), row, 0);
    simLayout->addWidget(m_lsEngineStateCombo, row++, 1);
    simLayout->addWidget(new QLabel("Vehicle Speed (Kph):"), row, 0);
    simLayout->addWidget(m_lsVehicleSpeedSpin, row++, 1);

    simLayout->addWidget(new QLabel("<b>BcmInfo4:</b>"), row++, 0);
    simLayout->addWidget(m_lsAcStateCheck, row, 0);
    simLayout->addWidget(m_lsRearHeaterCheck, row++, 1);
    simLayout->addWidget(new QLabel("Water Temp (°C):"), row, 0);
    simLayout->addWidget(m_lsWaterTempSpin, row++, 1);
    simLayout->addWidget(new QLabel("Ambient Temp (°C):"), row, 0);
    simLayout->addWidget(m_lsAmbientTempSpin, row++, 1);

    simLayout->addWidget(new QLabel("<b>BcmInfo19:</b>"), row++, 0);
    simLayout->addWidget(new QLabel("Compressor Speed (RPM):"), row, 0);
    simLayout->addWidget(m_lsCompressorRpmSpin, row++, 1);
    simLayout->addWidget(new QLabel("Out Gas Temp (°C):"), row, 0);
    simLayout->addWidget(m_lsOutGasTempSpin, row++, 1);
    simLayout->addWidget(new QLabel("In Gas Temp (°C):"), row, 0);
    simLayout->addWidget(m_lsInGasTempSpin, row++, 1);
    simLayout->addWidget(m_lsShutOffValveCheck, row++, 0, 1, 2);

    return simGroup;
}

// Rx High-Speed
QWidget* MainWindow::createHighSpeedRxPanel()
{
    QGroupBox *rxGroup = new QGroupBox("High-Speed HVAC Monitor (Rx HVAC_1 0x086)", this);
    QVBoxLayout *rxLayout = new QVBoxLayout(rxGroup);

    m_hsRxTable = new QTableWidget(7, 2, this);
    m_hsRxTable->setHorizontalHeaderLabels({"Signal Name", "Live Value"});
    m_hsRxTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

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
        m_hsRxTable->setItem(i, 0, new QTableWidgetItem(signalNames[i]));
        m_hsRxTable->setItem(i, 1, new QTableWidgetItem("N/A"));
    }

    rxLayout->addWidget(m_hsRxTable);
    return rxGroup;
}

// Rx Low-Speed
QWidget* MainWindow::createLowSpeedRxPanel()
{
    QGroupBox *rxGroup = new QGroupBox("Low-Speed HVAC Received Data (Rx)", this);
    QVBoxLayout *rxLayout = new QVBoxLayout(rxGroup);

    m_lsRxTable = new QTableWidget(6, 2, this);
    m_lsRxTable->setHorizontalHeaderLabels({"Low-Speed Parameter (HvacInfo2)", "Live Value"});
    m_lsRxTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QStringList lsSignals = {
        "Cabin Temp (°C)",
        "Set Temp (°C)",
        "Blower Speed",
        "AC Indicator Status",
        "Auto Mode",
        "Power State"
    };

    for (int i = 0; i < lsSignals.size(); ++i) {
        m_lsRxTable->setItem(i, 0, new QTableWidgetItem(lsSignals[i]));
        m_lsRxTable->setItem(i, 1, new QTableWidgetItem("N/A"));
    }

    m_lsFaultTable = new QTableWidget(1, 2, this);
    m_lsFaultTable->setHorizontalHeaderLabels({"Diagnostic (HvacFlt)", "Active Fault Code (DTC)"});
    m_lsFaultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_lsFaultTable->setItem(0, 0, new QTableWidgetItem("Fault DTC Code"));
    m_lsFaultTable->setItem(0, 1, new QTableWidgetItem("No Fault"));

    rxLayout->addWidget(m_lsRxTable);
    rxLayout->addWidget(m_lsFaultTable);

    return rxGroup;
}

// DLL Loading & Connection
bool MainWindow::loadControlCanDll()
{
    if (m_canLib.isLoaded()) return true;

    const QString dllPath = QCoreApplication::applicationDirPath() + "/ControlCAN.dll";
    m_canLib.setFileName(dllPath);

    if (!m_canLib.load()) {
        QMessageBox::critical(this, "ControlCAN Load Error", m_canLib.errorString());
        return false;
    }

    fnOpenDevice  = reinterpret_cast<pfnVCI_OpenDevice>(m_canLib.resolve("VCI_OpenDevice"));
    fnCloseDevice = reinterpret_cast<pfnVCI_CloseDevice>(m_canLib.resolve("VCI_CloseDevice"));
    fnInitCAN     = reinterpret_cast<pfnVCI_InitCAN>(m_canLib.resolve("VCI_InitCAN"));
    fnStartCAN    = reinterpret_cast<pfnVCI_StartCAN>(m_canLib.resolve("VCI_StartCAN"));
    fnTransmit   = reinterpret_cast<pfnVCI_Transmit>(m_canLib.resolve("VCI_Transmit"));
    fnReceive    = reinterpret_cast<pfnVCI_Receive>(m_canLib.resolve("VCI_Receive"));

    return (fnOpenDevice && fnCloseDevice && fnInitCAN && fnStartCAN && fnTransmit && fnReceive);
}

void MainWindow::toggleConnection()
{
    if (m_isConnected) {
        closeCanDevice();
        m_connectBtn->setText("Connect");
        m_devTypeCombo->setEnabled(true);
        m_canChannelCombo->setEnabled(true);
        m_busTypeCombo->setEnabled(true);
        return;
    }

    if (!loadControlCanDll()) return;

    m_devType  = m_devTypeCombo->currentData().toUInt();
    m_canIndex = m_canChannelCombo->currentData().toUInt();

    m_db.setBusType(static_cast<CanBusType>(m_busTypeCombo->currentData().toInt()));

    if (openCanDevice()) {
        m_isConnected = true;
        m_connectBtn->setText("Disconnect");
        m_devTypeCombo->setEnabled(false);
        m_canChannelCombo->setEnabled(false);
        m_busTypeCombo->setEnabled(false);

        m_txTimer.start(100);
        m_rxTimer.start(10);
    } else {
        QMessageBox::critical(this, "Error", "Failed to initialize Waveshare USB-CAN device!");
    }
}

// *** اصلاح حیاتی ۲: تنظیم دقیق Timing0/Timing1 و AccMask ***
bool MainWindow::openCanDevice()
{
    if (!fnOpenDevice) return false;

    if (fnOpenDevice(m_devType, m_devIndex, 0) != 1) return false;

    VCI_INIT_CONFIG config{};
    config.AccCode = 0x00000000;
    config.AccMask = 0xFFFFFFFF; // دریافت تمام فریم‌ها
    config.Filter  = 0;          // Single filter mode (0 = รับทุก فیلتر)
    config.Mode    = 0;          // Normal operation mode

    if (m_db.busType() == CanBusType::HighSpeed) {
        config.Timing0 = 0x00; // 500 Kbps
        config.Timing1 = 0x1C;
    } else {
        config.Timing0 = 0x03; // 125 Kbps
        config.Timing1 = 0x1C;
    }

    if (fnInitCAN(m_devType, m_devIndex, m_canIndex, &config) != 1) {
        fnCloseDevice(m_devType, m_devIndex);
        return false;
    }

    if (fnStartCAN(m_devType, m_devIndex, m_canIndex) != 1) {
        fnCloseDevice(m_devType, m_devIndex);
        return false;
    }

    return true;
}

void MainWindow::closeCanDevice()
{
    m_txTimer.stop();
    m_rxTimer.stop();

    if (m_isConnected) {
        if (fnCloseDevice) fnCloseDevice(m_devType, m_devIndex);
        m_isConnected = false;
    }
}

// Cyclical Transmission & Processing
void MainWindow::sendCyclicMessages()
{
    if (!m_isConnected) return;

    std::array<uint8_t, 8> buffer{};

    if (m_db.busType() == CanBusType::HighSpeed) {
        m_db.bcm4().setStartSwitchStatus(m_startSwitchCombo->currentIndex());
        m_db.bcm4().encode(buffer);
        sendCanFrame(m_db.bcm4().kId, buffer, m_db.bcm4().kDlc);

        m_db.bcm5().setAmbientTempDegC(m_ambientTempSpin->value());
        m_db.bcm5().encode(buffer);
        sendCanFrame(m_db.bcm5().kId, buffer, m_db.bcm5().kDlc);

        m_db.ems3().setEngineCoolantTempDegC(m_coolantTempSpin->value());
        m_db.ems3().encode(buffer);
        sendCanFrame(m_db.ems3().kId, buffer, m_db.ems3().kDlc);

        m_db.hcu10().setOutGasPressure(m_outGasPressureSpin->value());
        m_db.hcu10().setCompressorActualSpeed(m_compressorSpeedSpin->value());
        m_db.hcu10().setAcRequestStatus(m_acReqStatusCheck->isChecked());
        m_db.hcu10().encode(buffer);
        sendCanFrame(m_db.hcu10().kId, buffer, m_db.hcu10().kDlc);
    }
    else {
        // 1. BcmInfo2
        m_db.bcmInfo2().setStartSwitchState(m_lsStartSwitchCombo->currentIndex());
        m_db.bcmInfo2().setEngineState(m_lsEngineStateCombo->currentIndex());
        m_db.bcmInfo2().setInstantVehicleSpeedKph(m_lsVehicleSpeedSpin->value());
        m_db.bcmInfo2().encode(buffer);
        sendCanFrame(m_db.bcmInfo2().kId, buffer, m_db.bcmInfo2().kDlc);

        // 2. BcmInfo4
        m_db.bcmInfo4().setAcState(m_lsAcStateCheck->isChecked() ? 1 : 0);
        m_db.bcmInfo4().setRearScreenHeaterState(m_lsRearHeaterCheck->isChecked() ? 1 : 0);
        m_db.bcmInfo4().setEngineWaterTemperatureC(m_lsWaterTempSpin->value());
        m_db.bcmInfo4().setAmbientTemperatureC(m_lsAmbientTempSpin->value());
        m_db.bcmInfo4().encode(buffer);
        sendCanFrame(m_db.bcmInfo4().kId, buffer, m_db.bcmInfo4().kDlc);

        // 3. BcmInfo19
        m_db.bcmInfo19().setCompressorActualSpeedRpm(m_lsCompressorRpmSpin->value());
        m_db.bcmInfo19().setOutGasPressure(m_lsOutGasPressureSpin->value());
        m_db.bcmInfo19().setInGasPressure(m_lsInGasPressureSpin->value());
        m_db.bcmInfo19().setOutGasTemperatureC(m_lsOutGasTempSpin->value());
        m_db.bcmInfo19().setInGasTemperatureC(m_lsInGasTempSpin->value());
        m_db.bcmInfo19().setShutOffValveStatus(m_lsShutOffValveCheck->isChecked() ? 1 : 0);
        m_db.bcmInfo19().encode(buffer);
        sendCanFrame(m_db.bcmInfo19().kId, buffer, m_db.bcmInfo19().kDlc);
    }
}

// *** اصلاح حیاتی ۳: تنظیم صریح فریم‌های Extended ID برای Low-Speed ***
void MainWindow::sendCanFrame(uint32_t id, const std::array<uint8_t, 8>& data, uint8_t dlc)
{
    if (!fnTransmit) return;

    VCI_CAN_OBJ sendFrame{};
    sendFrame.ID = id;
    sendFrame.SendType = 0;
    sendFrame.DataLen = dlc;

    if (m_db.busType() == CanBusType::LowSpeed) {
        sendFrame.ExternFlag = 1; // 1 برای Extended 29-bit ID
        sendFrame.RemoteFlag = 0;
    } else {
        sendFrame.ExternFlag = 0; // 0 برای Standard 11-bit ID
        sendFrame.RemoteFlag = 0;
    }

    for (int i = 0; i < dlc; ++i) {
        sendFrame.Data[i] = data[i];
    }

    fnTransmit(m_devType, m_devIndex, m_canIndex, &sendFrame, 1);
}

void MainWindow::readCanData()
{
    if (!m_isConnected || !fnReceive) return;

    VCI_CAN_OBJ rxFrames[50]{};
    const ULONG len = fnReceive(m_devType, m_devIndex, m_canIndex, rxFrames, 50, 0);

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
    ccl::msg::ICanMessage* msg = m_db.find(id);
    if (!msg) return;

    msg->decode(data, dlc);

    // --- High-Speed Decoding ---
    if (m_db.busType() == CanBusType::HighSpeed && id == kt08::msg::Hvac1::kId) {
        m_hsRxTable->item(0, 1)->setText(QString::number(m_db.hvac1().actualEvaporatorTempDegC(), 'f', 1));
        m_hsRxTable->item(1, 1)->setText(QString::number(m_db.hvac1().targetEvaporatorTempDegC(), 'f', 1));
        m_hsRxTable->item(2, 1)->setText(m_db.hvac1().acRequest() ? "ON" : "OFF");
        m_hsRxTable->item(3, 1)->setText(m_db.hvac1().rearDefrostRequestSwitch() ? "ON" : "OFF");
        m_hsRxTable->item(4, 1)->setText(m_db.hvac1().heaterRequest() ? "ON" : "OFF");
        m_hsRxTable->item(5, 1)->setText(QString::number(m_db.hvac1().targetCabinTemp()));
        m_hsRxTable->item(6, 1)->setText(QString::number(m_db.hvac1().actualCabinTempDegC(), 'f', 1));
    }
    // --- Low-Speed Decoding ---
    else if (m_db.busType() == CanBusType::LowSpeed) {
        if (id == kt08::msg::HvacInfo2::kId) {
            m_lsRxTable->item(0, 1)->setText(QString::number(m_db.hvacInfo2().cabinTemperatureC(), 'f', 1) + " °C");
            m_lsRxTable->item(1, 1)->setText(QString::number(m_db.hvacInfo2().temperatureSetC(), 'f', 1) + " °C");
            m_lsRxTable->item(2, 1)->setText(QString::number(m_db.hvacInfo2().blowerSpeed()));
            m_lsRxTable->item(3, 1)->setText(m_db.hvacInfo2().acIndicator() ? "ON" : "OFF");
            m_lsRxTable->item(4, 1)->setText(m_db.hvacInfo2().autoMode() ? "ON" : "OFF");
            m_lsRxTable->item(5, 1)->setText(m_db.hvacInfo2().powerState() ? "ON" : "OFF");
        }
        else if (id == kt08::msg::HvacFlt::kId) {
            m_lsFaultTable->item(0, 1)->setText(QString("0x%1").arg(m_db.hvacFlt().faultDtc(), 6, 16, QChar('0')).toUpper());
        }
    }
}