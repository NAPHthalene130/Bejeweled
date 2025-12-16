#include "AuthLineEdit.h"
#include <QFont>
#include <QStyle>

AuthLineEdit::AuthLineEdit(const QString& placeholder, QWidget* parent)
    : QLineEdit(parent), toggleButton(nullptr), isPasswordVisible(false), isPasswordMode(false) {
    // 设置placeholder
    setPlaceholderText(placeholder);
    setMinimumHeight(55);
    
    // 设置字体
    QFont font = this->font();
    font.setPointSize(13);
    setFont(font);

    // 增强样式：更美观的边框、清晰的focus状态
    setStyleSheet(R"(
        QLineEdit {
            padding: 0 18px;
            border: 2px solid #e2e8f0;
            border-radius: 12px;
            background-color: #f8fafc;
            color: #1a202c;
            selection-background-color: #667eea;
            selection-color: white;
        }
        QLineEdit:hover {
            border-color: #cbd5e0;
            background-color: #f1f5f9;
        }
        QLineEdit:focus {
            border: 2px solid #667eea;
            background-color: white;
        }
        QLineEdit::placeholder {
            color: #a0aec0;
        }
    )");
}

void AuthLineEdit::setPasswordMode(bool enable) {
    isPasswordMode = enable;
    if (enable) {
        setEchoMode(QLineEdit::Password);
        setupToggleButton();
    } else {
        setEchoMode(QLineEdit::Normal);
        if (toggleButton) {
            toggleButton->hide();
        }
    }
}

void AuthLineEdit::setupToggleButton() {
    if (!toggleButton) {
        toggleButton = new QPushButton(this);
        toggleButton->setFixedSize(35, 35);
        toggleButton->setCursor(Qt::PointingHandCursor);
        toggleButton->setStyleSheet(R"(
            QPushButton {
                border: none;
                background-color: transparent;
                padding: 5px;
                border-radius: 6px;
                font-size: 16px;
            }
            QPushButton:hover {
                background-color: rgba(102, 126, 234, 0.1);
            }
            QPushButton:pressed {
                background-color: rgba(102, 126, 234, 0.2);
            }
        )");
        
        connect(toggleButton, &QPushButton::clicked, this, &AuthLineEdit::togglePasswordVisibility);
        
        // 定位按钮到右侧
        int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        toggleButton->move(width() - toggleButton->width() - frameWidth - 12, 
                          (height() - toggleButton->height()) / 2);
        
        // 调整文本边距，避免文字被按钮遮挡
        setStyleSheet(styleSheet() + QString("\nQLineEdit { padding-right: %1px; }").arg(toggleButton->width() + 24));
    }
    
    updateToggleButtonIcon();
    toggleButton->show();
}

void AuthLineEdit::togglePasswordVisibility() {
    isPasswordVisible = !isPasswordVisible;
    setEchoMode(isPasswordVisible ? QLineEdit::Normal : QLineEdit::Password);
    updateToggleButtonIcon();
}

void AuthLineEdit::updateToggleButtonIcon() {
    if (!toggleButton) return;
    
    // 使用更清晰的emoji图标
    if (isPasswordVisible) {
        toggleButton->setText("👁");  // 显示状态：眼睛睁开
        toggleButton->setToolTip("点击隐藏密码");
    } else {
        toggleButton->setText("🔒");  // 隐藏状态：锁图标
        toggleButton->setToolTip("点击显示密码");
    }
    
    // 设置字体大小
    QFont btnFont = toggleButton->font();
    btnFont.setPointSize(14);
    toggleButton->setFont(btnFont);
}

void AuthLineEdit::resizeEvent(QResizeEvent* event) {
    QLineEdit::resizeEvent(event);
    
    // 更新按钮位置
    if (toggleButton && isPasswordMode) {
        int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        toggleButton->move(width() - toggleButton->width() - frameWidth - 12, 
                          (height() - toggleButton->height()) / 2);
    }
}

