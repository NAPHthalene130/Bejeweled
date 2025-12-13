#include "AuthLineEdit.h"
#include <QFont>

AuthLineEdit::AuthLineEdit(const QString& placeholder, QWidget* parent) 
    : QLineEdit(parent) {
    // 设置统一样式（可根据需求调整）
    setPlaceholderText(placeholder);
    setMinimumHeight(40);
    QFont font = this->font();
    font.setPointSize(12);
    setFont(font);
    setStyleSheet("padding: 0 10px; border: 1px solid #ccc; border-radius: 4px;");
}