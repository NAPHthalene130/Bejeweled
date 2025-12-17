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
    achievementsWidget = new AchievementsWidget(this, this);
    menuWidget = new MenuWidget(this, this);
    connect(menuWidget, &MenuWidget::openAchievements, this, [this]() { this->switchWidget(achievementsWidget); });
    connect(achievementsWidget, &AchievementsWidget::backToMenu, this, [this]() { this->switchWidget(menuWidget); });
    playMenuWidget = new PlayMenuWidget(this, this);
    settingWidget = new SettingWidget(this, this);
    storeWidget = new StoreWidget(this, this);
    rankListWidget = new RankListWidget(this, this);
    singleModeGameWidget = new SingleModeGameWidget(this, this);

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

    // GameWindow.cpp 构造函数中
    connect(settingWidget, &SettingWidget::backgroundImageChanged, [this](const QString& imagePath) {
        menuWidget->setBackgroundImage(QPixmap(imagePath));
    });
     // 初始化菜单背景图（从设置中读取）
    QString initBgPath = SettingWidget::getMenuBackgroundImage();
    menuWidget->setBackgroundImage(QPixmap(initBgPath));
    
    connect(menuWidget, &MenuWidget::openSettings, this, [this]() {
    switchWidget(settingWidget); // 点击设置时切换到设置界面
    });
    
    connect(playMenuWidget, &PlayMenuWidget::startRotateMode, [this]() {
        singleModeGameWidget->reset(2); // 旋风模式
        switchWidget(singleModeGameWidget);
    });

    // Add test achievements
    for (int i = 1; i <= 15; ++i) {
        AchievementData ad;
        ad.setTitle(QString("TITLE%1").arg(i));
        ad.setDescription(QString("CONTENTS%1").arg(i));
        ad.setUnlocked(i % 2 == 0);
        if (ad.isUnlocked()) {
            ad.setCompletedAt(QDateTime::currentDateTime());
        }
        addAchievement(ad);
    }
    if (achievementsWidget) achievementsWidget->updateView();

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
