#ifndef REGISTER_WIDGET_H
#define REGISTER_WIDGET_H

#include <QWidget>
#include <QLabel>
#include "../components/AuthLineEdit.h"
#include "../components/AuthButton.h"

class RegisterWidget : public QWidget {
    Q_OBJECT
signals:
    // 切换到登录界面的信号
    void switchToLogin();
    // 注册按钮点击信号（传递注册信息）
    void registerClicked(const QString& id, const QString& password, 
                         const QString& confirmPwd, const QString& email, 
                         const QString& emailCode);
    // 请求邮箱验证码信号
    void requestEmailCode(const QString& email);

public:
    explicit RegisterWidget(QWidget* parent = nullptr);

private:
    AuthLineEdit* idEdit;         // 账号输入框
    AuthLineEdit* passwordEdit;   // 密码输入框
    AuthLineEdit* confirmPwdEdit; // 确认密码输入框
    AuthLineEdit* emailEdit;      // 邮箱输入框
    AuthLineEdit* emailCodeEdit;  // 邮箱验证码输入框
    AuthButton* registerBtn;      // 注册按钮
    AuthButton* toLoginBtn;       // 切换登录按钮
    AuthButton* getCodeBtn;       // 获取验证码按钮

    QLabel* idHintLabel;          // 账号提示标签
    QLabel* passwordHintLabel;    // 密码提示标签
    QLabel* emailHintLabel;       // 邮箱提示标签
};

#endif // REGISTER_WIDGET_H

