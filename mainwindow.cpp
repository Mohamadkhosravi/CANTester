#include "mainwindow.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>
#include <QHeaderView>
#include <QMap>
#include <QSplitter>
#include <QTabWidget>
#include <QTime>

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
    mainLayout->setContentsMargins(8, 8, 8, 8);


    QWidget *topWidget = new QWidget(this);
    QVBoxLayout *topLayout = new QVBoxLayout(topWidget);
    topLayout->setContentsMargins(0, 0, 0, 0);

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

    m_txStackedWidget = new QStackedWidget(this);
    m_txStackedWidget->addWidget(createHighSpeedTxPanel());
    m_txStackedWidget->addWidget(createLowSpeedTxPanel());

    m_rxStackedWidget = new QStackedWidget(this);
    m_rxStackedWidget->addWidget(createHighSpeedRxPanel());
    m_rxStackedWidget->addWidget(createLowSpeedRxPanel());


    QWidget *panelsWidget = new QWidget(this);
    QHBoxLayout *panelsLayout = new QHBoxLayout(panelsWidget);
    panelsLayout->setContentsMargins(0, 0, 0, 0);

    panelsLayout->addWidget(m_txStackedWidget); //  TX Panel
    panelsLayout->addWidget(m_rxStackedWidget); //  RX Panel


    topLayout->addWidget(connGroup);
    topLayout->addWidget(panelsWidget);

    // Splitter
    QSplitter *mainSplitter = new QSplitter(Qt::Vertical, this);
    mainSplitter->addWidget(topWidget);
    mainSplitter->addWidget(createLogConsolePanel());

    mainSplitter->setStretchFactor(0, 4);
    mainSplitter->setStretchFactor(1, 6);

    mainLayout->addWidget(mainSplitter);

    setCentralWidget(centralWidget);
    setWindowTitle("KT08 HVAC CAN Analyzer");
    resize(850, 900);

    // applyCustomStyle();
}

void MainWindow::applyCustomStyle()
{
    this->setStyleSheet(R"(
        QMainWindow {
            background-color: #EAECEE;
            font-family: 'Segoe UI', Arial, sans-serif;
            font-size: 13px;
        }
        QGroupBox {
            font-weight: bold;
            font-size: 13px;
            color: #1F2C39;
            border: 1px solid #BDC3C7;
            border-radius: 6px;
            margin-top: 12px;
            padding-top: 12px;
            background-color: #FFFFFF;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 12px;
            padding: 0 6px;
            background-color: #FFFFFF;
            color: #2980B9;
        }
        QPushButton {
            background-color: #2980B9;
            color: white;
            border-radius: 4px;
            padding: 6px 16px;
            font-weight: bold;
            border: none;
        }
        QPushButton:hover {
            background-color: #3498DB;
        }
        QPushButton:pressed {
            background-color: #1C5980;
        }
        QTableWidget {
            background-color: #FFFFFF;
            gridline-color: #E5E8E8;
            border: 1px solid #BDC3C7;
            border-radius: 4px;
            selection-background-color: #D4E6F1;
            selection-color: #000000;
            alternate-background-color: #F8F9F9;
        }
        QHeaderView::section {
            background-color: #2C3E50;
            color: #FFFFFF;
            padding: 6px;
            font-weight: bold;
            border: none;
        }
        QComboBox, QSpinBox, QDoubleSpinBox {
            border: 1px solid #BDC3C7;
            border-radius: 4px;
            padding: 4px 8px;
            background: #FAFAFA;
            min-height: 22px;
        }
        QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus {
            border: 1px solid #2980B9;
            background: #FFFFFF;
        }
        QSplitter::handle {
            background-color: #BDC3C7;
            height: 4px;
            margin: 2px 0;
            border-radius: 2px;
        }
        QSplitter::handle:hover {
            background-color: #2980B9;
        }
    )");
}

void MainWindow::onBusTypeChanged(int index)
{
    m_txStackedWidget->setCurrentIndex(index);
    m_rxStackedWidget->setCurrentIndex(index);
    m_db.setBusType(static_cast<CanBusType>(m_busTypeCombo->currentData().toInt()));

    if (m_isConnected) {
        closeCanDevice();
        if (!openCanDevice()) {
            QMessageBox::warning(this, "CAN Warning", "Failed to re-initialize CAN device with new Baud Rate!");
        } else {
            m_txTimer.start(100);
            m_rxTimer.start(10);
        }
    }
}

