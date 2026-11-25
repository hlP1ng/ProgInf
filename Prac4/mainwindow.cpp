#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "telemetrymodel.h"
#include "mavlinkmanager.h"
#include "uavsimulator.h"

#include <QMessageBox>
#include <QTime>
#include <QHeaderView>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_telemetryModel = new TelemetryModel(this);
    m_mavlinkManager = new MavlinkManager(m_telemetryModel, this);
    m_uavSimulator   = new UavSimulator(this);

    setupChart();
    setupTable();

    setCommandButtonsEnabled(false);
    ui->labelLinkStatus->setStyleSheet("color: red; font-weight: bold;");

    connect(m_mavlinkManager, &MavlinkManager::telemetryReceived,
            m_telemetryModel, &TelemetryModel::updateTelemetry);
    connect(m_telemetryModel, &TelemetryModel::telemetryChanged,
            this, &MainWindow::onTelemetryChanged);
    connect(m_telemetryModel, &TelemetryModel::flightModeChanged,
            this, &MainWindow::onFlightModeChanged);

    connect(m_mavlinkManager, &MavlinkManager::linkStateChanged,
            m_telemetryModel, &TelemetryModel::updateLink);
    connect(m_telemetryModel, &TelemetryModel::linkStateChanged,
            this, &MainWindow::onLinkStateChanged);
    connect(m_mavlinkManager, &MavlinkManager::linkRestored,
            this, &MainWindow::onLinkRestored);

    connect(m_mavlinkManager, &MavlinkManager::packetReceived,
            this, &MainWindow::onPacketReceived);
    connect(m_mavlinkManager, &MavlinkManager::commandResult,
            this, &MainWindow::onCommandResult);
    connect(m_mavlinkManager, &MavlinkManager::errorOccurred,
            this, &MainWindow::onMavlinkError);
    connect(m_mavlinkManager, &MavlinkManager::networkEventLogged,
            this, &MainWindow::appendNetworkEvent);

    connect(ui->btnStartLink, &QPushButton::clicked,
            this, &MainWindow::onBtnStartLinkClicked);
    connect(ui->btnStopLink, &QPushButton::clicked,
            this, &MainWindow::onBtnStopLinkClicked);
    connect(ui->btnTakeoff, &QPushButton::clicked,
            this, &MainWindow::onBtnTakeoffClicked);
    connect(ui->btnLand, &QPushButton::clicked,
            this, &MainWindow::onBtnLandClicked);
    connect(ui->btnStopCommand, &QPushButton::clicked,
            this, &MainWindow::onBtnStopCommandClicked);

    connect(ui->btnStartUav, &QPushButton::clicked,
            this, &MainWindow::onBtnStartUavClicked);
    connect(ui->btnDisconnectUav, &QPushButton::clicked,
            this, &MainWindow::onBtnDisconnectUavClicked);
    connect(ui->btnCorruptPacket, &QPushButton::clicked,
            this, &MainWindow::onBtnCorruptPacketClicked);
    connect(ui->btnSkipPackets, &QPushButton::clicked,
            this, &MainWindow::onBtnSkipPacketsClicked);
    connect(ui->chkSuppressAck, &QCheckBox::toggled,
            this, &MainWindow::onChkSuppressAckToggled);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupChart()
{
    m_altitudeSeries = new QLineSeries(this);
    m_altitudeSeries->setName("Высота, м");

    m_chart = new QChart();
    m_chart->addSeries(m_altitudeSeries);
    m_chart->setTitle("Высота БПЛА");
    m_chart->legend()->setVisible(true);

    m_axisX = new QValueAxis(this);
    m_axisX->setTitleText("Время, с");
    m_axisX->setRange(0, 30);

    m_axisY = new QValueAxis(this);
    m_axisY->setTitleText("Высота, м");
    m_axisY->setRange(0, 150);

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_altitudeSeries->attachAxis(m_axisX);
    m_altitudeSeries->attachAxis(m_axisY);

    ui->chartView->setChart(m_chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::setupTable()
{
    ui->tableTelemetry->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableTelemetry->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableTelemetry->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void MainWindow::setCommandButtonsEnabled(bool enabled)
{
    ui->btnTakeoff->setEnabled(enabled);
    ui->btnLand->setEnabled(enabled);
    ui->btnStopCommand->setEnabled(enabled);
}

void MainWindow::onTelemetryChanged(quint8 sysId,
                                    quint8 compId,
                                    double latitude,
                                    double longitude,
                                    double altitude,
                                    const QString &flightMode)
{
    ui->labelLatitude->setText(QString::number(latitude, 'f', 7));
    ui->labelLongitude->setText(QString::number(longitude, 'f', 7));
    ui->labelAltitude->setText(QString::number(altitude, 'f', 1) + " м");
    ui->labelFlightMode->setText(flightMode);

    m_altitudeSeries->append(m_timeSeconds, altitude);
    m_timeSeconds += 0.1;
    if (m_altitudeSeries->count() > 300) {
        m_altitudeSeries->remove(0);
    }
    if (m_timeSeconds > m_axisX->max()) {
        m_axisX->setRange(m_timeSeconds - 30.0, m_timeSeconds);
    }
    if (altitude + 20.0 > m_axisY->max()) {
        m_axisY->setMax(altitude + 30.0);
    }

    const QDateTime now = QDateTime::currentDateTime();
    if (!m_lastTableAddTime.isValid() || m_lastTableAddTime.msecsTo(now) >= 1000) {
        m_lastTableAddTime = now;

        if (ui->tableTelemetry->rowCount() >= 200) {
            ui->tableTelemetry->removeRow(0);
        }

        const int row = ui->tableTelemetry->rowCount();
        ui->tableTelemetry->insertRow(row);
        ui->tableTelemetry->setItem(row, 0, new QTableWidgetItem(now.time().toString("hh:mm:ss")));
        ui->tableTelemetry->setItem(row, 1, new QTableWidgetItem(QString::number(sysId)));
        ui->tableTelemetry->setItem(row, 2, new QTableWidgetItem(QString::number(compId)));
        ui->tableTelemetry->setItem(row, 3, new QTableWidgetItem(QString::number(latitude, 'f', 7)));
        ui->tableTelemetry->setItem(row, 4, new QTableWidgetItem(QString::number(longitude, 'f', 7)));
        ui->tableTelemetry->setItem(row, 5, new QTableWidgetItem(QString::number(altitude, 'f', 1)));
        ui->tableTelemetry->setItem(row, 6, new QTableWidgetItem(flightMode));
        ui->tableTelemetry->scrollToBottom();
    }
}

void MainWindow::onFlightModeChanged(const QString &oldMode, const QString &newMode)
{
    appendNetworkEvent(QString("Смена режима полета: %1 -> %2").arg(oldMode, newMode));
}

void MainWindow::onLinkStateChanged(bool connected, const QString &address, quint16 port)
{
    if (connected) {
        ui->labelLinkStatus->setText("Связь установлена");
        ui->labelLinkStatus->setStyleSheet("color: green; font-weight: bold;");
        ui->labelSender->setText(address + ":" + QString::number(port));
        setCommandButtonsEnabled(true);
    } else {
        if (!address.isEmpty()) {
            ui->labelLinkStatus->setText("Связь потеряна (последний пакет: "
                                         + m_telemetryModel->lastUpdate().time().toString("hh:mm:ss") + ")");
        } else {
            ui->labelLinkStatus->setText("Связь отсутствует");
        }
        ui->labelLinkStatus->setStyleSheet("color: red; font-weight: bold;");
        ui->labelSender->setText("-");
        setCommandButtonsEnabled(false);
    }
}

void MainWindow::onLinkRestored()
{
    ui->labelLinkStatus->setText("Связь восстановлена");
    ui->labelLinkStatus->setStyleSheet("color: green; font-weight: bold;");
    setCommandButtonsEnabled(true);
}

void MainWindow::onPacketReceived(quint64 received, quint64 lost, quint64 errors)
{
    ui->labelPackets->setText(
        QString("Принято: %1 | Потеряно: %2 | Ошибок: %3")
            .arg(received)
            .arg(lost)
            .arg(errors));
}

void MainWindow::onCommandResult(int command, bool accepted, const QString &message)
{
    if (accepted) {
        ui->labelCommandStatus->setText("Статус: " + message);
        ui->labelCommandStatus->setStyleSheet("color: green; font-weight: bold;");
    } else {
        ui->labelCommandStatus->setText("Статус: " + message);
        ui->labelCommandStatus->setStyleSheet("color: red; font-weight: bold;");
    }

    const QString record = QString("[%1] COMMAND %2: %3")
                               .arg(QTime::currentTime().toString("hh:mm:ss"))
                               .arg(command)
                               .arg(message);
    ui->listCommandLog->insertItem(0, record);
}

void MainWindow::onMavlinkError(const QString &message)
{
    ui->labelLinkStatus->setText("Ошибка: " + message);
    ui->labelLinkStatus->setStyleSheet("color: red; font-weight: bold;");
}

void MainWindow::appendNetworkEvent(const QString &eventText)
{
    const QString entry = QString("[%1] %2")
    .arg(QTime::currentTime().toString("hh:mm:ss"))
        .arg(eventText);
    ui->listNetworkEvents->addItem(entry);
    ui->listNetworkEvents->scrollToBottom();
}

void MainWindow::onBtnStartLinkClicked()
{
    m_mavlinkManager->start(14540);
}

void MainWindow::onBtnStopLinkClicked()
{
    m_mavlinkManager->stop();
}

void MainWindow::onBtnTakeoffClicked()
{
    if (!m_telemetryModel->isLinkConnected()) {
        QMessageBox::warning(
            this,
            "Команда недоступна",
            "Невозможно отправить команду взлета: нет связи с БПЛА.");
        return;
    }

    const QMessageBox::StandardButton answer =
        QMessageBox::question(
            this,
            "Подтверждение команды",
            "Отправить команду взлета на высоту "
                + QString::number(ui->spinTakeoffAltitude->value(), 'f', 0)
                + " м?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

    if (answer != QMessageBox::Yes) {
        return;
    }

    m_mavlinkManager->sendTakeoff(ui->spinTakeoffAltitude->value());
}

void MainWindow::onBtnLandClicked()
{
    if (!m_telemetryModel->isLinkConnected()) {
        QMessageBox::warning(
            this,
            "Команда недоступна",
            "Невозможно отправить команду посадки: нет связи с БПЛА.");
        return;
    }
    m_mavlinkManager->sendLand();
}

void MainWindow::onBtnStopCommandClicked()
{
    if (!m_telemetryModel->isLinkConnected()) {
        QMessageBox::warning(
            this,
            "Команда недоступна",
            "Невозможно отправить команду экстренной остановки: нет связи с БПЛА.");
        return;
    }
    m_mavlinkManager->sendEmergencyStop();
}

void MainWindow::onBtnStartUavClicked()
{
    m_uavSimulator->start("127.0.0.1", 14540);
    appendNetworkEvent("Эмулятор БПЛА запущен (отправка на 127.0.0.1:14540)");
}

void MainWindow::onBtnDisconnectUavClicked()
{
    m_uavSimulator->stop();
    appendNetworkEvent("Эмулятор БПЛА остановлен");
}

void MainWindow::onBtnCorruptPacketClicked()
{
    m_uavSimulator->corruptNextPacket();
}

void MainWindow::onBtnSkipPacketsClicked()
{
    m_uavSimulator->skipTwoPackets();
}

void MainWindow::onChkSuppressAckToggled(bool checked)
{
    m_uavSimulator->setSuppressAck(checked);
}