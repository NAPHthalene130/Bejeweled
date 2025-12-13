#include "AuthWindow.h"
#include "../auth/AuthNetData.h"
#include "../game/GameWindow.h"
#include "components/AuthNoticeDialog.h"
#include <QVBoxLayout>
#include <QString>
#include <QMetaEnum>
#include <regex>
#include <json.hpp>
#include <stdexcept>
#include <iostream>
#include <QDialog>

AuthWindow::AuthWindow(QWidget *parent) : QWidget(parent), socket(new QTcpSocket(this)) {
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
        AuthNetData authData;
        authData.setType(1);
        authData.setId(id.toStdString());
        authData.setPassword(password.toStdString());
        
        // 连接登录结果信号，处理登录结果
        QMetaObject::Connection* conn = new QMetaObject::Connection;
        *conn = 
        connect(this, &AuthWindow::loginResult, this, [=](bool success, const QString& msg) {
            if (success) {// 登录成功
                GameWindow* mainUI = new GameWindow();
                mainUI->show();
                this->hide();
            } else {// 登录失败弹窗
                AuthNoticeDialog* dlg = new AuthNoticeDialog("登录失败", msg, 3, this);
                dlg->exec();
                delete dlg;
            }
            disconnect(*conn);
            delete conn;
        });

        handleLoginRequest(authData);
    });
    
    // 连接注册信号，处理注册数据
    connect(registerWidget, &RegisterWidget::registerClicked, this,
            [=](const QString& id, const QString& password, const QString& confirmPwd,
                const QString& email, const QString& emailCode) {
        AuthNetData authData;
        authData.setType(2);
        authData.setId(id.toStdString());
        authData.setPassword(password.toStdString());
        authData.setEmail(email.toStdString());
        authData.setData(emailCode.toStdString());
        
        QMetaObject::Connection* conn = new QMetaObject::Connection;
        *conn = connect(this, &AuthWindow::registerResult, this, [=](bool success, const QString& msg) {
            if (success) {
                AuthNoticeDialog* dlg = new AuthNoticeDialog("注册成功", msg, 1, this);
                dlg->exec();
                delete dlg;
            } else {
                AuthNoticeDialog* dlg = new AuthNoticeDialog("注册失败", msg, 3, this);
                dlg->exec();
                delete dlg;
            }
            disconnect(*conn);
            delete conn;
        });

        handleRegisterRequest(authData);
    });

    // 连接请求邮箱验证码信号
    connect(registerWidget, &RegisterWidget::requestEmailCode, this, [=](const QString& email) {
        AuthNetData authData;
        authData.setType(3);
        authData.setEmail(email.toStdString());
        
        QMetaObject::Connection* conn = new QMetaObject::Connection;
        *conn = connect(this, &AuthWindow::emailCodeResult, this, [=](bool success, const QString& msg) {
            if (success) {
                AuthNoticeDialog* dlg = new AuthNoticeDialog("提示", msg, 1, this);
                dlg->exec();
                delete dlg;
            } else {
                AuthNoticeDialog* dlg = new AuthNoticeDialog("错误", msg, 2, this);
                dlg->exec();
                delete dlg;
            }
            disconnect(*conn);
            delete conn;
        });
        
        handleRequestEmailCode(authData);
    });
    
    // 离线登录处理
    connect(loginWidget, &LoginWidget::oflLoginClicked, this, [=]() {
        GameWindow* mainUI = new GameWindow();
        mainUI->show();
        this->hide();
    });
}

AuthWindow::~AuthWindow() {
    if (socket->isOpen()) {
        socket->disconnectFromHost();
        socket->waitForDisconnected(1000);
    }
}

// 切换界面实现
void AuthWindow::switchWidget(QWidget* widget) {
    loginWidget->hide();
    registerWidget->hide();
    if (widget) {
        widget->show();
    }
}

