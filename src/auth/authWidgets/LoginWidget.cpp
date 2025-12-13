#include "LoginWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QMessageBox>

LoginWidget::LoginWidget(QWidget* parent) : QWidget(parent) {
    // 背景占位（预留图片位置）
    setStyleSheet("background-color: #d14949ff;");

    // 标题
    QLabel* titleLabel = new QLabel("用户登录", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    // 输入框
    idEdit = new AuthLineEdit("请输入账号", this);
    passwordEdit = new AuthLineEdit("请输入密码", this);
    passwordEdit->setEchoMode(QLineEdit::Password); // 密码隐藏

    // 按钮
    loginBtn = new AuthButton("登录", this);
    oflLoginBtn = new AuthButton("离线登录", this);
    toRegisterBtn = new AuthButton("没有账号？去注册", this);
    toRegisterBtn->setStyleSheet("background-color: #66c2a5; color: white; border-radius: 4px;");

    // 布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addSpacing(100);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(60);
    mainLayout->addWidget(idEdit);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(passwordEdit);
    mainLayout->addSpacing(40);
    mainLayout->addWidget(loginBtn);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(oflLoginBtn);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(toRegisterBtn);
    
    // 添加底部空白占位
    mainLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    mainLayout->setContentsMargins(400, 0, 400, 50); 

    // 信号连接
    connect(toRegisterBtn, &QPushButton::clicked, this, &LoginWidget::switchToRegister);
    connect(loginBtn, &QPushButton::clicked, this, [=]() {
        emit loginClicked(idEdit->text(), passwordEdit->text());
    });
    connect(oflLoginBtn, &QPushButton::clicked, this, [=]() {
        emit oflLoginClicked();
    });
}