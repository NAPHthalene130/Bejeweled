#include "AuthNetData.h"

AuthNetData::AuthNetData(QObject* parent) : QObject(parent)
{
}

AuthNetData::~AuthNetData()
{
}

int AuthNetData::getType()
{
    return type;
}

std::string AuthNetData::getId()
{
    return id;
}

std::string AuthNetData::getPassword()
{
    return password;
}

std::string AuthNetData::getEmail()
{
    return email;
}

std::string AuthNetData::getData()
{
    return data;
}

void AuthNetData::setType(int type)
{
    this->type = type;
}

void AuthNetData::setId(std::string id)
{
    this->id = id;
}

void AuthNetData::setPassword(std::string password)
{
    this->password = password;
}

void AuthNetData::setEmail(std::string email)
{
    this->email = email;
}

void AuthNetData::setData(std::string data)
{
    this->data = data;
}

void to_json(nlohmann::json& j, const AuthNetData& p) {
    j = nlohmann::json{
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

//校验账号密码合法性
bool AuthNetData::validate() const
{
    // 校验账号：非空且长度在6-20位（字母+数字）
    if (id.empty() || id.length() < 6 || id.length() > 20) return false;
    std::regex idRegex("^[a-zA-Z0-9]+$");
    if (!std::regex_match(id, idRegex)) return false;

    // 校验密码：非空且长度在8-20位（至少包含字母和数字）
    if (password.empty() || password.length() < 8 || password.length() > 20) return false;
    std::regex pwdRegex("^(?=.*[a-zA-Z])(?=.*[0-9]).+$");
    if (!std::regex_match(password, pwdRegex)) return false;
    
    return true;
}

// 登录请求处理
void AuthNetData::handleLoginRequest()
{
    //1. 校验数据合法性
    if (!validate()) {
        emit loginResult(false, "账号密码格式错误");
        return;
    }

    //2. 网络请求
    

    // 示例：模拟网络发送成功后的逻辑
    // 这里可以调用Qt的QNetworkAccessManager发送POST请求到后端接口
    // 也可以调用第三方网络库（如libcurl）进行通信
    
    // 第三步：后续可扩展处理后端响应（如解析响应数据、返回登录结果）
    emit loginResult(true, "登录成功");
}

// 注册请求处理
void AuthNetData::handleRegisterRequest()
{
    if (!validate()) {
        emit registerResult(false, "账号密码格式错误");
        return;
    }

    emit registerResult(true, "登录成功");
}