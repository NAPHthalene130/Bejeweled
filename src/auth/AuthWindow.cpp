#include "AuthWindow.h"
#include <QVBoxLayout>

AuthWindow::AuthWindow(QWidget *parent) : QWidget(parent) {
    resize(1600, 1000);
    setWindowTitle("登录注册");

    // 初始化子界面
    loginWidget = new LoginWidget(this);
    registerWidget = new RegisterWidget(this);

    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(loginWidget);  // 默认显示登录界面
    mainLayout->addWidget(registerWidget);
    registerWidget->hide();  // 初始隐藏注册界面

    // 连接切换信号
    connect(loginWidget, &LoginWidget::switchToRegister, this, [=]() {
        switchWidget(registerWidget);
    });
    connect(registerWidget, &RegisterWidget::switchToLogin, this, [=]() {
        switchWidget(loginWidget);
    });
}

AuthWindow::~AuthWindow() {
}

// 切换界面实现
void AuthWindow::switchWidget(QWidget* widget) {
    // 隐藏所有子界面
    loginWidget->hide();
    registerWidget->hide();
    // 显示目标界面
    if (widget) {
        widget->show();
    }
}