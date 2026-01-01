#include "RegisterWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QGraphicsDropShadowEffect>

RegisterWidget::RegisterWidget(QWidget* parent) : QWidget(parent) {
    // 现代渐变背景
    setStyleSheet(R"(
        RegisterWidget {
            background: transparent;
        }
    )");

    // 创建中心卡片容器
    QWidget* cardWidget = new QWidget(this);
    cardWidget->setMinimumWidth(600);
    cardWidget->setStyleSheet(R"(
        QWidget {
            background-color: rgba(255, 255, 255, 0.85);
            border-radius: 20px;
        }
    )");

    // 添加阴影效果
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(40);
    shadow->setColor(QColor(0, 0, 0, 100));
    shadow->setOffset(0, 15);
    cardWidget->setGraphicsEffect(shadow);

    // 顶部图标/Logo区域 - 进一步缩小
    QLabel* logoLabel = new QLabel("✨", cardWidget);
    QFont logoFont = logoLabel->font();
    logoFont.setPointSize(32);  // 从40进一步减小到32
    logoLabel->setFont(logoFont);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setStyleSheet("background: transparent;");

    // 标题 - 进一步缩小
    QLabel* titleLabel = new QLabel("创建新账户", cardWidget);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);  // 从24减小到20
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #1a202c; background: transparent;");

    // 副标题 - 进一步缩小
    QLabel* subtitleLabel = new QLabel("填写以下信息完成注册", cardWidget);
    QFont subtitleFont = subtitleLabel->font();
    subtitleFont.setPointSize(11);  // 从12减小到11
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("color: #718096; background: transparent;");

    // 标签字体设置
    QFont labelFont;
    labelFont.setPointSize(12);  // 从13减小到12
    labelFont.setBold(true);

    // === 账号输入区域 ===
    QLabel* idLabel = new QLabel("👤 账号", cardWidget);
    idLabel->setFont(labelFont);
    idLabel->setStyleSheet("color: #2d3748; background: transparent; padding: 3px 5px;");

    // 提示放在输入框上方
    idHintLabel = new QLabel("", cardWidget);
    idHintLabel->setStyleSheet(R"(
        color: #a0aec0; 
        font-size: 10px; 
        background: transparent; 
        padding: 2px 5px;
        font-style: italic;
    )");

    idEdit = new AuthLineEdit("请设置您的账号（6-20位）", cardWidget);
    idEdit->setMinimumHeight(48);  // 从50减小到48

    // === 密码输入区域 ===
    QLabel* passwordLabel = new QLabel("🔒 密码", cardWidget);
    passwordLabel->setFont(labelFont);
    passwordLabel->setStyleSheet("color: #2d3748; background: transparent; padding: 3px 5px;");

    passwordHintLabel = new QLabel("", cardWidget);
    passwordHintLabel->setStyleSheet(R"(
        color: #a0aec0; 
        font-size: 10px; 
        background: transparent; 
        padding: 2px 5px;
        font-style: italic;
    )");

    passwordEdit = new AuthLineEdit("请设置您的密码（8-20位）", cardWidget);
    passwordEdit->setPasswordMode(true);
    passwordEdit->setMinimumHeight(48);

    // === 确认密码区域 ===
    QLabel* confirmPwdLabel = new QLabel("🔒 确认密码", cardWidget);
    confirmPwdLabel->setFont(labelFont);
    confirmPwdLabel->setStyleSheet("color: #2d3748; background: transparent; padding: 3px 5px;");

    QLabel* confirmPwdHintLabel = new QLabel("", cardWidget);
    confirmPwdHintLabel->setStyleSheet(R"(
        color: #a0aec0; 
        font-size: 10px; 
        background: transparent; 
        padding: 2px 5px;
        font-style: italic;
    )");

    confirmPwdEdit = new AuthLineEdit("请再次输入密码进行确认", cardWidget);
    confirmPwdEdit->setPasswordMode(true);
    confirmPwdEdit->setMinimumHeight(48);

    // 分隔线
    QFrame* separator = new QFrame(cardWidget);
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background-color: #e2e8f0; margin: 5px 0;");

    // === 按钮区域 ===
    // 注册按钮
    registerBtn = new AuthButton("🎉 立即注册", cardWidget);
    registerBtn->setMinimumHeight(48);
    registerBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #f093fb, stop:1 #f5576c);
            color: white;
            border: none;
            border-radius: 10px;
            padding: 12px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #e082ea, stop:1 #e4465b);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #d071d9, stop:1 #d3354a);
        }
    )");

    // 返回登录按钮
    toLoginBtn = new AuthButton("已有账号？点击登录 →", cardWidget);
    toLoginBtn->setMinimumHeight(45);
    toLoginBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #f5576c;
            border: 2px solid #f5576c;
            border-radius: 10px;
            padding: 10px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgba(245, 87, 108, 0.08);
            border-color: #e4465b;
            color: #e4465b;
        }
        QPushButton:pressed {
            background-color: rgba(245, 87, 108, 0.15);
        }
    )");

    // 卡片内布局 - 大幅优化间距，提示放在输入框上方
    QVBoxLayout* cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->addSpacing(15);  // 从25大幅减小到15
    cardLayout->addWidget(logoLabel);
    cardLayout->addSpacing(8);   // 从12减小到8
    cardLayout->addWidget(titleLabel);
    cardLayout->addSpacing(3);   // 从5减小到3
    cardLayout->addWidget(subtitleLabel);
    cardLayout->addSpacing(15);  // 从20减小到15
    
    // 账号（提示在输入框上方）
    cardLayout->addWidget(idLabel);
    cardLayout->addSpacing(3);
    cardLayout->addWidget(idHintLabel);      // 提示在上
    cardLayout->addSpacing(5);
    cardLayout->addWidget(idEdit);           // 输入框在下
    cardLayout->addSpacing(10);  // 从12减小到10
    
    // 密码（提示在输入框上方）
    cardLayout->addWidget(passwordLabel);
    cardLayout->addSpacing(3);
    cardLayout->addWidget(passwordHintLabel); // 提示在上
    cardLayout->addSpacing(5);
    cardLayout->addWidget(passwordEdit);      // 输入框在下
    cardLayout->addSpacing(10);
    
    // 确认密码（提示在输入框上方）
    cardLayout->addWidget(confirmPwdLabel);
    cardLayout->addSpacing(3);
    cardLayout->addWidget(confirmPwdHintLabel); // 提示在上
    cardLayout->addSpacing(5);
    cardLayout->addWidget(confirmPwdEdit);      // 输入框在下
    cardLayout->addSpacing(10);
    
    // 分隔线和按钮
    cardLayout->addWidget(separator);
    cardLayout->addSpacing(10);  // 从12减小到10
    cardLayout->addWidget(registerBtn);
    cardLayout->addSpacing(10);  // 从12减小到10
    cardLayout->addWidget(toLoginBtn);
    cardLayout->addSpacing(15);  // 从25减小到15
    
    cardLayout->setContentsMargins(40, 0, 40, 0);

    // 主布局 - 进一步减小上边距，让内容上移
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch();
    mainLayout->addWidget(cardWidget);
    mainLayout->addStretch();
    mainLayout->setContentsMargins(200, 15, 200, 15);  // 从30减小到15

    // 信号连接
    connect(toLoginBtn, &QPushButton::clicked, this, &RegisterWidget::switchToLogin);
    connect(registerBtn, &QPushButton::clicked, this, [=]() {
        emit registerClicked(idEdit->text(), passwordEdit->text(),
                            confirmPwdEdit->text());
    });
}

