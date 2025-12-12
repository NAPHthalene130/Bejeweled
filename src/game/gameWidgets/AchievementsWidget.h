#ifndef ACHIEVEMENTS_WIDGET_H
#define ACHIEVEMENTS_WIDGET_H
#include <QWidget>

class GameWindow;

class AchievementsWidget : public QWidget {
    Q_OBJECT
public:
    explicit AchievementsWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
private:
    GameWindow* gameWindow = nullptr;
};
#endif // ACHIEVEMENTS_WIDGET_H

