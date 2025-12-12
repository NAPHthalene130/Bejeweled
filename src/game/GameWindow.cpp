#include "GameWindow.h"
#include "gameWidgets/AchievementsWidget.h"
#include "gameWidgets/MenuWidget.h"
#include "gameWidgets/PlayMenuWidget.h"
#include "gameWidgets/SettingWidget.h"
#include "gameWidgets/StoreWidget.h"
#include "gameWidgets/RankListWidget.h"
#include "components/MenuButton.h"
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
    
    resize(1600, 1000);
    setWindowTitle("宝石迷阵");

    //以下是测试，到时候记得删了
    MenuButton* btn1 = new MenuButton(200, 60, 24, Qt::white, "开始游戏", this);
    btn1->move(100, 100);
    btn1->raise();

    MenuButton* btn2 = new MenuButton(180, 50, 20, QColor("#FFD700"), "排行榜", this);
    btn2->move(100, 180);
    btn2->raise();

    MenuButton* btn3 = new MenuButton(150, 40, 16, QColor(200, 200, 255), "退出", this);
    btn3->move(100, 250);
    btn3->raise();
     //以上是测试，到时候记得删了
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