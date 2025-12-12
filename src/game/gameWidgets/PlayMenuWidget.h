#ifndef PLAY_MENU_WIDGET_H
#define PLAY_MENU_WIDGET_H
#include <QWidget>

class GameWindow;

class PlayMenuWidget : public QWidget {
    Q_OBJECT
public:
    explicit PlayMenuWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
private:
    GameWindow* gameWindow = nullptr;
};
#endif // PLAY_MENU_WIDGET_H
