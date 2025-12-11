#ifndef AUTH_NET_DATA_H
#define AUTH_NET_DATA_H
#include <string>
#include <json.hpp>
#include <regex>
#include <QString>
#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>

class AuthNetData : public QObject {
    Q_OBJECT
    using string = std::string;
signals:
    void loginResult(bool success, const QString& msg);
    void registerResult(bool success, const QString& msg);
    // 验证码相关信号
    void emailCodeResult(bool success, const QString& msg);

public:
    AuthNetData(QObject* parent = nullptr);
    ~AuthNetData() override;

    // 原有 getter/setter
    int getType();
    string getId();
    string getPassword();
    string getEmail();
    string getData();
    void setType(int type);
    void setId(string id);
    void setPassword(string password);
    void setEmail(string email);
    void setData(string data);

    bool validate() const;                // 校验账号密码合法性
    void handleLoginRequest();            // 处理登录请求
    void handleRegisterRequest();         // 处理注册请求
    void handleRequestEmailCode();        // 新增：处理验证码请求

private:
    int type; // 1:登录 2:注册 3:请求验证码
    string id;
    string password;
    string email;
    string data; // 存储验证码
    QTcpSocket* socket;
    void onErrorOccurred(QAbstractSocket::SocketError error); // 错误处理
    void onReadyRead(); // 读取响应处理
    friend void to_json(nlohmann::json& j, const AuthNetData& p);
    friend void from_json(const nlohmann::json& j, AuthNetData& p);
};

#endif // AUTH_NET_DATA_H