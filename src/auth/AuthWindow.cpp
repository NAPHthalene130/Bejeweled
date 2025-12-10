#include "AuthWindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <json.hpp>

AuthWindow::AuthWindow(QWidget *parent)
    : QWidget(parent)
{
    resize(1600, 1000);
}

AuthWindow::~AuthWindow()
{
}
