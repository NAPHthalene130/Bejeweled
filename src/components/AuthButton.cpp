#include "AuthButton.h"
#include <QFont>

AuthButton::AuthButton(const QString& text, QWidget* parent) 
    : QPushButton(text, parent) {
    // 设置统一样式（可根据需求调整）
    setMinimumHeight(40);
    QFont font = this->font();
    font.setPointSize(12);
    setFont(font);
    setStyleSheet("background-color: #4a90e2; color: white; border-radius: 4px;");
}