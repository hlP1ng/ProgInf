#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QProgressBar>
#include <QGroupBox>
#include <QFrame>
#include <QDebug>
#include <iostream>
#include <vector>

#include "uav.h"
#include "sensor.h"
#include "navigation.h"
#include "controller.h"

// Глобальный указатель на графический терминал для дублирования вывода qDebug()
static QTextEdit* g_terminal = nullptr;

// Перехватчик сообщений qDebug(): выводит текст И в консоль Qt Creator, И в окно программы
void customMessageHandler(QtMsgType /*type*/, const QMessageLogContext& /*context*/, const QString& msg) {
    // 1. Стандартный вывод в консоль Qt Creator (Application Output)
    std::cout << msg.toStdString() << std::endl;

    // 2. Визуальный вывод в графический терминал с цветовой индикацией
    if (g_terminal) {
        QString color = "#e2e8f0"; // Базовый светлый цвет
        if (msg.startsWith("===") || msg.startsWith("---")) {
            color = "#38bdf8"; // Голубой для заголовков
        } else if (msg.contains("Error") || msg.contains("aborted") ||
                   msg.contains("invalid") || msg.contains("failure") ||
                   msg.contains("not ready")) {
            color = "#f87171"; // Красный для ошибок и отказов
        } else if (msg.contains("completed") || msg.contains("All sensors are ready") ||
                   msg.contains("near: false")) {
            color = "#4ade80"; // Зелёный для успешных операций
        } else if (msg.contains("near: true") || msg.contains("output:")) {
            color = "#facc15"; // Жёлтый для предупреждений и важных значений
        }

        g_terminal->append(QString("<span style='color:%1;'>%2</span>")
                               .arg(color, msg.toHtmlEscaped()));
    }
}

// --- Функции выполнения заданий ---

void runTask1() {
    qDebug().noquote() << "=== ЗАДАНИЕ 1 (Вариант 3): Виртуальные классы БПЛА ===";
    SurveyUAV survey;
    CargoUAV cargo;
    ReconnaissanceUAV recon;

    std::vector<UAV*> fleet = { &survey, &cargo, &recon };
    for (UAV* uav : fleet) {
        execute_test_mission(*uav);
    }
    qDebug().noquote() << "";
}

void runTask2() {
    qDebug().noquote() << "=== ЗАДАНИЕ 2 (Вариант 3): Сенсоры окружающей обстановки ===";
    Lidar lidar(12.5, true);
    Radar radar(18.2, true);
    Camera camera(3, true);

    std::vector<Sensor*> sensors = { &lidar, &radar, &camera };

    // Полиморфный обход вектора сенсоров
    for (Sensor* s : sensors) {
        s->print_reading();
    }

    qDebug().noquote() << "--- Проверка порога 10 м ---";
    obstacle_is_near(lidar, 10.0);
    obstacle_is_near(radar, 10.0);

    qDebug().noquote() << "--- Проверка порога 20 м ---";
    obstacle_is_near(lidar, 20.0);
    obstacle_is_near(radar, 20.0);

    qDebug().noquote() << "--- Проверка неготового сенсора ---";
    lidar.set_ready(false);
    obstacle_is_near(lidar, 20.0);
    lidar.set_ready(true);
    qDebug().noquote() << "";
}

void runTask3() {
    qDebug().noquote() << "=== ЗАДАНИЕ 3 (Вариант 3): Абстрактные контроллеры полёта ===";
    PIDController pid(0.5, 0.08, 0.024); // Выдаёт ровно 1.87 при error=2.5, dt=0.1
    SimpleController simple(0.5);
    FuzzyController fuzzy(0.3, 0.6, 0.9);

    std::vector<FlightController*> controllers = { &pid, &simple, &fuzzy };
    for (FlightController* ctrl : controllers) {
        test_controller(*ctrl, 2.5, 0.1);
        qDebug().noquote() << "---";
    }
    qDebug().noquote() << "";
}

