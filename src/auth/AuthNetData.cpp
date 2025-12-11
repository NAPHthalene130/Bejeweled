#include "AuthNetData.h"
#include <QByteArray>
#include <QMetaEnum>  // Qt 6 枚举处理
#include <stdexcept>

namespace json = nlohmann;

AuthNetData::AuthNetData(QObject* parent) : QObject(parent), socket(new QTcpSocket(this)) {
    // 连接套接字信号（处理数据接收）
    connect(socket, &QTcpSocket::readyRead, this, &AuthNetData::onReadyRead);
    // 连接错误信号（Qt 6 中 errorOccurred 传递 QAbstractSocket::SocketError 枚举）
    connect(socket, &QTcpSocket::errorOccurred, this, &AuthNetData::onErrorOccurred);
}

AuthNetData::~AuthNetData() {
    if (socket->isOpen()) {
        socket->disconnectFromHost();
        socket->waitForDisconnected(1000); // 等待断开连接
    }
}

// 数据接收处理
void AuthNetData::onReadyRead() {
    QByteArray responseData = socket->readAll();
    std::string responseStr = responseData.toStdString();

    try {
        nlohmann::json j = nlohmann::json::parse(responseStr);
        AuthNetData response_data(this);
        from_json(j, response_data);

        // 根据请求类型处理响应
        switch (response_data.getType()) {
        case 1: // 登录响应
            if (response_data.getData() == "LOGIN_SUCCESS") {
                emit loginResult(true, "登录成功");
            } else {
                emit loginResult(false, "账号或密码错误");
            }
            break;
        case 2: // 注册响应
            if (response_data.getData() == "REGISTER_SUCCESS") {
                emit registerResult(true, "注册成功");
            } else {
                emit registerResult(false, "注册失败：" + QString::fromStdString(response_data.getData()));
            }
            break;
        case 3: // 验证码响应
            if (response_data.getData() == "CODE_SENT") {
                emit emailCodeResult(true, "验证码已发送至邮箱");
            } else {
                emit emailCodeResult(false, "验证码发送失败");
            }
            break;
        default:
            emit loginResult(false, "服务器响应类型错误");
        }
    } catch (const std::exception& e) {
        emit loginResult(false, "解析响应失败: " + QString::fromStdString(e.what()));
    }

    socket->disconnectFromHost();
}

// 错误处理
void AuthNetData::onErrorOccurred(QAbstractSocket::SocketError error) {
    // 使用 QMetaEnum 获取错误信息字符串
    QMetaEnum metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketError>();
    QString errorStr = metaEnum.valueToKey(error);
    emit loginResult(false, "网络错误: " + errorStr);
}

// 缩进一下
int AuthNetData::getType() { return type; }
std::string AuthNetData::getId() { return id; }
std::string AuthNetData::getPassword() { return password; }
std::string AuthNetData::getEmail() { return email; }
std::string AuthNetData::getData() { return data; }

void AuthNetData::setType(int type) { this->type = type; }
void AuthNetData::setId(string id) { this->id = id; }
void AuthNetData::setPassword(string password) { this->password = password; }
void AuthNetData::setEmail(string email) { this->email = email; }
void AuthNetData::setData(string data) { this->data = data; }

void to_json(nlohmann::json& j, const AuthNetData& p) {
    j = nlohmann::json {
        {"type", p.type},
        {"id", p.id},
        {"password", p.password},
        {"email", p.email},
        {"data", p.data}
    };
}

void from_json(const nlohmann::json& j, AuthNetData& p) {
    j.at("type").get_to(p.type);
    j.at("id").get_to(p.id);
    j.at("password").get_to(p.password);
    j.at("email").get_to(p.email);
    j.at("data").get_to(p.data);
}

// 数据合法性校验
bool AuthNetData::validate() const {
    // 账号：6-20位字母数字
    if (id.empty() || id.length() < 6 || id.length() > 20) return false;
    std::regex idRegex("^[a-zA-Z0-9]+$");
    if (!std::regex_match(id, idRegex)) return false;

    // 密码：8-20位，含字母和数字
    if (password.empty() || password.length() < 8 || password.length() > 20) return false;
    std::regex pwdRegex("^(?=.*[a-zA-Z])(?=.*[0-9]).+$");
    if (!std::regex_match(password, pwdRegex)) return false;

    return true;
}

// 处理登录请求
void AuthNetData::handleLoginRequest() {
    if (!validate()) {
        emit loginResult(false, "账号密码格式错误（账号6-20位字母数字，密码8-20位含字母和数字）");
        return;
    }
    socket->connectToHost(QHostAddress("127.0.0.1"), 10086);
    nlohmann::json j = *this;
    socket->write(QByteArray::fromStdString(j.dump()));
}

// 处理注册请求
void AuthNetData::handleRegisterRequest() {
    if (!validate()) {
        emit registerResult(false, "账号密码格式错误");
        return;
    }
    socket->connectToHost(QHostAddress("127.0.0.1"), 10086);
    nlohmann::json j = *this;
    socket->write(QByteArray::fromStdString(j.dump()));
}

// 处理验证码请求
void AuthNetData::handleRequestEmailCode() {
    if (email.empty() || !std::regex_match(email, std::regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"))) {
        emit emailCodeResult(false, "邮箱格式错误");
        return;
    }
    setType(3); // 标记为验证码请求
    socket->connectToHost(QHostAddress("127.0.0.1"), 10086);
    nlohmann::json j = *this;
    socket->write(QByteArray::fromStdString(j.dump()));
}