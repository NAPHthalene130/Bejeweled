#include "AuthButton.h"
#include <QFont>

AuthButton::AuthButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent) {
    // 设置统一样式（可根据需求调整）
    setMinimumHeight(50);
    QFont font = this->font();
    font.setPointSize(14);
    font.setBold(true);
    setFont(font);
    setCursor(Qt::PointingHandCursor);

    // 默认按钮样式（可被外部覆盖）
    setStyleSheet(R"(
        QPushButton {
            background-color: #4a90e2;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 24px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #357abd;
        }
        QPushButton:pressed {
            background-color: #2a5f8f;
        }
        QPushButton:disabled {
            background-color: #cbd5e0;
            color: #a0aec0;
        }
    )");
}