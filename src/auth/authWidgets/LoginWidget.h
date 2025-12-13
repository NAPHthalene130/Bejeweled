#ifndef LOGIN_WIDGET_H
#define LOGIN_WIDGET_H

#include <QWidget>
#include "../components/AuthLineEdit.h"
#include "../components/AuthButton.h"
class LoginWidget : public QWidget {
    Q_OBJECT
signals:
    // 切换到注册界面的信号
    void switchToRegister();
    // 登录按钮点击信号（传递账号密码）
    void loginClicked(const QString& id, const QString& password);
    //离线登录按钮点击信号 (直接进入游戏)
    void oflLoginClicked();
public:
    explicit LoginWidget(QWidget* parent = nullptr);

private:
    AuthLineEdit* idEdit;      // 账号输入框
    AuthLineEdit* passwordEdit;// 密码输入框
    AuthButton* loginBtn;      // 登录按钮
    AuthButton* oflLoginBtn;   // 离线登录按钮
    AuthButton* toRegisterBtn; // 切换注册按钮
};

#endif // LOGIN_WIDGET_H