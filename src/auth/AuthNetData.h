#ifndef AUTH_NET_DATA_H
#define AUTH_NET_DATA_H
#include <string>
#include <json.hpp>
#include <regex>
#include <QString>
#include <QObject>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

class AuthNetData : public QObject {
    Q_OBJECT
    using string = std::string;
signals:
    // 登录注册结果信号
    void loginResult(bool success, const QString& msg);
    void registerResult(bool success, const QString& msg);
public:
    AuthNetData(QObject* parent = nullptr);
    ~AuthNetData();
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
    //处理两项请求
    bool validate() const;// 校验账号密码合法性
    void handleLoginRequest();
    void handleRegisterRequest();
private:
    int type; //1:登录 2:注册
    string id;
    string password;
    string email;
    string data;
    
    friend void to_json(nlohmann::json& j, const AuthNetData& p);
    friend void from_json(const nlohmann::json& j, AuthNetData& p);
};

#endif // AUTH_NET_DATA_H