// Запуск сценариев Интеграционного задания (1..5)
void runAutopilotScenario(int scenario,
                          QLabel* statusBadge,
                          QProgressBar* altBar,
                          QLabel* telemetryLabel) {
    SurveyUAV uav;
    Lidar lidar(12.5, true);
    Radar radar(18.2, true);
    Camera camera(3, true);
    std::vector<Sensor*> sensors = { &lidar, &radar, &camera };

    GPSNavigation gps({55.75, 37.61, 150.0}, 8, true);
    SimpleController controller(0.5); // 0.5 * 50 = 25 (эталонный вывод)

    switch (scenario) {
    case 1:
        qDebug().noquote() << "=== АВТОПИЛОТ — Сценарий 1: Штатная работа (высота 150 м) ===";
        statusBadge->setText("ШТАТНЫЙ РЕЖИМ");
        statusBadge->setStyleSheet("background:#065f46; color:#6ee7b7; padding:6px 12px; border-radius:6px; font-weight:bold;");
        altBar->setValue(150);
        telemetryLabel->setText("БПЛА: Survey UAV | Навигация: GPS (Valid) | Высота: 150 / 200 м | Сенсоры: OK");
        {
            Autopilot ap(&uav, sensors, &gps, &controller);
            ap.run_cycle();
        }
        break;

    case 2:
        qDebug().noquote() << "=== АВТОПИЛОТ — Сценарий 2: Отказ сенсора (Сенсор не готов) ===";
        radar.set_ready(false);
        statusBadge->setText("АВАРИЯ СЕНСОРА");
        statusBadge->setStyleSheet("background:#7f1d1d; color:#fca5a5; padding:6px 12px; border-radius:6px; font-weight:bold;");
        telemetryLabel->setText("БПЛА: Survey UAV | Навигация: GPS | Радар: ОТКАЗ (Not Ready)");
        {
            Autopilot ap(&uav, sensors, &gps, &controller);
            ap.run_cycle();
        }
        break;

    case 3:
        qDebug().noquote() << "=== АВТОПИЛОТ — Сценарий 3: Навигация невалидна ===";
        gps.set_valid(false);
        statusBadge->setText("ПОТЕРЯ GPS");
        statusBadge->setStyleSheet("background:#78350f; color:#fcd34d; padding:6px 12px; border-radius:6px; font-weight:bold;");
        telemetryLabel->setText("БПЛА: Survey UAV | Навигация: GPS (INVALID) | Сенсоры: OK");
        {
            Autopilot ap(&uav, sensors, &gps, &controller);
            ap.run_cycle();
        }
        break;

    case 4:
        qDebug().noquote() << "=== АВТОПИЛОТ — Сценарий 4: controller == nullptr ===";
        statusBadge->setText("ОШИБКА КОНТРОЛЛЕРА");
        statusBadge->setStyleSheet("background:#7f1d1d; color:#fca5a5; padding:6px 12px; border-radius:6px; font-weight:bold;");
        telemetryLabel->setText("БПЛА: Survey UAV | Контроллер: NULLPTR | Сенсоры: OK");
        {
            Autopilot ap(&uav, sensors, &gps, nullptr);
            ap.run_cycle();
        }
        break;

    case 5:
        qDebug().noquote() << "=== АВТОПИЛОТ — Сценарий 5: Отрицательная высота (-25 м) ===";
        gps.set_position({55.75, 37.61, -25.0});
        statusBadge->setText("ОШИБКА КООРДИНАТ");
        statusBadge->setStyleSheet("background:#7f1d1d; color:#fca5a5; padding:6px 12px; border-radius:6px; font-weight:bold;");
        altBar->setValue(0);
        telemetryLabel->setText("БПЛА: Survey UAV | Навигация: GPS (Высота -25 м — некорректно)");
        {
            Autopilot ap(&uav, sensors, &gps, &controller);
            ap.run_cycle();
        }
        break;
    }
    qDebug().noquote() << "";
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    qInstallMessageHandler(customMessageHandler);

    // Главное окно наземной станции управления
    QWidget window;
    window.setWindowTitle("КР №2 — Архитектура БПЛА (Вариант 3 + Autopilot) | Ground Control Station");
    window.resize(1020, 720);
    window.setStyleSheet(
        "QWidget { background-color: #0f172a; color: #f8fafc; font-family: 'Segoe UI', sans-serif; }"
        "QGroupBox { border: 1px solid #334155; border-radius: 8px; margin-top: 12px; padding-top: 10px; font-weight: bold; color: #38bdf8; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }"
        "QPushButton { background-color: #1e293b; border: 1px solid #475569; border-radius: 6px; padding: 8px 12px; color: #e2e8f0; font-weight: 600; text-align: left; }"
        "QPushButton:hover { background-color: #334155; border-color: #38bdf8; }"
        "QPushButton:pressed { background-color: #0284c7; }"
        "QProgressBar { border: 1px solid #334155; border-radius: 5px; text-align: center; background: #1e293b; color: white; font-weight: bold; }"
        "QProgressBar::chunk { background-color: #0ea5e9; border-radius: 4px; }"
        );

    QVBoxLayout* mainLayout = new QVBoxLayout(&window);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Верхняя панель статуса и телеметрии
    QFrame* headerFrame = new QFrame();
    headerFrame->setStyleSheet("background-color: #1e293b; border: 1px solid #334155; border-radius: 8px;");
    QHBoxLayout* headerLayout = new QHBoxLayout(headerFrame);

    QVBoxLayout* titleBox = new QVBoxLayout();
    QLabel* titleLabel = new QLabel("ПО ОБЪЕКТОВ НАЗЕМНОЙ ИНФРАСТРУКТУРЫ БПЛА");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #38bdf8; border: none;");
    QLabel* telemetryLabel = new QLabel("БПЛА: Survey UAV | Навигация: GPS (Valid) | Высота: 150 / 200 м | Сенсоры: Lidar, Radar, Camera");
    telemetryLabel->setStyleSheet("font-size: 12px; color: #94a3b8; border: none;");
    titleBox->addWidget(titleLabel);
    titleBox->addWidget(telemetryLabel);

    QLabel* statusBadge = new QLabel("ГОТОВ К РАБОТЕ");
    statusBadge->setAlignment(Qt::AlignCenter);
    statusBadge->setStyleSheet("background:#065f46; color:#6ee7b7; padding:6px 14px; border-radius:6px; font-weight:bold;");

    headerLayout->addLayout(titleBox, 1);
    headerLayout->addWidget(statusBadge);
    mainLayout->addWidget(headerFrame);

    // Шкала высоты БПЛА
    QHBoxLayout* altLayout = new QHBoxLayout();
    QLabel* altTitle = new QLabel("Текущая высота полёта (Target = 200 m):");
    altTitle->setStyleSheet("color: #cbd5e1; font-weight: 600;");
    QProgressBar* altBar = new QProgressBar();
    altBar->setRange(0, 200);
    altBar->setValue(150);
    altBar->setFormat("%v м из 200 м");
    altLayout->addWidget(altTitle);
    altLayout->addWidget(altBar, 1);
    mainLayout->addLayout(altLayout);

    // Центральная часть: слева кнопки управления, справа терминал вывода qDebug()
    QHBoxLayout* centerLayout = new QHBoxLayout();

    // Левая панель управления
    QVBoxLayout* controlsLayout = new QVBoxLayout();

    QGroupBox* tasksGroup = new QGroupBox("Задания 1–3 (Вариант 3)");
    QVBoxLayout* tasksBox = new QVBoxLayout(tasksGroup);
    QPushButton* btnTask1 = new QPushButton("1. Классы БПЛА (Survey, Cargo, Recon)");
    QPushButton* btnTask2 = new QPushButton("2. Сенсоры (Lidar, Radar, Camera)");
    QPushButton* btnTask3 = new QPushButton("3. Контроллеры (PID, Simple, Fuzzy)");
    tasksBox->addWidget(btnTask1);
    tasksBox->addWidget(btnTask2);
    tasksBox->addWidget(btnTask3);

    QGroupBox* apGroup = new QGroupBox("Интеграционное задание: Autopilot");
    QVBoxLayout* apBox = new QVBoxLayout(apGroup);
    QPushButton* btnScen1 = new QPushButton("Сценарий 1: Штатный цикл (150 м -> 200 м)");
    QPushButton* btnScen2 = new QPushButton("Сценарий 2: Сенсор не готов (Failure)");
    QPushButton* btnScen3 = new QPushButton("Сценарий 3: Навигация невалидна");
    QPushButton* btnScen4 = new QPushButton("Сценарий 4: controller == nullptr");
    QPushButton* btnScen5 = new QPushButton("Сценарий 5: Отрицательная высота");
    apBox->addWidget(btnScen1);
    apBox->addWidget(btnScen2);
    apBox->addWidget(btnScen3);
    apBox->addWidget(btnScen4);
    apBox->addWidget(btnScen5);

    QPushButton* btnRunAll = new QPushButton("▶ ЗАПУСТИТЬ ПОЛНЫЙ ОТЧЁТ (Все тесты)");
    btnRunAll->setStyleSheet("background-color: #0284c7; color: white; font-weight: bold; padding: 10px; text-align: center;");

    QPushButton* btnClear = new QPushButton("Очистить терминал");
    btnClear->setStyleSheet("background-color: #334155; color: #cbd5e1; text-align: center;");

    controlsLayout->addWidget(tasksGroup);
    controlsLayout->addWidget(apGroup);
    controlsLayout->addWidget(btnRunAll);
    controlsLayout->addWidget(btnClear);
    controlsLayout->addStretch();

    // Правая панель: Терминал телеметрии и логов qDebug()
    QGroupBox* termGroup = new QGroupBox("Терминал телеметрии (Вывод qDebug)");
    QVBoxLayout* termLayout = new QVBoxLayout(termGroup);
    QTextEdit* terminal = new QTextEdit();
    terminal->setReadOnly(true);
    terminal->setStyleSheet(
        "background-color: #020617; color: #e2e8f0; font-family: 'Consolas', 'Courier New', monospace; "
        "font-size: 13px; border: 1px solid #1e293b; border-radius: 6px; padding: 8px;"
        );
    termLayout->addWidget(terminal);
    g_terminal = terminal;

    centerLayout->addLayout(controlsLayout, 1);
    centerLayout->addWidget(termGroup, 2);
    mainLayout->addLayout(centerLayout, 1);

    // Функция полного прогона всех заданий
    auto runAllTests = [&]() {
        terminal->clear();
        runTask1();
        runTask2();
        runTask3();
        for (int s = 1; s <= 5; ++s) {
            runAutopilotScenario(s, statusBadge, altBar, telemetryLabel);
        }
        // Возвращаем индикаторы в штатное состояние после демонстрации
        runAutopilotScenario(1, statusBadge, altBar, telemetryLabel);
    };

    // Подключение кнопок
    QObject::connect(btnTask1, &QPushButton::clicked, [&]() { runTask1(); });
    QObject::connect(btnTask2, &QPushButton::clicked, [&]() { runTask2(); });
    QObject::connect(btnTask3, &QPushButton::clicked, [&]() { runTask3(); });

    QObject::connect(btnScen1, &QPushButton::clicked, [&]() { runAutopilotScenario(1, statusBadge, altBar, telemetryLabel); });
    QObject::connect(btnScen2, &QPushButton::clicked, [&]() { runAutopilotScenario(2, statusBadge, altBar, telemetryLabel); });
    QObject::connect(btnScen3, &QPushButton::clicked, [&]() { runAutopilotScenario(3, statusBadge, altBar, telemetryLabel); });
    QObject::connect(btnScen4, &QPushButton::clicked, [&]() { runAutopilotScenario(4, statusBadge, altBar, telemetryLabel); });
    QObject::connect(btnScen5, &QPushButton::clicked, [&]() { runAutopilotScenario(5, statusBadge, altBar, telemetryLabel); });

    QObject::connect(btnRunAll, &QPushButton::clicked, runAllTests);
    QObject::connect(btnClear, &QPushButton::clicked, [&]() { terminal->clear(); });

    // Автоматический запуск всех тестов при старте программы
    runAllTests();

    window.show();
    return app.exec();
}