// Log Console
QWidget* MainWindow::createLogConsolePanel()
{
    QGroupBox *logGroup = new QGroupBox("CAN Data Log Console (Overwrite Mode)", this);
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);

    QHBoxLayout *topBarLayout = new QHBoxLayout();
    QLabel *formatLabel = new QLabel("Data Display Format:", this);
    m_dataFormatCombo = new QComboBox(this);
    m_dataFormatCombo->addItem("HEX", HexFormat);
    m_dataFormatCombo->addItem("BINARY", BinaryFormat);

    connect(m_dataFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onDataFormatChanged);

    QPushButton *clearLogBtn = new QPushButton("Clear Log", this);
    connect(clearLogBtn, &QPushButton::clicked, this, [this]() {
        m_logTable->setRowCount(0);
        m_idToRowMap.clear();
    });

    topBarLayout->addWidget(formatLabel);
    topBarLayout->addWidget(m_dataFormatCombo);
    topBarLayout->addStretch();
    topBarLayout->addWidget(clearLogBtn);

    m_logTable = new QTableWidget(0, 7, this);
    m_logTable->setHorizontalHeaderLabels({"Last Time", "Dir", "ID (Hex)", "Type", "DLC", "Data Payload", "Count"});
    m_logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QHeaderView *header = m_logTable->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Time
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Dir
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents); // ID
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Type
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents); // DLC
    header->setSectionResizeMode(6, QHeaderView::ResizeToContents); // Count
    header->setSectionResizeMode(5, QHeaderView::Stretch);

    logLayout->addLayout(topBarLayout);
    logLayout->addWidget(m_logTable);

    return logGroup;
}

//Hex Binary
QString MainWindow::formatBytes(const std::array<uint8_t, 8> &data, uint8_t dlc, DataFormat format)
{
    QStringList result;
    for (int i = 0; i < dlc; ++i) {
        if (format == HexFormat) {
            result << QString("%1").arg(data[i], 2, 16, QChar('0')).toUpper();
        } else {
            result << QString("%1").arg(data[i], 8, 2, QChar('0'));
        }
    }
    return result.join(" ");
}


void MainWindow::addLogEntry(const QString &dir, uint32_t id, const std::array<uint8_t, 8> &data, uint8_t dlc)
{
    QString timeStr = QTime::currentTime().toString("hh:mm:ss.z");
    QString idStr = QString("0x%1").arg(id, (m_db.busType() == CanBusType::LowSpeed ? 8 : 3), 16, QChar('0')).toUpper();
    QString typeStr = (m_db.busType() == CanBusType::LowSpeed) ? "EXT (29-bit)" : "STD (11-bit)";
    QString formattedData = formatBytes(data, dlc, m_currentDataFormat);

    if (m_idToRowMap.contains(id)) {
        int targetRow = m_idToRowMap[id];

        m_logTable->item(targetRow, 0)->setText(timeStr);
        m_logTable->item(targetRow, 1)->setText(dir);

        if (dir == "TX") {
            m_logTable->item(targetRow, 1)->setForeground(Qt::blue);
        } else {
            m_logTable->item(targetRow, 1)->setForeground(Qt::darkGreen);
        }

        m_logTable->item(targetRow, 3)->setText(typeStr);
        m_logTable->item(targetRow, 4)->setText(QString::number(dlc));
        m_logTable->item(targetRow, 5)->setText(formattedData);

        int currentCount = m_logTable->item(targetRow, 6)->text().toInt();
        m_logTable->item(targetRow, 6)->setText(QString::number(currentCount + 1));
    } else {
        int newRow = m_logTable->rowCount();
        m_logTable->insertRow(newRow);

        m_logTable->setItem(newRow, 0, new QTableWidgetItem(timeStr));

        QTableWidgetItem *dirItem = new QTableWidgetItem(dir);
        if (dir == "TX") {
            dirItem->setForeground(Qt::blue);
        } else {
            dirItem->setForeground(Qt::darkGreen);
        }
        m_logTable->setItem(newRow, 1, dirItem);

        m_logTable->setItem(newRow, 2, new QTableWidgetItem(idStr));
        m_logTable->setItem(newRow, 3, new QTableWidgetItem(typeStr));
        m_logTable->setItem(newRow, 4, new QTableWidgetItem(QString::number(dlc)));
        m_logTable->setItem(newRow, 5, new QTableWidgetItem(formattedData));
        m_logTable->setItem(newRow, 6, new QTableWidgetItem("1"));

        m_idToRowMap[id] = newRow;
    }
}

