#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QWidget>
#include <QTcpSocket>
#include <QAbstractSocket>
#include "../widgets/LoginWidget.h"
#include "../widgets/RegisterWidget.h"
#include "AuthNetData.h"

class AuthWindow : public QWidget {
    Q_OBJECT

public:
    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow();

    // 切换显示的界面
    void switchWidget(QWidget* widget);

private:
    LoginWidget* loginWidget;    // 登录界面
    RegisterWidget* registerWidget; // 注册界面

    // 从AuthNetData迁移的方法声明
    bool validate(AuthNetData& data) const;
    void handleLoginRequest(AuthNetData* data);
    void handleRegisterRequest(AuthNetData* data);
    void handleRequestEmailCode(AuthNetData* data);
    void onErrorOccurred(QAbstractSocket::SocketError error, AuthNetData* data);
    void onReadyRead(AuthNetData* data);
};

#endif // AUTHWINDOW_H