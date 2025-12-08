#include "LoginWindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    auto *label = new QLabel("Login Window", this);
    auto *button = new QPushButton("Login", this);
    
    layout->addWidget(label);
    layout->addWidget(button);
    
    setLayout(layout);
    resize(400, 300);
}

LoginWindow::~LoginWindow()
{
}
