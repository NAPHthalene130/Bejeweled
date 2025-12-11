#include "AuthNetData.h"

namespace asio = boost::asio;
using tcp = asio::ip::tcp;
using json = nlohmann::json;

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

    // 2. 创建网络连接并发送数据
    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        
        // 连接到本地服务器（127.0.0.1:10086）
        boost::system::error_code ec;
        socket.connect(tcp::endpoint(asio::ip::make_address("127.0.0.1"), 10086), ec);
        
        if (ec) {
            emit loginResult(false, "连接服务器失败: " + QString::fromStdString(ec.message()));
            return;
        }

        // 3. 序列化登录数据
        nlohmann::json j = *this;
        std::string sendData = j.dump();

        // 4. 发送数据到服务器
        asio::write(socket, boost::asio::buffer(sendData), ec);
        if (ec) {
            emit loginResult(false, "发送数据失败: " + QString::fromStdString(ec.message()));
            return;
        }

        // 5. 接收服务器响应
        std::vector<char> buffer(4096);
        size_t bytesRead = socket.read_some(boost::asio::buffer(buffer), ec);
        if (ec) {
            emit loginResult(false, "接收响应失败: " + QString::fromStdString(ec.message()));
            return;
        }

        // 6. 解析响应数据
        std::string responseStr(buffer.data(), bytesRead);
        nlohmann::json responseJson = nlohmann::json::parse(responseStr);
        AuthNetData responseData = responseJson.get<AuthNetData>();

        // 7. 根据服务器返回结果发射信号
        if (responseData.getType() == 1) {
            if (responseData.getData() == "LOGIN_SUCCESS") {
                emit loginResult(true, "登录成功");
            } else {
                emit loginResult(false, "账号或密码错误");
            }
        } else {
            emit loginResult(false, "服务器返回异常");
        }

    } catch (const std::exception& e) {
        emit loginResult(false, "网络错误: " + QString::fromStdString(e.what()));
    }
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