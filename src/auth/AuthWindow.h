#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QWidget>
#include "../widgets/LoginWidget.h"
#include "../widgets/RegisterWidget.h"
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
};

#endif // AUTHWINDOW_H