#ifndef AUTH_BUTTON_H
#define AUTH_BUTTON_H

#include <QPushButton>

// 登录注册模块专用按钮组件
class AuthButton : public QPushButton {
    Q_OBJECT
public:
    explicit AuthButton(const QString& text, QWidget* parent = nullptr);
};

#endif // AUTH_BUTTON_H