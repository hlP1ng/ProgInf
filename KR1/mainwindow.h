#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>
#include <cstdint>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Спусковые слоты для кнопок (вызываются при клике)
    void on_btnSend_clicked();
    void on_btnNoise_clicked();
    void on_btnCheck_clicked();

private:
    Ui::MainWindow *ui;

    // Данные пакетов
    QByteArray originalPacket; // Исходный пакет
    QByteArray receivedPacket; // Принятый пакет (с возможной помехой)

    // Вспомогательные функции
    uint16_t crc16(const QByteArray &data);     // Расчёт CRC16
    QString packetToHex(const QByteArray &pkt); // Перевод байтов в Hex-строку
    void log(const QString &message);          // Запись в QTextEdit
};

#endif // MAINWINDOW_H