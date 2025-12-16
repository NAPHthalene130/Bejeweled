#include "LoginWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

LoginWidget::LoginWidget(QWidget* parent) : QWidget(parent) {
    // 现代渐变背景
    setStyleSheet(R"(
        LoginWidget {
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

    // 顶部图标/Logo区域
    QLabel* logoLabel = new QLabel("🔐", cardWidget);
    QFont logoFont = logoLabel->font();
    logoFont.setPointSize(32);
    logoLabel->setFont(logoFont);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setStyleSheet("background: transparent;");

    // 标题
    QLabel* titleLabel = new QLabel("欢迎回来", cardWidget);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #1a202c; background: transparent;");

    // 副标题
    QLabel* subtitleLabel = new QLabel("登录您的账户继续使用", cardWidget);
    QFont subtitleFont = subtitleLabel->font();
    subtitleFont.setPointSize(11);
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("color: #718096; background: transparent;");

    // 标签字体
    QFont labelFont;
    labelFont.setPointSize(12);
    labelFont.setBold(true);

    // === 账号输入区域 ===
    QLabel* idLabel = new QLabel("👤 账号", cardWidget);
    idLabel->setFont(labelFont);
    idLabel->setStyleSheet("color: #2d3748; background: transparent; padding: 3px 5px;");

    // 提示在输入框上方
    idHintLabel = new QLabel("💡 输入您注册时使用的账号", cardWidget);
    idHintLabel->setStyleSheet(R"(
        color: #a0aec0; 
        font-size: 10px; 
        background: transparent; 
        padding: 2px 5px;
        font-style: italic;
    )");

    idEdit = new AuthLineEdit("请输入您的账号", cardWidget);
    idEdit->setMinimumHeight(48);

    // === 密码输入区域 ===
    QLabel* passwordLabel = new QLabel("🔒 密码", cardWidget);
    passwordLabel->setFont(labelFont);
    passwordLabel->setStyleSheet("color: #2d3748; background: transparent; padding: 3px 5px;");

    // 提示在输入框上方
    QLabel* passwordHintLabel = new QLabel("💡 点击右侧图标可显示/隐藏密码", cardWidget);
    passwordHintLabel->setStyleSheet(R"(
        color: #a0aec0; 
        font-size: 10px; 
        background: transparent; 
        padding: 2px 5px;
        font-style: italic;
    )");

    passwordEdit = new AuthLineEdit("请输入您的密码", cardWidget);
    passwordEdit->setPasswordMode(true);
    passwordEdit->setMinimumHeight(48);

    // 分隔线
    QFrame* separator = new QFrame(cardWidget);
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background-color: #e2e8f0; margin: 5px 0;");

    // === 按钮区域 ===
    // 登录按钮
    loginBtn = new AuthButton("🚀 立即登录", cardWidget);
    loginBtn->setMinimumHeight(48);
    loginBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #667eea, stop:1 #764ba2);
            color: white;
            border: none;
            border-radius: 10px;
            padding: 12px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #5568d3, stop:1 #6b3fa0);
            transform: translateY(-2px);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #4a5bc4, stop:1 #5d3691);
        }
    )");

    // 离线登录标签说明
    oflHintLabel = new QLabel("────────  或  ────────", cardWidget);
    oflHintLabel->setAlignment(Qt::AlignCenter);
    oflHintLabel->setStyleSheet(R"(
        color: #cbd5e0; 
        font-size: 11px; 
        background: transparent; 
        padding: 6px 0;
    )");

    // 离线登录按钮
    oflLoginBtn = new AuthButton("🎮 离线登录（游客模式）", cardWidget);
    oflLoginBtn->setMinimumHeight(45);
    oflLoginBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #f7fafc;
            color: #4a5568;
            border: 2px solid #e2e8f0;
            border-radius: 10px;
            padding: 10px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #edf2f7;
            border-color: #cbd5e0;
        }
        QPushButton:pressed {
            background-color: #e2e8f0;
        }
    )");

    // 离线登录说明
    QLabel* oflDescLabel = new QLabel("⚠️ 游客模式下进度将不会保存", cardWidget);
    oflDescLabel->setAlignment(Qt::AlignCenter);
    oflDescLabel->setStyleSheet(R"(
        color: #f59e0b; 
        font-size: 10px; 
        background: transparent; 
        padding: 2px;
    )");

    // 注册按钮
    toRegisterBtn = new AuthButton("还没有账号？点击注册 →", cardWidget);
    toRegisterBtn->setMinimumHeight(45);
    toRegisterBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #667eea;
            border: 2px solid #667eea;
            border-radius: 10px;
            padding: 10px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgba(102, 126, 234, 0.08);
            border-color: #5568d3;
            color: #5568d3;
        }
        QPushButton:pressed {
            background-color: rgba(102, 126, 234, 0.15);
        }
    )");

    // 卡片内布局 - 优化间距，提示在输入框上方
    QVBoxLayout* cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->addSpacing(20);  // 从30减小到20
    cardLayout->addWidget(logoLabel);
    cardLayout->addSpacing(10);  // 从12减小到10
    cardLayout->addWidget(titleLabel);
    cardLayout->addSpacing(3);   // 从5减小到3
    cardLayout->addWidget(subtitleLabel);
    cardLayout->addSpacing(20);  // 从25减小到20
    
    // 账号区域（提示在输入框上方）
    cardLayout->addWidget(idLabel);
    cardLayout->addSpacing(3);
    cardLayout->addWidget(idHintLabel);  // 提示在上
    cardLayout->addSpacing(5);
    cardLayout->addWidget(idEdit);       // 输入框在下
    cardLayout->addSpacing(12);  // 从15减小到12
    
    // 密码区域（提示在输入框上方）
    cardLayout->addWidget(passwordLabel);
    cardLayout->addSpacing(3);
    cardLayout->addWidget(passwordHintLabel);  // 提示在上
    cardLayout->addSpacing(5);
    cardLayout->addWidget(passwordEdit);       // 输入框在下
    cardLayout->addSpacing(15);  // 从20减小到15
    
    // 登录按钮
    cardLayout->addWidget(loginBtn);
    cardLayout->addSpacing(10);  // 从12减小到10
    
    // 分隔线
    cardLayout->addWidget(separator);
    cardLayout->addWidget(oflHintLabel);
    cardLayout->addSpacing(6);   // 从8减小到6
    
    // 离线登录
    cardLayout->addWidget(oflLoginBtn);
    cardLayout->addSpacing(3);
    cardLayout->addWidget(oflDescLabel);
    cardLayout->addSpacing(12);  // 从15减小到12
    
    // 注册按钮
    cardLayout->addWidget(toRegisterBtn);
    cardLayout->addSpacing(20);  // 从30减小到20
    
    cardLayout->setContentsMargins(40, 0, 40, 0);

    // 主布局 - 减小上边距，让内容上移
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch();
    mainLayout->addWidget(cardWidget);
    mainLayout->addStretch();
    mainLayout->setContentsMargins(200, 30, 200, 30);  // 从50减小到30

    // 信号连接
    connect(toRegisterBtn, &QPushButton::clicked, this, &LoginWidget::switchToRegister);
    connect(loginBtn, &QPushButton::clicked, this, [=]() {
        emit loginClicked(idEdit->text(), passwordEdit->text());
    });
    connect(oflLoginBtn, &QPushButton::clicked, this, [=]() {
        emit oflLoginClicked();
    });
}

