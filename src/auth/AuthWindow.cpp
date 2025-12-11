#include "AuthWindow.h"
#include "../auth/AuthNetData.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QString>

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

    // 连接登录信号，处理登录数据
    connect(loginWidget, &LoginWidget::loginClicked, this,
            [=](const QString& id, const QString& password) {
        AuthNetData* authData = new AuthNetData(this);
        authData -> setType(1);// 登录类型
        authData -> setId(id.toStdString()); // 账号
        authData -> setPassword(password.toStdString()); // 密码
        // 连接登录结果信号，处理登录结果
        connect(authData, &AuthNetData::loginResult, this, [=](bool success, const QString& msg) {
            if (success) {
                QMessageBox::information(// 登录成功弹窗
                    this , "登录成功",
                    msg , QMessageBox::Ok );
            } else {// 登录失败弹窗
                QMessageBox::critical(this , "登录失败",
                    msg , QMessageBox::Ok);
            }
        });
        authData -> handleLoginRequest(); // 处理登录请求
    });
    
    // 连接登录信号，处理登录数据
    connect(registerWidget, &RegisterWidget::registerClicked, this,
            [=](const QString& id, const QString& password, const QString& confirmPwd,
                const QString& email, const QString& emailCode) {
        AuthNetData* authData = new AuthNetData(this);
        authData -> setType(2);// 登录类型
        authData -> setId(id.toStdString()); // 账号
        authData -> setPassword(password.toStdString()); // 密码
        authData -> setEmail(email.toStdString()); // 邮箱
        authData -> setData(emailCode.toStdString()); // 邮箱验证码
        // 连接登录结果信号，处理登录结果
        connect(authData, &AuthNetData::registerResult, this, [=](bool success, const QString& msg) {
            if (success) {
                QMessageBox::information(// 登录成功弹窗
                    this , "注册成功",
                    msg , QMessageBox::Ok );
            } else {// 登录失败弹窗
                QMessageBox::critical(this , "注册失败，请检查信息格式",
                    msg , QMessageBox::Ok);
            }
        });
        authData -> handleRegisterRequest(); // 处理登录请求
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