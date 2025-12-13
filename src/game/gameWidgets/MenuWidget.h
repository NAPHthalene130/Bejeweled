#ifndef MENU_WIDGET_H
#define MENU_WIDGET_H
#include <QWidget>

class GameWindow;

class MenuWidget : public QWidget {
    Q_OBJECT
public:
    explicit MenuWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
private:
    GameWindow* gameWindow = nullptr;
};
#endif // MENU_WIDGET_H
