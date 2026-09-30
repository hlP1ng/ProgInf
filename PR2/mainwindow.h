#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>          // Подключаем таймер
#include <QGraphicsScene>  // Подключаем сцену для отображения
#include <QGraphicsEllipseItem> // Для создания круга на сцене

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
    // Слоты для кнопок
    void on_btnStartPause_clicked(); // Слот кнопки Старт/Пауза (Задача 9)
    void on_btnZoomIn_clicked();     // Слот кнопки Приблизить (Задача 10)
    void on_btnZoomOut_clicked();    // Слот кнопки Отдалить (Задача 10)

    // Слот для работы таймера (вызывается каждые 20 мс)
    void onTimerTimeout();

private:
    Ui::MainWindow *ui;

    QTimer *timer;          // Указатель на таймер
    QGraphicsScene *scene;  // Сцена для графика
    QGraphicsEllipseItem *item; // Предмет на сцене (круг)
};
#endif // MAINWINDOW_H