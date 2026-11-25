#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QMessageBox>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QPen>
#include <QBrush>
#include <QPolygonF>
#include <QPointF>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    vehicle1(nullptr),
    gyro1(nullptr),
    accel1(nullptr),
    baro1(nullptr),
    nav1(nullptr),
    ctrl1(nullptr),
    autopilot1(nullptr),
    vehicle2(nullptr),
    gyro2(nullptr),
    accel2(nullptr),
    baro2(nullptr),
    nav2(nullptr),
    ctrl2(nullptr),
    autopilot2(nullptr),
    cycle_timer(nullptr),
    altitude_chart(nullptr),
    series_uav1(nullptr),
    series_uav2(nullptr),
    time_uav1(0.0),
    time_uav2(0.0),
    scene(nullptr),
    marker1_heading(nullptr),
    marker2_heading(nullptr),
    marker1_icon(nullptr),
    marker2_icon(nullptr),
    trajectory1(nullptr),
    trajectory2(nullptr),
    trajectory1_started(false),
    trajectory2_started(false),
    first_fault_reported(false) {
    ui->setupUi(this);


    vehicle1 = new Quadcopter(0.0, 92.0, -130.0, -110.0);
    gyro1 = new Gyroscope(1.2);
    accel1 = new Accelerometer(9.81);
    baro1 = new Barometer(1012.5);
    nav1 = new GPSNavigation();
    ctrl1 = new PIDController(0.65, 0.08, 0.12);
    autopilot1 = new Autopilot("Quadcopter",
                               vehicle1,
                               {gyro1, accel1, baro1},
                               nav1,
                               ctrl1,
                               150.0,
                               this);

  
    vehicle2 = new FixedWingUAV(25.0, 86.0, -40.0, -110.0);
    gyro2 = new Gyroscope(0.9);
    accel2 = new Accelerometer(9.81);
    baro2 = new Barometer(1012.5);
    nav2 = new GPSNavigation();
    ctrl2 = new PIDController(0.50, 0.06, 0.10);
    autopilot2 = new Autopilot("FixedWingUAV",
                               vehicle2,
                               {gyro2, accel2, baro2},
                               nav2,
                               ctrl2,
                               150.0,
                               this);

    fleet = {vehicle1, vehicle2};

  
    setup_fleet_table();
    setup_altitude_chart();
    setup_trajectory_scene();

    ui->labelStatus->setText("Stopped");
    ui->labelStatus->setStyleSheet("color: #333333; font-weight: bold;");
    ui->labelMode1->setText(autopilot1->mode_string());
    ui->labelMode2->setText(autopilot2->mode_string());


    cycle_timer = new QTimer(this);
    cycle_timer->setInterval(100);

    connect(cycle_timer, &QTimer::timeout, autopilot1, &Autopilot::run_cycle);
    connect(cycle_timer, &QTimer::timeout, autopilot2, &Autopilot::run_cycle);

    
    connect(autopilot1, &Autopilot::telemetry_updated, this, &MainWindow::on_telemetry_labels_updated);
    connect(autopilot1, &Autopilot::telemetry_updated, this, &MainWindow::on_telemetry_chart_updated);
    connect(autopilot1, &Autopilot::telemetry_updated, this, &MainWindow::on_telemetry_table_updated);

    connect(autopilot2, &Autopilot::telemetry_updated, this, &MainWindow::on_telemetry_labels_updated);
    connect(autopilot2, &Autopilot::telemetry_updated, this, &MainWindow::on_telemetry_chart_updated);
    connect(autopilot2, &Autopilot::telemetry_updated, this, &MainWindow::on_telemetry_table_updated);


    connect(autopilot1, &Autopilot::position_updated, this, &MainWindow::on_position_scene_updated);
    connect(autopilot2, &Autopilot::position_updated, this, &MainWindow::on_position_scene_updated);

    connect(autopilot1, &Autopilot::mode_changed, this, &MainWindow::on_mode_changed);
    connect(autopilot2, &Autopilot::mode_changed, this, &MainWindow::on_mode_changed);

    connect(autopilot1, &Autopilot::fault_detected, this, &MainWindow::on_fault_detected);
    connect(autopilot2, &Autopilot::fault_detected, this, &MainWindow::on_fault_detected);

    
    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::handle_start_clicked);
    connect(ui->btnTakeoff, &QPushButton::clicked, this, &MainWindow::handle_takeoff_clicked);
    connect(ui->btnLand, &QPushButton::clicked, this, &MainWindow::handle_land_clicked);
    connect(ui->btnStop, &QPushButton::clicked, this, &MainWindow::handle_stop_clicked);
    connect(ui->btnFaultSensor, &QPushButton::clicked, this, &MainWindow::handle_fault_sensor_clicked);
    connect(ui->btnReset, &QPushButton::clicked, this, &MainWindow::handle_reset_clicked);
}

