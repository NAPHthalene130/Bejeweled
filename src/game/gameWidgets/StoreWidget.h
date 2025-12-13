#ifndef STORE_WIDGET_H
#define STORE_WIDGET_H
#include <QWidget>

class GameWindow;

class StoreWidget : public QWidget {
    Q_OBJECT
public:
    explicit StoreWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
private:
    GameWindow* gameWindow = nullptr;
};
#endif // STORE_WIDGET_H
