#ifndef SETTING_WIDGET_H
#define SETTING_WIDGET_H
#include <QWidget>

class GameWindow;

class SettingWidget : public QWidget {
    Q_OBJECT
public:
    explicit SettingWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);    
private:
    GameWindow* gameWindow = nullptr;
};
#endif // SETTING_WIDGET_H