// Hex/Binary ComboBox
void MainWindow::onDataFormatChanged(int index)
{
    m_currentDataFormat = static_cast<DataFormat>(m_dataFormatCombo->currentData().toInt());

    for (int row = 0; row < m_logTable->rowCount(); ++row) {
        uint8_t dlc = m_logTable->item(row, 4)->text().toUInt();
        QString currentDataStr = m_logTable->item(row, 5)->text();
        QStringList byteTokens = currentDataStr.split(" ");
        std::array<uint8_t, 8> dataBuffer{};

        for (int i = 0; i < byteTokens.size() && i < dlc; ++i) {
            bool ok = false;
            if (m_currentDataFormat == HexFormat) {
                dataBuffer[i] = static_cast<uint8_t>(byteTokens[i].toUInt(&ok, 2));
            } else {
                dataBuffer[i] = static_cast<uint8_t>(byteTokens[i].toUInt(&ok, 16));
            }
        }

        m_logTable->item(row, 5)->setText(formatBytes(dataBuffer, dlc, m_currentDataFormat));
    }
}

// Tx High-Speed Panel
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

// Tx Low-Speed Panel
QWidget* MainWindow::createLowSpeedTxPanel()
{
    QGroupBox *simGroup = new QGroupBox("BD-CAN Transmitter Simulation (MMS & BCM -> HVAC)", this);
    QVBoxLayout *mainLayout = new QVBoxLayout(simGroup);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    QTabWidget *txTabs = new QTabWidget(this);

    // Tab 1: MMS Buttons (MMS_INFO3 - 0x170)
    QWidget *mmsTab = new QWidget(this);
    QGridLayout *mmsGrid = new QGridLayout(mmsTab);
    mmsGrid->setSpacing(4);

    mmsGrid->addWidget(m_mmsAutoCheck = new QCheckBox("AUTO Button"), 0, 0);
    mmsGrid->addWidget(m_mmsAcCheck = new QCheckBox("AC Button"), 0, 1);
    mmsGrid->addWidget(m_mmsPowerCheck = new QCheckBox("POWER Button"), 0, 2);
    mmsGrid->addWidget(m_mmsIntakeCheck = new QCheckBox("Intake (Recirc) Button"), 0, 3);

    mmsGrid->addWidget(m_mmsTempIncCheck = new QCheckBox("Temp + Button"), 1, 0);
    mmsGrid->addWidget(m_mmsTempDecCheck = new QCheckBox("Temp - Button"), 1, 1);
    mmsGrid->addWidget(m_mmsBlowerIncCheck = new QCheckBox("Blower + Button"), 1, 2);
    mmsGrid->addWidget(m_mmsBlowerDecCheck = new QCheckBox("Blower - Button"), 1, 3);

    mmsGrid->addWidget(m_mmsFaceCheck = new QCheckBox("Face Mode"), 2, 0);
    mmsGrid->addWidget(m_mmsFootCheck = new QCheckBox("Foot Mode"), 2, 1);
    mmsGrid->addWidget(m_mmsScreenCheck = new QCheckBox("Screen Mode"), 2, 2);
    mmsGrid->addWidget(m_mmsDefrostCheck = new QCheckBox("Front Defrost"), 2, 3);

    mmsGrid->addWidget(m_mmsRearHeaterCheck = new QCheckBox("Rear Defrost"), 3, 0);
    mmsGrid->addWidget(m_mmsFootFaceCheck = new QCheckBox("Foot/Face Mode"), 3, 1);
    mmsGrid->addWidget(m_mmsFootScreenCheck = new QCheckBox("Foot/Screen Mode"), 3, 2);

    txTabs->addTab(mmsTab, "MMS Controls (0x170)");

    // Tab 2: BCM States (BCM_INFO2 / INFO4 / INFO5)
    QWidget *bcmTab = new QWidget(this);
    QGridLayout *bcmGrid = new QGridLayout(bcmTab);
    bcmGrid->setSpacing(6);

    int row = 0;
    bcmGrid->addWidget(new QLabel("Vehicle Type:"), row, 0);
    m_lsVehicleTypeCombo = new QComboBox(this);
    m_lsVehicleTypeCombo->addItems({"Petrol (0)", "Hybrid (1)"});
    bcmGrid->addWidget(m_lsVehicleTypeCombo, row, 1);

    bcmGrid->addWidget(new QLabel("Start Switch:"), row, 2);
    m_lsStartSwitchCombo = new QComboBox(this);
    m_lsStartSwitchCombo->addItems({"OFF (0)", "ACC (1)", "IGN (2)", "CRANK (3)"});
    bcmGrid->addWidget(m_lsStartSwitchCombo, row, 3);
    row++;

    bcmGrid->addWidget(new QLabel("Engine State:"), row, 0);
    m_lsEngineStateCombo = new QComboBox(this);
    m_lsEngineStateCombo->addItems({"Stop (0)", "Crank (1)", "Running (2)", "Error (3)"});
    bcmGrid->addWidget(m_lsEngineStateCombo, row, 1);

    bcmGrid->addWidget(new QLabel("Vehicle Speed (Km/h):"), row, 2);
    m_lsVehicleSpeedSpin = new QDoubleSpinBox(this);
    m_lsVehicleSpeedSpin->setRange(0.0, 250.0);
    bcmGrid->addWidget(m_lsVehicleSpeedSpin, row, 3);
    row++;

    bcmGrid->addWidget(new QLabel("Engine Water Temp (°C):"), row, 0);
    m_lsWaterTempSpin = new QDoubleSpinBox(this);
    m_lsWaterTempSpin->setRange(-48.0, 142.5);
    m_lsWaterTempSpin->setValue(85.0);
    bcmGrid->addWidget(m_lsWaterTempSpin, row, 1);

    bcmGrid->addWidget(new QLabel("Ambient Temp (°C):"), row, 2);
    m_lsAmbientTempSpin = new QDoubleSpinBox(this);
    m_lsAmbientTempSpin->setRange(-40.0, 86.5);
    m_lsAmbientTempSpin->setValue(25.0);
    bcmGrid->addWidget(m_lsAmbientTempSpin, row, 3);
    row++;

    m_lsAcStateCheck = new QCheckBox("BCM AC Engaged", this);
    m_lsRearHeaterCheck = new QCheckBox("BCM Rear Heater ON", this);
    bcmGrid->addWidget(m_lsAcStateCheck, row, 0, 1, 2);
    bcmGrid->addWidget(m_lsRearHeaterCheck, row, 2, 1, 2);

    txTabs->addTab(bcmTab, "BCM Info (0x088/0x0C8/0x0E8)");

    // Tab 3: BCM Compressor & Gas (BCM_INFO19 - 0x1F0)
    QWidget *bcm19Tab = new QWidget(this);
    QGridLayout *bcm19Grid = new QGridLayout(bcm19Tab);
    bcm19Grid->setSpacing(6);

    row = 0;
    bcm19Grid->addWidget(new QLabel("Compressor Speed (RPM):"), row, 0);
    m_lsCompressorRpmSpin = new QSpinBox(this);
    m_lsCompressorRpmSpin->setRange(0, 12700);
    m_lsCompressorRpmSpin->setSingleStep(50);
    bcm19Grid->addWidget(m_lsCompressorRpmSpin, row, 1);

    bcm19Grid->addWidget(new QLabel("Out Gas Press (Bar):"), row, 2);
    m_lsOutGasPressureSpin = new QSpinBox(this);
    m_lsOutGasPressureSpin->setRange(0, 30);
    bcm19Grid->addWidget(m_lsOutGasPressureSpin, row, 3);
    row++;

    bcm19Grid->addWidget(new QLabel("In Gas Press (Bar):"), row, 0);
    m_lsInGasPressureSpin = new QSpinBox(this);
    m_lsInGasPressureSpin->setRange(0, 50);
    bcm19Grid->addWidget(m_lsInGasPressureSpin, row, 1);

    bcm19Grid->addWidget(new QLabel("Out Gas Temp (°C):"), row, 2);
    m_lsOutGasTempSpin = new QSpinBox(this);
    m_lsOutGasTempSpin->setRange(-40, 150);
    bcm19Grid->addWidget(m_lsOutGasTempSpin, row, 3);
    row++;

    bcm19Grid->addWidget(new QLabel("In Gas Temp (°C):"), row, 0);
    m_lsInGasTempSpin = new QSpinBox(this);
    m_lsInGasTempSpin->setRange(-40, 150);
    bcm19Grid->addWidget(m_lsInGasTempSpin, row, 1);

    m_lsShutOffValveCheck = new QCheckBox("ShutOff Valve Active", this);
    bcm19Grid->addWidget(m_lsShutOffValveCheck, row, 2, 1, 2);

    txTabs->addTab(bcm19Tab, "BCM HVAC Specs (0x1F0)");

    mainLayout->addWidget(txTabs);
    return simGroup;
}

