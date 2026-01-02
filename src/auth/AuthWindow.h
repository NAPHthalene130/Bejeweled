#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QWidget>
#include <QTcpSocket>
#include <QAbstractSocket>
#include "authWidgets/LoginWidget.h"
#include "authWidgets/RegisterWidget.h"
#include "AuthNetData.h"

class AuthWindow : public QWidget {
    Q_OBJECT

public:
    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow() override;

    // 切换显示的界面
    void switchWidget(QWidget* widget);

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    // 从AuthNetData迁移过来的信号
    void loginResult(bool success, const QString& msg);
    void registerResult(bool success, const QString& msg);

private:
    LoginWidget* loginWidget;    // 登录界面
    RegisterWidget* registerWidget; // 注册界面
    QTcpSocket* socket; // 从AuthNetData迁移过来的socket
    QPixmap backgroundPixmap;

    bool validate(AuthNetData& data) const;
    void handleLoginRequest(AuthNetData& data);
    void handleRegisterRequest(AuthNetData& data);
    
    // 网络收发封装
    void netDataSender(AuthNetData data);
    AuthNetData AuthDataReceiver();

};

#endif // AUTHWINDOW_H