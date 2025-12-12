#ifndef AUTH_NET_DATA_H
#define AUTH_NET_DATA_H
#include <string>
#include <json.hpp>
#include <QObject>
#include <QTcpSocket>

class AuthNetData : public QObject {
    Q_OBJECT
    using string = std::string;
signals:
    void loginResult(bool success, const QString& msg);
    void registerResult(bool success, const QString& msg);
    void emailCodeResult(bool success, const QString& msg);

public:
    AuthNetData(QObject* parent = nullptr);
    ~AuthNetData() override;

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

    // 友元函数保留（JSON序列化）
    friend void to_json(nlohmann::json& j, const AuthNetData& p);
    friend void from_json(const nlohmann::json& j, AuthNetData& p);

    QTcpSocket* socket; // 保留socket成员供外部使用
private:
    int type; // 1:登录 2:注册 3:请求验证码
    string id;
    string password;
    string email;
    string data; // 存储验证码
};

#endif // AUTH_NET_DATA_H