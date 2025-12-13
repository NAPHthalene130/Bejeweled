#include "RegisterWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QSizePolicy>

RegisterWidget::RegisterWidget(QWidget* parent) : QWidget(parent) {
    // 背景占位（预留图片位置）
    setStyleSheet("background-color: #706bceff;");

    // 标题
    QLabel* titleLabel = new QLabel("用户注册", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    // 输入框
    idEdit = new AuthLineEdit("请设置账号", this);
    passwordEdit = new AuthLineEdit("请设置密码", this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    confirmPwdEdit = new AuthLineEdit("请确认密码", this);
    confirmPwdEdit->setEchoMode(QLineEdit::Password);
    emailEdit = new AuthLineEdit("请输入邮箱", this);
    
    // 邮箱验证码布局（输入框+获取按钮）
    QHBoxLayout* emailCodeLayout = new QHBoxLayout();
    emailCodeEdit = new AuthLineEdit("请输入邮箱验证码", this);
    getCodeBtn = new AuthButton("获取验证码", this);
    getCodeBtn->setMinimumHeight(40);
    getCodeBtn->setStyleSheet("background-color: #f4b400; color: white; border-radius: 4px;");
    emailCodeLayout->addWidget(emailCodeEdit);
    emailCodeLayout->addSpacing(10);
    emailCodeLayout->addWidget(getCodeBtn);

    // 按钮
    registerBtn = new AuthButton("注册", this);
    toLoginBtn = new AuthButton("已有账号？去登录", this);
    toLoginBtn->setStyleSheet("background-color: #66c2a5; color: white; border-radius: 4px;");

    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addSpacing(80);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(40);
    mainLayout->addWidget(idEdit);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(passwordEdit);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(confirmPwdEdit);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(emailEdit);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(emailCodeLayout);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(registerBtn);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(toLoginBtn);
    
    // 底部空白占位
    mainLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    mainLayout->setContentsMargins(400, 0, 400, 50); // 左右边距留白

    // 信号连接
    connect(toLoginBtn, &QPushButton::clicked, this, &RegisterWidget::switchToLogin);
    connect(getCodeBtn, &QPushButton::clicked, this, [=]() {
        emit requestEmailCode(emailEdit->text());
    });
    connect(registerBtn, &QPushButton::clicked, this, [=]() {
        emit registerClicked(idEdit->text(), passwordEdit->text(), 
                            confirmPwdEdit->text(), emailEdit->text(), 
                            emailCodeEdit->text());
    });
}