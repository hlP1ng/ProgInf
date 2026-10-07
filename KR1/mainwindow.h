#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>
#include <cstdint>

// Подключаем модули графики и анимации Qt
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QVariantAnimation>

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
    void on_btnSend_clicked();
    void on_btnNoise_clicked();
    void on_btnCheck_clicked();

private:
    Ui::MainWindow *ui;

    QByteArray originalPacket;
    QByteArray receivedPacket;

    // --- ЭЛЕМЕНТЫ ГРАФИКИ И АНИМАЦИИ ---
    QGraphicsScene *scene;                // Графическая сцена
    QGraphicsRectItem *packetItem;        // Пакет (летающий прямоугольник)
    QGraphicsTextItem *packetTextItem;    // Текст внутри пакета
    QVariantAnimation *flyAnimation;      // Анимация полета пакета

    uint16_t crc16(const QByteArray &data);
    QString packetToHex(const QByteArray &pkt);
    void log(const QString &message);

    // Функция запуска анимации полета
    void startFlyAnimation();
};

#endif // MAINWINDOW_H