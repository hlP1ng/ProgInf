#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- ПОДГОТОВКА СЦЕНЫ (чтобы было видно масштабирование) ---
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    // Добавим красный круг в центр, чтобы видеть результат работы таймера и масштабирования
    item = scene->addEllipse(-25, -25, 50, 50, QPen(Qt::black), QBrush(Qt::red));

    // --- НАСТРОЙКА ТАЙМЕРА (Задача 9) ---
    timer = new QTimer(this);

    // Соединяем сигнал таймера со слотом onTimerTimeout
    connect(timer, &QTimer::timeout, this, &MainWindow::onTimerTimeout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ===================================================================
// ЗАДАЧА 9: Кнопка Старт / Пауза
// ===================================================================
void MainWindow::on_btnStartPause_clicked()
{
    // 1. Проверяем, активен ли таймер в данный момент
    if (timer->isActive()) {
        // 2. Если активен: останавливаем и меняем текст на «Старт»
        timer->stop();
        ui->btnStartPause->setText("Start");
    } else {
        // 3. Если не активен: запускаем с интервалом 20 мс и меняем текст на «Пауза»
        timer->start(20);
        ui->btnStartPause->setText("Pause");
    }
}

// Вспомогательная функция: вызывается каждые 20 мс при работе таймера
void MainWindow::onTimerTimeout()
{
    // Просто поворачиваем наш круг для визуализации симуляции
    item->setRotation(item->rotation() + 2);
}

// ===================================================================
// ЗАДАЧА 10: Масштабирование вида (Zoom In / Zoom Out)
// ===================================================================

// Кнопка Zoom In (Приближение)
void MainWindow::on_btnZoomIn_clicked()
{
    // Увеличиваем масштаб вида на 10% (коэффициент 1.1)
    ui->graphicsView->scale(1.1, 1.1);
}

// Кнопка Zoom Out (Отдаление - опционально)
void MainWindow::on_btnZoomOut_clicked()
{
    // Уменьшаем масштаб вида на 10% (коэффициент 0.9)
    ui->graphicsView->scale(0.9, 0.9);
}