// 数据合法性校验
bool AuthWindow::validate(AuthNetData& data) const {
    // 账号：6-20位字母数字
    std::string id = data.getId();
    if (id.empty() || id.length() < 6 || id.length() > 20) return false;
    std::regex idRegex("^[a-zA-Z0-9]+$");
    if (!std::regex_match(id, idRegex)) return false;

    // 密码：8-20位，含字母和数字
    std::string password = data.getPassword();
    if (password.empty() || password.length() < 8 || password.length() > 20) return false;
    std::regex pwdRegex("^(?=.*[a-zA-Z])(?=.*[0-9]).+$");
    if (!std::regex_match(password, pwdRegex)) return false;

    return true;
}

// 处理登录请求
void AuthWindow::handleLoginRequest(AuthNetData& data) {
    if (!validate(data)) {
        emit loginResult(false, "账号密码格式错误（账号6-20位字母数字，密码8-20位含字母和数字）");
        return;
    }

    try {
        netDataSender(data);
        AuthNetData response_data = AuthDataReceiver();

        if (response_data.getData() == "LOGIN_SUCCESS") {
            emit loginResult(true, "登录成功");
        } else if (response_data.getData() == "LOGIN_FAIL") {
            emit loginResult(false, "账号或密码错误");
        } else {
            emit loginResult(false, "未知登录错误");
        }
    } catch (const std::exception& e) {
        emit loginResult(false, QString("登录异常: ") + e.what());
    }
}

// 处理注册请求
void AuthWindow::handleRegisterRequest(AuthNetData& data) {
    if (!validate(data)) {
        emit registerResult(false, "账号密码格式错误");
        return;
    } 
    
    try {
        netDataSender(data);
        AuthNetData response_data = AuthDataReceiver();
        std::string res = response_data.getData();

        if (res == "REGISTER_SUCCESS") {
            emit registerResult(true, "注册成功");
        } else if (res == "REGISTER_FAIL_EMAILCODE") {
            emit registerResult(false, "邮箱验证码错误");
        } else if (res == "REGISTER_FAIL_ACCOUNT") {
            emit registerResult(false, "账号已存在");
        } else if (res == "REGISTER_FAIL_EMAIL") {
            emit registerResult(false, "邮箱已存在");
        } else if (res == "REGISTER_FAIL_UNKNOWN") {
            emit registerResult(false, "注册失败：未知错误");
        } else {
            emit registerResult(false, "注册失败：" + QString::fromStdString(res));
        }
    } catch (const std::exception& e) {
        emit registerResult(false, QString("注册异常: ") + e.what());
    }
}

// 处理验证码请求
void AuthWindow::handleRequestEmailCode(AuthNetData& data) {
    const std::string& email = data.getEmail();
    if (email.empty() || !std::regex_match(email, std::regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"))) {
        emit emailCodeResult(false, "邮箱格式错误");
        return;
    }
    
    try {
        netDataSender(data);
        AuthNetData response_data = AuthDataReceiver();

        if (response_data.getData() == "EMAIL_SUCCESS") {
            emit emailCodeResult(true, "验证码已发送至邮箱");
        } else {
            emit emailCodeResult(false, "验证码发送失败");
        }
    } catch (const std::exception& e) {
        emit emailCodeResult(false, QString("验证码请求异常: ") + e.what());
    }
}

// 网络发送封装
void AuthWindow::netDataSender(AuthNetData data) {
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
    socket->connectToHost(QHostAddress("127.0.0.1"), 10086);
    if (!socket->waitForConnected(3000)) {
        throw std::runtime_error("无法连接到服务器");
    }

    nlohmann::json j = data;
    socket->write(QByteArray::fromStdString(j.dump()));
    if (!socket->waitForBytesWritten(3000)) {
        throw std::runtime_error("发送数据超时");
    }
}

// 网络接收封装
AuthNetData AuthWindow::AuthDataReceiver() {
    if (!socket->waitForReadyRead(5000)) {
        throw std::runtime_error("服务器响应超时");
    }

    QByteArray responseData = socket->readAll();
    socket->disconnectFromHost();
    
    try {
        std::string responseStr = responseData.toStdString();
        nlohmann::json j = nlohmann::json::parse(responseStr);
        AuthNetData response_data;
        from_json(j, response_data);
        return response_data;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("数据解析失败: ") + e.what());
    }
}