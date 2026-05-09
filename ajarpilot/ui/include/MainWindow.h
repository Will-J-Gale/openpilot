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
};
