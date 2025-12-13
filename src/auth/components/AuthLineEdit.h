#ifndef AUTH_LINE_EDIT_H
#define AUTH_LINE_EDIT_H

#include <QLineEdit>

// 登录注册模块专用输入框组件
class AuthLineEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit AuthLineEdit(const QString& placeholder, QWidget* parent = nullptr);
};

#endif // AUTH_LINE_EDIT_H