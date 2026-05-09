#pragma once
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <QtCore/QTimer>


class MainWindow : public QWidget
{
public:

    MainWindow(QWidget* parent = 0);

private:
    QHBoxLayout* horizontalLayout;
    QPushButton* ajarpilotButton;
    QPushButton* openpilotButton;
    QTimer* timer;
    int timeRemaining = 10;

    void runAjarpilot();
    void runOpenpilot();
    void decrementTimer();
};
