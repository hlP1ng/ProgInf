#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <cstdlib>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Вариант 6 — Помехоустойчивое кодирование (CRC16)");
    log("Программа готова к работе.");
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ------------------------------------------------------------------
// 1. Алгоритм вычисления контрольной суммы CRC16 (Polynomial: 0xA001)
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

// Вспомогательная функция вывода Hex-байтов в понятном виде
QString MainWindow::packetToHex(const QByteArray &pkt)
{
    if (pkt.isEmpty()) return "-";

    // Преобразуем массив байт в Hex-строку с пробелами
    QString hex = pkt.toHex(' ').toUpper();
    return hex;
}

// Вспомогательная функция логирования
void MainWindow::log(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss] ");
    ui->textLog->append(timestamp + message);
}

// ------------------------------------------------------------------
// 2. Нажатие кнопки «Сформировать пакет»
// ------------------------------------------------------------------
void MainWindow::on_btnSend_clicked()
{
    QString cmdStr = ui->lineEditCmd->text();
    if (cmdStr.isEmpty()) {
        log("Ошибка: введите команду!");
        return;
    }

    // Преобразуем текст команды в байты
    QByteArray cmd = cmdStr.toUtf8();

    // Считаем CRC16 от текста
    uint16_t crc = crc16(cmd);

    // Упаковываем: Команда + Младший байт CRC + Старший байт CRC
    originalPacket = cmd;
    originalPacket.append(static_cast<char>(crc & 0xFF));
    originalPacket.append(static_cast<char>((crc >> 8) & 0xFF));

    // По умолчанию принятый пакет совпадает с исходным
    receivedPacket = originalPacket;

    // Выводим в интерфейс
    ui->labelOriginal->setText("Исходный пакет: " + packetToHex(originalPacket));
    ui->labelReceived->setText("Принятый пакет: " + packetToHex(receivedPacket));
    ui->labelResult->setText("Результат: Ожидает проверки");

    log("Пакет сформирован: " + cmdStr + " | CRC = 0x" + QString::number(crc, 16).toUpper());
}

// ------------------------------------------------------------------
// 3. Нажатие кнопки «Имитация помехи»
// ------------------------------------------------------------------
void MainWindow::on_btnNoise_clicked()
{
    if (receivedPacket.size() < 3) {
        log("Ошибка: Сначала сформируйте пакет!");
        return;
    }

    // Выбираем случайный байт для искажения (из всех байтов пакета)
    int bytePos = rand() % receivedPacket.size();

    // Выбираем случайный бит в этом байте (от 0 до 7)
    int bitPos = rand() % 8;

    // Инвертируем бит с помощью операции XOR (^)
    receivedPacket[bytePos] = receivedPacket[bytePos] ^ (1 << bitPos);

    // Обновляем UI
    ui->labelReceived->setText("Принятый пакет: " + packetToHex(receivedPacket));
    log(QString("Помеха: инвертирован бит %1 в байте №%2").arg(bitPos).arg(bytePos));
}

// ------------------------------------------------------------------
// 4. Нажатие кнопки «Проверить»
// ------------------------------------------------------------------
void MainWindow::on_btnCheck_clicked()
{
    if (receivedPacket.size() < 3) {
        log("Ошибка: Сначала сформируйте пакет!");
        return;
    }

    // Отделяем данные от полученной CRC (последние 2 байта)
    QByteArray payload = receivedPacket.left(receivedPacket.size() - 2);

    // Извлекаем пришедшую CRC
    uint8_t lowByte = static_cast<uint8_t>(receivedPacket[receivedPacket.size() - 2]);
    uint8_t highByte = static_cast<uint8_t>(receivedPacket[receivedPacket.size() - 1]);
    uint16_t receivedCrc = lowByte | (highByte << 8);

    // Вычисляем CRC заново по полученным данным
    uint16_t computedCrc = crc16(payload);

    // Сравниваем
    if (receivedCrc == computedCrc) {
        ui->labelResult->setText("Результат: ОК — пакет целостный");
        log("Проверка: ОК (Ошибок не обнаружено)");
    } else {
        ui->labelResult->setText("Результат: ОШИБКА CRC — пакет повреждён!");
        log(QString("Проверка: ОШИБКА CRC (Ожидалось 0x%1, получено 0x%2)")
                .arg(computedCrc, 4, 16, QChar('0'))
                .arg(receivedCrc, 4, 16, QChar('0')).toUpper());
    }
}