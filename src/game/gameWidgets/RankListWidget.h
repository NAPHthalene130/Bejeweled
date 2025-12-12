#ifndef RANK_LIST_WIDGET_H
#define RANK_LIST_WIDGET_H
#include <QWidget>

class GameWindow;

class RankListWidget : public QWidget {
    Q_OBJECT
public:
    explicit RankListWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
private:
    GameWindow* gameWindow = nullptr;
};
#endif // RANK_LIST_WIDGET_H