MainWindow::~MainWindow() {
    delete ctrl1;
    delete nav1;
    delete baro1;
    delete accel1;
    delete gyro1;
    delete vehicle1;

    delete ctrl2;
    delete nav2;
    delete baro2;
    delete accel2;
    delete gyro2;
    delete vehicle2;

    delete ui;
}

void MainWindow::setup_fleet_table() {
    ui->tableFleet->setRowCount(0);
    ui->tableFleet->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (UAV* uav : fleet) {
        const int row = ui->tableFleet->rowCount();
        ui->tableFleet->insertRow(row);
        ui->tableFleet->setItem(row, 0, new QTableWidgetItem(uav->type()));
        ui->tableFleet->setItem(row, 1, new QTableWidgetItem(QString::number(uav->battery(), 'f', 0) + " %"));
        ui->tableFleet->setItem(row, 2, new QTableWidgetItem("Connected"));
    }
}

void MainWindow::setup_altitude_chart() {
    series_uav1 = new QLineSeries(this);
    series_uav1->setName("Quadcopter");

    series_uav2 = new QLineSeries(this);
    series_uav2->setName("FixedWingUAV");

    altitude_chart = new QChart();
    altitude_chart->addSeries(series_uav1);
    altitude_chart->addSeries(series_uav2);
    altitude_chart->createDefaultAxes();
    altitude_chart->setTitle("Altitude, m");
    altitude_chart->legend()->setVisible(true);

    const auto axes_x = altitude_chart->axes(Qt::Horizontal);
    if (!axes_x.isEmpty()) {
        axes_x.first()->setRange(0.0, 30.0);
        axes_x.first()->setTitleText("Time, s");
    }

    const auto axes_y = altitude_chart->axes(Qt::Vertical);
    if (!axes_y.isEmpty()) {
        axes_y.first()->setRange(0.0, 230.0);
    }

    ui->chartView->setChart(altitude_chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);
}

QPainterPath MainWindow::create_quadcopter_icon_path() {
  
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);

    QPolygonF body;
    body << QPointF(11.0, 0.0)
         << QPointF(4.0, -4.0)
         << QPointF(-6.0, -4.0)
         << QPointF(-6.0, 4.0)
         << QPointF(4.0, 4.0);
    path.addPolygon(body);
    path.closeSubpath();

    QPolygonF arm1;
    arm1 << QPointF(2.0, -2.0) << QPointF(9.0, -9.0) << QPointF(7.5, -10.5) << QPointF(0.5, -3.5);
    path.addPolygon(arm1);
    path.closeSubpath();

    QPolygonF arm2;
    arm2 << QPointF(2.0, 2.0) << QPointF(9.0, 9.0) << QPointF(7.5, 10.5) << QPointF(0.5, 3.5);
    path.addPolygon(arm2);
    path.closeSubpath();

    QPolygonF arm3;
    arm3 << QPointF(-3.0, -2.0) << QPointF(-9.0, -9.0) << QPointF(-7.5, -10.5) << QPointF(-1.5, -3.5);
    path.addPolygon(arm3);
    path.closeSubpath();

    QPolygonF arm4;
    arm4 << QPointF(-3.0, 2.0) << QPointF(-9.0, 9.0) << QPointF(-7.5, 10.5) << QPointF(-1.5, 3.5);
    path.addPolygon(arm4);
    path.closeSubpath();

    path.addEllipse(QPointF(8.5, -9.5), 4.2, 4.2);
    path.addEllipse(QPointF(8.5,  9.5), 4.2, 4.2);
    path.addEllipse(QPointF(-8.5, -9.5), 4.2, 4.2);
    path.addEllipse(QPointF(-8.5,  9.5), 4.2, 4.2);

    return path;
}

QPainterPath MainWindow::create_fixed_wing_icon_path() {

    QPainterPath path;
    QPolygonF plane;
    plane << QPointF(16.0, 0.0)
          << QPointF(7.0, -2.5)
          << QPointF(2.0, -16.0)
          << QPointF(-3.0, -16.0)
          << QPointF(-2.0, -2.5)
          << QPointF(-10.0, -2.0)
          << QPointF(-14.0, -7.0)
          << QPointF(-16.0, -7.0)
          << QPointF(-14.5, 0.0)
          << QPointF(-16.0, 7.0)
          << QPointF(-14.0, 7.0)
          << QPointF(-10.0, 2.0)
          << QPointF(-2.0, 2.5)
          << QPointF(-3.0, 16.0)
          << QPointF(2.0, 16.0)
          << QPointF(7.0, 2.5);
    path.addPolygon(plane);
    path.closeSubpath();
    return path;
}

