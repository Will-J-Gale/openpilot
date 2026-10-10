#include <cstdlib>
#include <iostream>
#include <string>
#include <iostream>
#include <fstream>

#include "ajarpilot/selector/main_window.h"

MainWindow::MainWindow(QWidget* parent) : QWidget(parent)
{
    horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->setMargin(0);
    horizontalLayout->setSpacing(0);

    ajarpilotButton = new QPushButton(this);
    ajarpilotButton->setObjectName(QString::fromUtf8("ajarpilot"));
    ajarpilotButton->setText(QString::fromUtf8("ajarpilot"));
    QObject::connect(ajarpilotButton, &QPushButton::clicked, [this]() { this->runAjarpilot(); });

    openpilotButton = new QPushButton(this);
    openpilotButton->setObjectName(QString::fromUtf8("openpilot"));

    std::string openpilot_text = "openpilot\n(" + std::to_string(timeRemaining) + ")";
    openpilotButton->setText(QString::fromUtf8(openpilot_text.c_str()));
    QObject::connect(openpilotButton, &QPushButton::clicked, [this]() { this->runOpenpilot(); });

    QSizePolicy sPolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    sPolicy.setHorizontalStretch(0);
    sPolicy.setVerticalStretch(0);
    sPolicy.setHeightForWidth(ajarpilotButton->sizePolicy().hasHeightForWidth());
    ajarpilotButton->setSizePolicy(sPolicy);
    openpilotButton->setSizePolicy(sPolicy);

    horizontalLayout->addWidget(ajarpilotButton);
    horizontalLayout->addWidget(openpilotButton);

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

    timer = new QTimer(this);
    QObject::connect(timer, &QTimer::timeout, this, [this](){this->decrementTimer(); });
    timer->start(1000);
}

void MainWindow::runAjarpilot()
{
    set_launch_program("ajarpilot");
    QCoreApplication::quit();
}

void MainWindow::runOpenpilot()
{
    set_launch_program("openpilot");
    QCoreApplication::quit();
}

void MainWindow::decrementTimer()
{
    timeRemaining -= 1;

    if(timeRemaining <= 0)
        this->runOpenpilot();

    std::string openpilot_text = "openpilot\n(" + std::to_string(timeRemaining) + ")";
    openpilotButton->setText(QString::fromUtf8(openpilot_text.c_str()));

    update();
}


void MainWindow::set_launch_program(const char* program)
{
  std::ofstream launch_file;
  launch_file.open("/data/.launch_program");
  launch_file << program;
  launch_file.close();
}