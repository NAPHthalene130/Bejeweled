#ifndef MENU_BUTTON_H
#define MENU_BUTTON_H
#include <QPushButton>
#include <QColor>

class MenuButton : public QPushButton {
    Q_OBJECT
public:
    MenuButton(int width, int height, int fontSize, const QColor& fontColor, const QString& text = "", QWidget* parent = nullptr);
    ~MenuButton();

};
#endif // MENU_BUTTON_H
