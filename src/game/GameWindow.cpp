#include "GameWindow.h"
#include "gameWidgets/AchievementsWidget.h"
#include "gameWidgets/MenuWidget.h"
#include "gameWidgets/PlayMenuWidget.h"
#include "gameWidgets/SettingWidget.h"
#include "gameWidgets/StoreWidget.h"
#include "gameWidgets/RankListWidget.h"
#include "gameWidgets/SingleModeGameWidget.h"
#include "components/MenuButton.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QString>
#include <QDateTime>
#include <string>
GameWindow::GameWindow(QWidget* parent, std::string userID) : QMainWindow(parent) {
    this->userID = userID;
    
    // 初始化成就数据
    {
        AchievementData a1("初入江湖", "完成第一局游戏", true);
        a1.setDifficulty(AchievementData::Difficulty::Easy);
        a1.setCompletedAt(QDateTime::currentDateTime().addDays(-5));
        achievementsContainer.push_back(a1);
        
        AchievementData a2("连击大师", "单局达成5连消", false);
        a2.setDifficulty(AchievementData::Difficulty::Medium);
        achievementsContainer.push_back(a2);
        
        AchievementData a3("宝石猎人", "累计消除1000颗宝石", true);
        a3.setDifficulty(AchievementData::Difficulty::Medium);
        a3.setCompletedAt(QDateTime::currentDateTime().addDays(-2));
        achievementsContainer.push_back(a3);
        
        AchievementData a4("时间管理者", "在限时模式中获得10000分", false);
        a4.setDifficulty(AchievementData::Difficulty::Hard);
        achievementsContainer.push_back(a4);
        
        AchievementData a5("完美主义", "单局不使用提示完成游戏", false);
        a5.setDifficulty(AchievementData::Difficulty::Hard);
        achievementsContainer.push_back(a5);
        
        AchievementData a6("传奇玩家", "累计游戏时长达到100小时", false);
        a6.setDifficulty(AchievementData::Difficulty::Ultimate);
        achievementsContainer.push_back(a6);
    }
    
    achievementsWidget = new AchievementsWidget(this, this);
    menuWidget = new MenuWidget(this, this);
    connect(menuWidget, &MenuWidget::openAchievements, this, [this]() { this->switchWidget(achievementsWidget); });
    connect(achievementsWidget, &AchievementsWidget::backToMenu, this, [this]() { this->switchWidget(menuWidget); });
    playMenuWidget = new PlayMenuWidget(this, this);
    settingWidget = new SettingWidget(this, this);
    storeWidget = new StoreWidget(this, this);
    rankListWidget = new RankListWidget(this, this);
    singleModeGameWidget = new SingleModeGameWidget(this, this);

    // 连接排行榜信号
    connect(menuWidget, &MenuWidget::openLeaderboard, this, [this]() { this->switchWidget(rankListWidget); });
    connect(rankListWidget, &RankListWidget::backToMenu, this, [this]() { this->switchWidget(menuWidget); });

    achievementsWidget->hide();
    playMenuWidget->hide();
    settingWidget->hide();
    storeWidget->hide();
    rankListWidget->hide();
    singleModeGameWidget->hide();

    connect(menuWidget, &MenuWidget::startGame, [this]() {
        switchWidget(playMenuWidget);
    });

    connect(playMenuWidget, &PlayMenuWidget::backToMenu, [this]() {
        switchWidget(menuWidget);
    });

    // 连接 PlayMenuWidget 信号到 SingleModeGameWidget
    connect(playMenuWidget, &PlayMenuWidget::startNormalMode, [this]() {
        singleModeGameWidget->reset(1); // 普通模式
        switchWidget(singleModeGameWidget);
    });

    connect(playMenuWidget, &PlayMenuWidget::startRotateMode, [this]() {
        singleModeGameWidget->reset(2); // 旋风模式
        switchWidget(singleModeGameWidget);
    });

    switchWidget(menuWidget);
    
    resize(1600, 1000);
    setWindowTitle("宝石迷阵");
}

GameWindow::~GameWindow() {
}

std::string GameWindow::getUserID() {
    return userID;
}
void GameWindow::setUserID(std::string userID) {
    this->userID = userID;
}

void GameWindow::switchWidget(QWidget* widget)
{
    // 防止 QMainWindow 删除之前的中央部件
    if (centralWidget()) {
        takeCentralWidget();
    }

    if (currentWidget) {
        currentWidget->hide();
        // 不要让 setCentralWidget 删除之前的 widget
        if (currentWidget->parent() == this) {
            currentWidget->setParent(nullptr);
        }
    }
    setCentralWidget(widget);
    widget->show();
    currentWidget = widget;
}