// Rx High-Speed Panel
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

// Rx Low-Speed Panel
QWidget* MainWindow::createLowSpeedRxPanel()
{
    QGroupBox *rxGroup = new QGroupBox("HVAC Unit Live Telemetry (Rx)", this);
    QVBoxLayout *rxLayout = new QVBoxLayout(rxGroup);
    rxLayout->setContentsMargins(4, 4, 4, 4);

    m_lsRxTable = new QTableWidget(16, 2, this);
    m_lsRxTable->setHorizontalHeaderLabels({"BD-CAN Signal Name", "Decoded Value"});
    m_lsRxTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_lsRxTable->verticalHeader()->setVisible(false);
    m_lsRxTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_lsRxTable->setSelectionMode(QAbstractItemView::NoSelection);

    QStringList hvacSignals = {
        "Cabin Temp (°C) [0x146]",
        "Target Temp Set (°C) [0x146]",
        "Blower Speed Level (0-8) [0x146]",
        "Evaporator Target Temp (°C) [0x086]",
        "Evaporator Actual Temp (°C) [0x086]",
        "AC Request (0x086)",
        "Rear Defrost Request (0x086)",
        "AC Indicator Status (0x146)",
        "Auto Mode (0x146)",
        "Power State (0x146)",
        "Intake Pattern (Recirc) [0x146]",
        "Face Mode (0x146)",
        "Foot Mode (0x146)",
        "Screen Mode (0x146)",
        "Front Screen Defrost (0x146)",
        "Active DTC Fault Code [0x526]"
    };

    for (int i = 0; i < hvacSignals.size(); ++i) {
        m_lsRxTable->setItem(i, 0, new QTableWidgetItem(hvacSignals[i]));
        QTableWidgetItem *valItem = new QTableWidgetItem("N/A");
        valItem->setTextAlignment(Qt::AlignCenter);

        //  DTC
        if (i == 15) {
            valItem->setForeground(Qt::red);
        }

        m_lsRxTable->setItem(i, 1, valItem);
    }

    rxLayout->addWidget(m_lsRxTable);
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

bool MainWindow::openCanDevice()
{
    if (!fnOpenDevice) return false;

    if (fnOpenDevice(m_devType, m_devIndex, 0) != 1) return false;

    VCI_INIT_CONFIG config{};
    config.AccCode = 0x00000000;
    config.AccMask = 0xFFFFFFFF;
    config.Filter  = 0;
    config.Mode    = 0;

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

    if (m_db.busType() == CanBusType::HighSpeed) {
        std::array<uint8_t, 8> buffer{};

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
        sendLowSpeedCanFrames();
    }
}

void MainWindow::sendLowSpeedCanFrames()
{


        // MMS_INFO3 (0x170)
        {
            std::array<uint8_t, 8> data{};

            if (m_mmsAutoCheck && m_mmsAutoCheck->isChecked())         data[0] |= (1 << 0);
            if (m_mmsIntakeCheck && m_mmsIntakeCheck->isChecked())     data[0] |= (1 << 1);
            if (m_mmsFaceCheck && m_mmsFaceCheck->isChecked())         data[0] |= (1 << 2);
            if (m_mmsFootCheck && m_mmsFootCheck->isChecked())         data[0] |= (1 << 3);
            if (m_mmsScreenCheck && m_mmsScreenCheck->isChecked())       data[0] |= (1 << 4);
            if (m_mmsDefrostCheck && m_mmsDefrostCheck->isChecked())     data[0] |= (1 << 5);
            if (m_mmsRearHeaterCheck && m_mmsRearHeaterCheck->isChecked()) data[0] |= (1 << 6);
            if (m_mmsPowerCheck && m_mmsPowerCheck->isChecked())       data[0] |= (1 << 7);

            if (m_mmsAcCheck && m_mmsAcCheck->isChecked())             data[1] |= (1 << 0);
            if (m_mmsTempIncCheck && m_mmsTempIncCheck->isChecked())   data[1] |= (1 << 1);
            if (m_mmsTempDecCheck && m_mmsTempDecCheck->isChecked())   data[1] |= (1 << 2);
            if (m_mmsBlowerIncCheck && m_mmsBlowerIncCheck->isChecked()) data[1] |= (1 << 3);
            if (m_mmsBlowerDecCheck && m_mmsBlowerDecCheck->isChecked()) data[1] |= (1 << 4);
            if (m_mmsFootFaceCheck && m_mmsFootFaceCheck->isChecked())  data[1] |= (1 << 5);
            if (m_mmsFootScreenCheck && m_mmsFootScreenCheck->isChecked()) data[1] |= (1 << 6);

            sendCanFrame(0x170, data, 8);
        }

        // BCM_INFO2 ( 0x088)
        {
            std::array<uint8_t, 8> data{};

            uint8_t vehType = m_lsVehicleTypeCombo ? m_lsVehicleTypeCombo->currentIndex() : 0;
            uint8_t engState = m_lsEngineStateCombo ? m_lsEngineStateCombo->currentIndex() : 0;
            uint8_t startSw = m_lsStartSwitchCombo ? m_lsStartSwitchCombo->currentIndex() : 0;

            data[0] |= (vehType & 0x01);
            data[1] |= (engState & 0x03);
            data[1] |= ((startSw & 0x03) << 2);
            data[1] |= (0x0A << 4); // Speed Validity = Valid

            double speed = m_lsVehicleSpeedSpin ? m_lsVehicleSpeedSpin->value() : 0.0;
            uint16_t speedRaw = static_cast<uint16_t>(speed / 0.125);
            data[4] = speedRaw & 0xFF;
            data[5] = (speedRaw >> 8) & 0x0F;

            sendCanFrame(0x088, data, 8);
        }

        // BCM_INFO4 ( 0x0C8)
        {
            std::array<uint8_t, 8> data{};

            if (m_lsAcStateCheck && m_lsAcStateCheck->isChecked())         data[0] |= (1 << 6);
            if (m_lsRearHeaterCheck && m_lsRearHeaterCheck->isChecked()) data[0] |= (1 << 7);

            double waterTemp = m_lsWaterTempSpin ? m_lsWaterTempSpin->value() : 85.0;
            uint8_t waterTempRaw = static_cast<uint8_t>((waterTemp - (-48.0)) / 0.75);
            data[1] = waterTempRaw;

            sendCanFrame(0x0C8, data, 8);
        }

        //BCM_INFO19 (0x1F0)
        {
            std::array<uint8_t, 8> data{};

            uint16_t compRpm = m_lsCompressorRpmSpin ? m_lsCompressorRpmSpin->value() : 0;
            data[0] = static_cast<uint8_t>(compRpm / 50);

            data[2] = static_cast<uint8_t>(m_lsOutGasPressureSpin ? m_lsOutGasPressureSpin->value() : 0);
            data[3] = static_cast<uint8_t>(m_lsInGasPressureSpin ? m_lsInGasPressureSpin->value() : 0);
            data[4] = static_cast<uint8_t>((m_lsOutGasTempSpin ? m_lsOutGasTempSpin->value() : 0) + 40);
            data[5] = static_cast<uint8_t>((m_lsInGasTempSpin ? m_lsInGasTempSpin->value() : 0) + 40);

            if (m_lsShutOffValveCheck && m_lsShutOffValveCheck->isChecked()) {
                data[6] |= (1 << 0);
            }

            sendCanFrame(0x1F0, data, 8);
        }

}

void MainWindow::sendCanFrame(uint32_t id, const std::array<uint8_t, 8>& data, uint8_t dlc)
{
    if (!fnTransmit) return;

    VCI_CAN_OBJ sendFrame{};
    sendFrame.ID = id;
    sendFrame.SendType = 0;
    sendFrame.DataLen = dlc;

    if (m_db.busType() == CanBusType::LowSpeed) {
        sendFrame.ExternFlag = 0; // 1 Extended 29-bit ID (WE HAVE 11-bit ID )
        sendFrame.RemoteFlag = 0;
    } else {
        sendFrame.ExternFlag = 0; // 0 Standard 11-bit ID
        sendFrame.RemoteFlag = 0;
    }

    for (int i = 0; i < dlc; ++i) {
        sendFrame.Data[i] = data[i];
    }

    if (fnTransmit(m_devType, m_devIndex, m_canIndex, &sendFrame, 1) == 1) {
        addLogEntry("TX", id, data, dlc);
    }
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
            addLogEntry("RX", rxFrames[i].ID, data, rxFrames[i].DataLen);
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
        if (id == 0x086) { // HVAC_INFO (Evaporator & Requests)[cite: 2]
            double evapTarget = (data[0] * 0.5) - 20.0;
                double evapActual = (data[1] * 0.5) - 20.0;
                bool acReq = (data[5] & 0x01);
                bool rearDefrostReq = (data[5] & 0x02);

                m_lsRxTable->item(3, 1)->setText(data[0] == 0xFE ? "Not Available" : QString::number(evapTarget, 'f', 1) + " °C");
                m_lsRxTable->item(4, 1)->setText(data[1] == 0xFE ? "Not Available" : QString::number(evapActual, 'f', 1) + " °C");
                m_lsRxTable->item(5, 1)->setText(acReq ? "ON" : "OFF");
                m_lsRxTable->item(6, 1)->setText(rearDefrostReq ? "ON" : "OFF");
        }
        else if (id == 0x146) { // HVAC_INFO2 (Modes, Temp, Blower)
            bool autoMode       = (data[0] & 0x01);
            bool intakePattern  = (data[0] & 0x02);
            bool faceMode       = (data[0] & 0x04);
            bool footMode       = (data[0] & 0x08);
            bool screenMode     = (data[0] & 0x10);
            bool defrostState   = (data[0] & 0x20);
            bool powerState     = (data[0] & 0x40);
            bool acIndicator    = (data[0] & 0x80);

            uint8_t tempRaw     = data[1] & 0x3F;
            double tempSet      = (tempRaw * 0.5) + 16.0;
                bool heaterReq      = (data[1] & 0x40);
            uint8_t blowerSpeed = data[2] & 0x0F;
                double cabinTemp    = (data[3] * 0.5) - 40.0;

            m_lsRxTable->item(0, 1)->setText(data[3] == 0xFF ? "Invalid" : QString::number(cabinTemp, 'f', 1) + " °C");
            m_lsRxTable->item(1, 1)->setText(tempRaw == 0x3F ? "Invalid" : QString::number(tempSet, 'f', 1) + " °C");
            m_lsRxTable->item(2, 1)->setText("Level " + QString::number(blowerSpeed));
            m_lsRxTable->item(7, 1)->setText(acIndicator ? "ON" : "OFF");
            m_lsRxTable->item(8, 1)->setText(autoMode ? "ON" : "OFF");
            m_lsRxTable->item(9, 1)->setText(powerState ? "ON" : "OFF");
            m_lsRxTable->item(10, 1)->setText(intakePattern ? "Outer (Recirc OFF)" : "Inner (Recirc ON)");
            m_lsRxTable->item(11, 1)->setText(faceMode ? "ON" : "OFF");
            m_lsRxTable->item(12, 1)->setText(footMode ? "ON" : "OFF");
            m_lsRxTable->item(13, 1)->setText(screenMode ? "ON" : "OFF");
            m_lsRxTable->item(14, 1)->setText(defrostState ? "ON" : "OFF");
        }
        else if (id == 0x526) { // HVAC_FLT (Diagnostic Fault Code)
            uint32_t dtc = (data[1] << 16) | (data[2] << 8) | data[3];
                m_lsRxTable->item(15, 1)->setText(dtc == 0 ? "No Fault" : QString("DTC: 0x%1").arg(dtc, 6, 16, QChar('0')).toUpper());
        }
    }
}