void MainWindow::setup_trajectory_scene() {
    scene = new QGraphicsScene(this);
    scene->setSceneRect(-200, -200, 400, 400);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);

    QPolygonF mission_route;
    mission_route << QPointF(-130.0, -110.0)
                  << QPointF( 130.0, -110.0)
                  << QPointF( 130.0,  110.0)
                  << QPointF(-130.0,  110.0);
    scene->addPolygon(mission_route, QPen(Qt::lightGray, 1, Qt::DashLine));

    QPainterPath path1;
    path1.moveTo(vehicle1->x(), vehicle1->y());
    path1.lineTo(vehicle1->x(), vehicle1->y());
    trajectory1 = scene->addPath(path1, QPen(Qt::blue, 2));

    QPainterPath path2;
    path2.moveTo(vehicle2->x(), vehicle2->y());
    path2.lineTo(vehicle2->x(), vehicle2->y());
    trajectory2 = scene->addPath(path2, QPen(Qt::darkGreen, 2));

    marker1_heading = scene->createItemGroup({});
    marker1_icon = new QGraphicsPathItem(create_quadcopter_icon_path());
    marker1_icon->setPen(QPen(Qt::darkBlue, 1.2));
    marker1_icon->setBrush(QBrush(QColor(30, 110, 240, 210)));
    marker1_heading->addToGroup(marker1_icon);
    marker1_heading->setPos(vehicle1->x(), vehicle1->y());
    marker1_heading->setZValue(10.0);

    marker2_heading = scene->createItemGroup({});
    marker2_icon = new QGraphicsPathItem(create_fixed_wing_icon_path());
    marker2_icon->setPen(QPen(Qt::darkGreen, 1.2));
    marker2_icon->setBrush(QBrush(QColor(30, 175, 65, 220)));
    marker2_heading->addToGroup(marker2_icon);
    marker2_heading->setPos(vehicle2->x(), vehicle2->y());
    marker2_heading->setZValue(10.0);
}

void MainWindow::handle_start_clicked() {
    if (autopilot1->mode_string() == "Stopped") {
        autopilot1->command_takeoff();
    }
    if (autopilot2->mode_string() == "Stopped") {
        autopilot2->command_takeoff();
    }

    if (!cycle_timer->isActive()) {
        cycle_timer->start();
    }
    ui->labelStatus->setText("Mission status: Active");
    ui->labelStatus->setStyleSheet("color: green; font-weight: bold;");
}

void MainWindow::handle_takeoff_clicked() {
    autopilot1->command_takeoff();
    autopilot2->command_takeoff();
    if (!cycle_timer->isActive() && !first_fault_reported) {
        cycle_timer->start();
    }
}

void MainWindow::handle_land_clicked() {
    autopilot1->command_land();
    autopilot2->command_land();
    if (!cycle_timer->isActive() && !first_fault_reported) {
        cycle_timer->start();
    }
}

void MainWindow::handle_stop_clicked() {
    autopilot1->command_stop();
    autopilot2->command_stop();
    cycle_timer->stop();

    for (int row = 0; row < ui->tableFleet->rowCount(); ++row) {
        if (ui->tableFleet->item(row, 2) != nullptr && !first_fault_reported) {
            ui->tableFleet->item(row, 2)->setText("Stopped");
        }
    }
}

void MainWindow::handle_fault_sensor_clicked() {
    gyro1->set_ready(false);
}

void MainWindow::handle_reset_clicked() {
    gyro1->set_ready(true);
    gyro2->set_ready(true);
    nav1->set_valid(true);
    nav2->set_valid(true);
    first_fault_reported = false;

    ui->labelStatus->setText("Mission status: Active");
    ui->labelStatus->setStyleSheet("color: green; font-weight: bold;");

    for (int row = 0; row < ui->tableFleet->rowCount(); ++row) {
        if (ui->tableFleet->item(row, 2) != nullptr) {
            ui->tableFleet->item(row, 2)->setText("Connected");
        }
    }

    if (!cycle_timer->isActive()) {
        cycle_timer->start();
    }
}

