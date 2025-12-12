#include "GameWindow.h"
#include "gameWidgets/AchievementsWidget.h"
#include "gameWidgets/MenuWidget.h"
#include "gameWidgets/PlayMenuWidget.h"
#include "gameWidgets/SettingWidget.h"
#include "gameWidgets/StoreWidget.h"
#include "gameWidgets/RankListWidget.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <string>
GameWindow::GameWindow(QWidget* parent, std::string userID) : QMainWindow(parent) {
    this->userID = userID;
    achievementsWidget = new AchievementsWidget(this, this);
    menuWidget = new MenuWidget(this, this);
    playMenuWidget = new PlayMenuWidget(this, this);
    settingWidget = new SettingWidget(this, this);
    storeWidget = new StoreWidget(this, this);
    rankListWidget = new RankListWidget(this, this);
    switchWidget(menuWidget);
}
std::string GameWindow::getUserID() {
    return userID;
}
void GameWindow::setUserID(std::string userID) {
    this->userID = userID;
}

void GameWindow::switchWidget(QWidget* widget)
{
    if (currentWidget) {
        currentWidget->hide();
    }
    setCentralWidget(widget);
    widget->show();
    currentWidget = widget;
}