#include "MenuButton.h"
#include <QFont>

MenuButton::MenuButton(int width, int height, int fontSize, const QColor& fontColor, const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    setFixedSize(width, height);

    QFont font = this->font();
    font.setPointSize(fontSize);
    font.setBold(true); // 加粗让文字更显眼
    setFont(font);

    // 构建样式表
    QString colorStr = fontColor.name(QColor::HexArgb);
    int radius = height / 2; // 圆角为高度的一半，呈现胶囊状

    QString style = QString(
        "MenuButton {"
        "   background-color: transparent;"
        "   color: %1;"
        "   border-radius: %2px;"
        "   border: none;"
        "}"
        "MenuButton:hover {"
        "   background-color: rgba(200, 200, 200, 80);" // 浅灰色，半透明
        "}"
        "MenuButton:pressed {"
        "   background-color: rgba(180, 180, 180, 100);" // 按下时稍微深一点
        "}"
    ).arg(colorStr).arg(radius);

    setStyleSheet(style);
}

MenuButton::~MenuButton() {
}
