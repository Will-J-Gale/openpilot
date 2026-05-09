#include <cstdlib>
#include <iostream>
#include <string>

#include "ajarpilot/ui/include/MainWindow.h"
#include <Constants.h>

MainWindow::MainWindow(QWidget* parent) : QWidget(parent)
{
    horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->setMargin(0);
    horizontalLayout->setSpacing(0);

    QMetaObject::connectSlotsByName(this);

    // No system background
    setStyleSheet(R"(
        * {
            font-family: Inter;
            outline: none;
        }

        QPushButton {
            color: white;
            background: black;
            border: 2px solid red;
            font-size: 100pt;
            font-weight: bold;
        }
    )");
    setAttribute(Qt::WA_NoSystemBackground);
}