void MainWindow::on_telemetry_labels_updated(const QString& name,
                                             double altitude,
                                             double error,
                                             double control) {
    if (name == "Quadcopter") {
        ui->labelAltitude1->setText(QString::number(altitude, 'f', 1) + " m");
        ui->labelError1->setText(QString::number(error, 'f', 1) + " m");
        ui->labelOutput1->setText(QString::number(control, 'f', 2));
    } else if (name == "FixedWingUAV") {
        ui->labelAltitude2->setText(QString::number(altitude, 'f', 1) + " m");
        ui->labelError2->setText(QString::number(error, 'f', 1) + " m");
        ui->labelOutput2->setText(QString::number(control, 'f', 2));
    }
}

void MainWindow::on_telemetry_chart_updated(const QString& name,
                                            double altitude,
                                            double,
                                            double) {
    if (!cycle_timer->isActive()) {
        return;
    }

    double current_t = 0.0;
    if (name == "Quadcopter") {
        series_uav1->append(time_uav1, altitude);
        time_uav1 += 0.1;
        current_t = time_uav1;
    } else if (name == "FixedWingUAV") {
        series_uav2->append(time_uav2, altitude);
        time_uav2 += 0.1;
        current_t = time_uav2;
    }

    const auto axes_x = altitude_chart->axes(Qt::Horizontal);
    if (!axes_x.isEmpty() && current_t > 30.0) {
        axes_x.first()->setRange(current_t - 30.0, current_t);
    }
}

void MainWindow::on_telemetry_table_updated(const QString& name,
                                            double altitude,
                                            double,
                                            double) {
    if (!cycle_timer->isActive()) {
        return;
    }

    for (int row = 0; row < ui->tableFleet->rowCount(); ++row) {
        QTableWidgetItem* type_item = ui->tableFleet->item(row, 0);
        if (type_item != nullptr && type_item->text() == name && row < static_cast<int>(fleet.size())) {
            ui->tableFleet->item(row, 1)->setText(QString::number(fleet[row]->battery(), 'f', 1) + " %");
            ui->tableFleet->item(row, 2)->setText(altitude <= 0.05 ? "Landed" : "Active");
        }
    }
}

void MainWindow::on_position_scene_updated(const QString& name,
                                           double x,
                                           double y,
                                           double heading_deg,
                                           double roll) {
    if (name == "Quadcopter" && marker1_heading != nullptr && trajectory1 != nullptr) {
        marker1_heading->setPos(x, y);
        marker1_heading->setRotation(heading_deg);
        marker1_icon->setRotation(roll);

        QPainterPath path = trajectory1->path();
        if (!trajectory1_started) {
            path = QPainterPath();
            path.moveTo(x, y);
            path.lineTo(x, y);
            trajectory1_started = true;
        } else {
            path.lineTo(x, y);
        }
        trajectory1->setPath(path);
    } else if (name == "FixedWingUAV" && marker2_heading != nullptr && trajectory2 != nullptr) {
        marker2_heading->setPos(x, y);
        marker2_heading->setRotation(heading_deg);
        marker2_icon->setRotation(roll);

        QPainterPath path = trajectory2->path();
        if (!trajectory2_started) {
            path = QPainterPath();
            path.moveTo(x, y);
            path.lineTo(x, y);
            trajectory2_started = true;
        } else {
            path.lineTo(x, y);
        }
        trajectory2->setPath(path);
    }
}

void MainWindow::on_mode_changed(const QString& name, const QString& mode) {
    if (name == "Quadcopter") {
        ui->labelMode1->setText(mode);
    } else if (name == "FixedWingUAV") {
        ui->labelMode2->setText(mode);
    }

    ui->labelStatus->setText("Mode [" + name + "]: " + mode);
    ui->labelStatus->setStyleSheet("color: #0055aa; font-weight: bold;");
}

void MainWindow::on_fault_detected(const QString& name, const QString& reason) {
    cycle_timer->stop();

    const QString full_msg = name + " -> " + reason;


    ui->labelStatus->setText("FAULT: " + full_msg);
    ui->labelStatus->setStyleSheet("color: red; font-weight: bold;");


    for (int row = 0; row < ui->tableFleet->rowCount(); ++row) {
        QTableWidgetItem* type_item = ui->tableFleet->item(row, 0);
        if (type_item != nullptr) {
            if (type_item->text() == name) {
                ui->tableFleet->item(row, 2)->setText("FAULT");
            } else if (ui->tableFleet->item(row, 2)->text() != "FAULT") {
                ui->tableFleet->item(row, 2)->setText("Stopped");
            }
        }
    }

  
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    ui->listEventLog->addItem("[" + timestamp + "] FAULT: " + full_msg);
    ui->listEventLog->scrollToBottom();

 
    if (!first_fault_reported) {
        first_fault_reported = true;
        QMessageBox::warning(this, "Авария БПЛА (Failsafe)", "Обнаружен отказ:\n" + full_msg);
    }
}