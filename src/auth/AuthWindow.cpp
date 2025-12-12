#include "AuthWindow.h"
#include "../auth/AuthNetData.h"
#include "../gamewidget/MainInterface.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QString>
#include <QMetaEnum>
#include <regex>
#include <json.hpp>

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
        authData->setType(1);
        authData->setId(id.toStdString());
        authData->setPassword(password.toStdString());
        
        // 连接socket信号
        connect(authData->socket, &QTcpSocket::readyRead, this, 
                [=]() { onReadyRead(authData); });
        connect(authData->socket, &QTcpSocket::errorOccurred, this,
                [=](QAbstractSocket::SocketError error) { onErrorOccurred(error, authData); });
        
        // 连接登录结果信号，处理登录结果
        connect(authData, &AuthNetData::loginResult, this, [=](bool success, const QString& msg) {
            if (success) {// 登录成功弹窗
                QMessageBox::information(this , "登录成功" , msg , QMessageBox::Ok );

                MainInterface* mainUI = new MainInterface();
                mainUI->show();
                this->hide();
            } else {// 登录失败弹窗
                QMessageBox::critical(this , "登录失败",
                    msg , QMessageBox::Ok);
            }
        });

        handleLoginRequest(authData);
    });
    
    // 连接注册信号，处理注册数据
    connect(registerWidget, &RegisterWidget::registerClicked, this,
            [=](const QString& id, const QString& password, const QString& confirmPwd,
                const QString& email, const QString& emailCode) {
        AuthNetData* authData = new AuthNetData(this);
        authData->setType(2);
        authData->setId(id.toStdString());
        authData->setPassword(password.toStdString());
        authData->setEmail(email.toStdString());
        authData->setData(emailCode.toStdString());
        
        // 连接socket信号
        connect(authData->socket, &QTcpSocket::readyRead, this,
                [=]() { onReadyRead(authData); });
        connect(authData->socket, &QTcpSocket::errorOccurred, this,
                [=](QAbstractSocket::SocketError error) { onErrorOccurred(error, authData); });
        
        // 连接注册结果信号，处理注册结果
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

        handleRegisterRequest(authData);
    });

    // 连接请求邮箱验证码信号
    connect(registerWidget, &RegisterWidget::requestEmailCode, this, [=](const QString& email) {
        AuthNetData* authData = new AuthNetData(this);
        authData->setType(3);
        authData->setEmail(email.toStdString());
        
        // 连接socket信号
        connect(authData->socket, &QTcpSocket::readyRead, this,
                [=]() { onReadyRead(authData); });
        connect(authData->socket, &QTcpSocket::errorOccurred, this,
                [=](QAbstractSocket::SocketError error) { onErrorOccurred(error, authData); });
        
        connect(authData, &AuthNetData::emailCodeResult, this, [=](bool success, const QString& msg) {
            if (success) {
                QMessageBox::information(this, "提示", msg);
            } else {
                QMessageBox::warning(this, "错误", msg);
            }
        });
        
        handleRequestEmailCode(authData);
    });
    
    // 离线登录处理
    connect(loginWidget, &LoginWidget::oflLoginClicked, this, [=]() {
        MainInterface* mainUI = new MainInterface();
        mainUI->show();
        this->hide();
    });
}

AuthWindow::~AuthWindow() {
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
void AuthWindow::handleLoginRequest(AuthNetData* data) {
    if (!validate(*data)) {
        emit data->loginResult(false, "账号密码格式错误（账号6-20位字母数字，密码8-20位含字母和数字）");
        return;
    }
    data->socket->connectToHost(QHostAddress("127.0.0.1"), 10086);
    nlohmann::json j = *data;
    data->socket->write(QByteArray::fromStdString(j.dump()));
}

// 处理注册请求
void AuthWindow::handleRegisterRequest(AuthNetData* data) {
    if (!validate(*data)) {
        emit data->registerResult(false, "账号密码格式错误");
        return;
    }
    data->socket->connectToHost(QHostAddress("127.0.0.1"), 10086);
    nlohmann::json j = *data;
    data->socket->write(QByteArray::fromStdString(j.dump()));
}

// 处理验证码请求
void AuthWindow::handleRequestEmailCode(AuthNetData* data) {
    const std::string& email = data->getEmail();
    if (email.empty() || !std::regex_match(email, std::regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"))) {
        emit data->emailCodeResult(false, "邮箱格式错误");
        return;
    }
    data->setType(3); // 标记为验证码请求
    data->socket->connectToHost(QHostAddress("127.0.0.1"), 10086);
    nlohmann::json j = *data;
    data->socket->write(QByteArray::fromStdString(j.dump()));
}

// 错误处理
void AuthWindow::onErrorOccurred(QAbstractSocket::SocketError error, AuthNetData* data) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketError>();
    QString errorStr = metaEnum.valueToKey(error);
    emit data->loginResult(false, "网络错误: " + errorStr);
}

// 数据接收处理
void AuthWindow::onReadyRead(AuthNetData* data) {
    QByteArray responseData = data->socket->readAll();
    std::string responseStr = responseData.toStdString();

    try {
        nlohmann::json j = nlohmann::json::parse(responseStr);
        AuthNetData response_data(this);
        from_json(j, response_data);

        // 根据请求类型处理响应
        switch (response_data.getType()) {
            case 1: // 登录响应
                if (response_data.getData() == "LOGIN_SUCCESS") {
                    emit data->loginResult(true, "登录成功");
                } else {
                    emit data->loginResult(false, "账号或密码错误");
                }
                break;
            case 2: // 注册响应
                if (response_data.getData() == "REGISTER_SUCCESS") {
                    emit data->registerResult(true, "注册成功");
                } else {
                    emit data->registerResult(false, "注册失败：" + QString::fromStdString(response_data.getData()));
                }
                break;
            case 3: // 验证码响应
                if (response_data.getData() == "CODE_SENT") {
                    emit data->emailCodeResult(true, "验证码已发送至邮箱");
                } else {
                    emit data->emailCodeResult(false, "验证码发送失败");
                }
                break;
            default:
                emit data->loginResult(false, "服务器响应类型错误");
        }
    } catch (const std::exception& e) {
        emit data->loginResult(false, "解析响应失败: " + QString::fromStdString(e.what()));
    }

    data->socket->disconnectFromHost();
}