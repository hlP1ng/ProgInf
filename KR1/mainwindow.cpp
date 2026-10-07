#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <cstdlib>
#include <QGraphicsItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Вариант 6 — Помехоустойчивое кодирование (CRC16) + Анимация");

    // =================================----------------================
    // ИНИЦИАЛИЗА ГРАФИЧЕСКОЙ СЦЕНЫ И АНИМАЦИИ
    // =================================----------------================
    scene = new QGraphicsScene(0, 0, 500, 100, this);
    ui->graphicsView->setScene(scene);

    // 1. Передатчик (TX) - синий блок слева
    scene->addRect(10, 20, 70, 60, QPen(Qt::black), QBrush(QColor(180, 210, 255)));
    QGraphicsTextItem *txText = scene->addText("НСУ\n(TX)");
    txText->setPos(22, 30);

    // 2. Канал связи - пунктирная линия по центру
    scene->addLine(80, 50, 420, 50, QPen(Qt::DashLine));

    // 3. Приёмник (RX) - зелёный блок справа
    scene->addRect(420, 20, 70, 60, QPen(Qt::black), QBrush(QColor(180, 255, 180)));
    QGraphicsTextItem *rxText = scene->addText("БПЛА\n(RX)");
    rxText->setPos(432, 30);

    // 4. Создаем пакет (прямоугольник), который будет летать
    packetItem = scene->addRect(-35, -15, 70, 30, QPen(Qt::black, 2), QBrush(Qt::green));
    packetTextItem = scene->addText("DATA");
    packetTextItem->setParentItem(packetItem); // Привязываем текст к пакету
    packetTextItem->setPos(-30, -13);

    // Скрываем пакет до нажатия кнопки
    packetItem->setVisible(false);

    // 5. Настройка аниматора перемещения (от X=110 до X=385)
    flyAnimation = new QVariantAnimation(this);
    flyAnimation->setDuration(800); // Длительность полёта - 0.8 секунды
    connect(flyAnimation, &QVariantAnimation::valueChanged, this, [=](const QVariant &value){
        packetItem->setPos(value.toPointF());
    });

    log("Программа готова к работе.");
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ------------------------------------------------------------------
// Алгоритм CRC16
// ------------------------------------------------------------------
uint16_t MainWindow::crc16(const QByteArray &data)
{
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<uint8_t>(data[i]);
        for (int j = 0; j < 8; ++j) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

QString MainWindow::packetToHex(const QByteArray &pkt)
{
    if (pkt.isEmpty()) return "-";
    return pkt.toHex(' ').toUpper();
}

void MainWindow::log(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss] ");
    ui->textLog->append(timestamp + message);
}

// Вспомогательный метод запуска анимации
void MainWindow::startFlyAnimation()
{
    packetItem->setVisible(true);
    flyAnimation->setStartValue(QPointF(110, 50));
    flyAnimation->setEndValue(QPointF(385, 50));
    flyAnimation->start();
}

// ------------------------------------------------------------------
// 1. Кнопка «Сформировать пакет»
// ------------------------------------------------------------------
void MainWindow::on_btnSend_clicked()
{
    QString cmdStr = ui->lineEditCmd->text();
    if (cmdStr.isEmpty()) {
        log("Ошибка: введите команду!");
        return;
    }

    QByteArray cmd = cmdStr.toUtf8();
    uint16_t crc = crc16(cmd);

    originalPacket = cmd;
    originalPacket.append(static_cast<char>(crc & 0xFF));
    originalPacket.append(static_cast<char>((crc >> 8) & 0xFF));

    receivedPacket = originalPacket;

    ui->labelOriginal->setText("Исходный пакет: " + packetToHex(originalPacket));
    ui->labelReceived->setText("Принятый пакет: " + packetToHex(receivedPacket));
    ui->labelResult->setText("Результат: Ожидает проверки");

    // НАСТРОЙКА АНИМАЦИИ: пакет чистый (зелёный)
    packetItem->setBrush(QBrush(QColor(100, 255, 100)));
    packetTextItem->setPlainText("OK (" + QString::number(crc, 16).toUpper() + ")");
    startFlyAnimation();

    log("Пакет сформирован: " + cmdStr + " | CRC = 0x" + QString::number(crc, 16).toUpper());
}

// ------------------------------------------------------------------
// 2. Кнопка «Имитация помехи»
// ------------------------------------------------------------------
void MainWindow::on_btnNoise_clicked()
{
    if (receivedPacket.size() < 3) {
        log("Ошибка: Сначала сформируйте пакет!");
        return;
    }

    int bytePos = rand() % receivedPacket.size();
    int bitPos = rand() % 8;

    receivedPacket[bytePos] = receivedPacket[bytePos] ^ (1 << bitPos);

    ui->labelReceived->setText("Принятый пакет: " + packetToHex(receivedPacket));

    // АНИМАЦИЯ ПОМЕХИ: пакет становится красным, текст меняется на NOISE!
    packetItem->setBrush(QBrush(QColor(255, 90, 90)));
    packetTextItem->setPlainText("ПОМЕХА!");

    // Перезапускаем полёт с эффектом повреждённого пакета
    startFlyAnimation();

    log(QString("Помеха: инвертирован бит %1 в байте №%2").arg(bitPos).arg(bytePos));
}

// ------------------------------------------------------------------
// 3. Кнопка «Проверить»
// ------------------------------------------------------------------
void MainWindow::on_btnCheck_clicked()
{
    if (receivedPacket.size() < 3) {
        log("Ошибка: Сначала сформируйте пакет!");
        return;
    }

    QByteArray payload = receivedPacket.left(receivedPacket.size() - 2);

    uint8_t lowByte = static_cast<uint8_t>(receivedPacket[receivedPacket.size() - 2]);
    uint8_t highByte = static_cast<uint8_t>(receivedPacket[receivedPacket.size() - 1]);
    uint16_t receivedCrc = lowByte | (highByte << 8);

    uint16_t computedCrc = crc16(payload);

    if (receivedCrc == computedCrc) {
        ui->labelResult->setText("Результат: ОК — пакет целостный");
        packetItem->setBrush(QBrush(QColor(0, 230, 0)));
        log("Проверка: ОК (Ошибок не обнаружено)");
    } else {
        ui->labelResult->setText("Результат: ОШИБКА CRC — пакет повреждён!");
        packetItem->setBrush(QBrush(QColor(255, 0, 0)));
        log(QString("Проверка: ОШИБКА CRC (Ожидалось 0x%1, получено 0x%2)")
                .arg(computedCrc, 4, 16, QChar('0'))
                .arg(receivedCrc, 4, 16, QChar('0')).toUpper());
    }
}