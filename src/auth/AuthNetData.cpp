#include "AuthNetData.h"
#include <QByteArray>
#include <stdexcept>

AuthNetData::AuthNetData(QObject* parent) : QObject(parent), socket(new QTcpSocket(this)) {}

AuthNetData::~AuthNetData() {
    if (socket->isOpen()) {
        socket->disconnectFromHost();
        socket->waitForDisconnected(1000);
    }
}

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

// JSON序列化
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