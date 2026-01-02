#include "GameWindow.h"
#include "gameWidgets/AchievementsWidget.h"
#include "gameWidgets/MenuWidget.h"
#include "gameWidgets/PlayMenuWidget.h"
#include "gameWidgets/SettingWidget.h"
#include "gameWidgets/StoreWidget.h"
#include "gameWidgets/RankListWidget.h"
#include "gameWidgets/SingleModeGameWidget.h"
#include "gameWidgets/WhirlwindModeGameWidget.h"
#include "gameWidgets/MultiplayerModeGameWidget.h"
#include "gameWidgets/PuzzleModeGameWidget.h"
#include "gameWidgets/FinalWidget.h"
#include "gameWidgets/MultiGameWaitWidget.h"
#include "gameWidgets/AboutWidget.h"
#include "components/MenuButton.h"
#include "data/CoinSystem.h"
#include "data/ItemSystem.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QString>
#include <QDateTime>
#include <string>
#include "../utils/BGMManager.h"
#include "../utils/ResourceUtils.h"
#include "../utils/LogWindow.h"
#include "data/OtherNetDataIO.h"
GameWindow::GameWindow(QWidget* parent, std::string userID) : QMainWindow(parent) {
    this->userID = userID;

    // 初始化网络IO（需要在CoinSystem和ItemSystem之前初始化）
    otherNetDataIO = std::make_unique<OtherNetDataIO>(this);

    // 初始化金币系统
    CoinSystem::instance().initialize(userID);

    // 设置网络IO并从服务器加载金币（仅在非离线模式下）
    if (userID != "$#SINGLE#$") {
        CoinSystem::instance().setNetworkIO(otherNetDataIO.get());

        // 从服务器加载金币数量
        int coins = otherNetDataIO->getMoney(userID);
        if (coins >= 0) {
            // 设置金币数量到CoinSystem (不自动保存，避免重复写入)
            CoinSystem::instance().setCoins(coins, false);
            qDebug() << "[GameWindow] Loaded coins from server:" << coins;
        } else {
            qDebug() << "[GameWindow] Failed to load coins from server, using local data";
        }

        qDebug() << "[GameWindow] CoinSystem network sync enabled";
    } else {
        qDebug() << "[GameWindow] CoinSystem running in offline mode";
    }
    qDebug() << "[GameWindow] CoinSystem initialized for user:" << QString::fromStdString(userID);

    // 初始化道具系统
    ItemSystem::instance().initialize(userID);

    // 设置网络IO并从服务器加载道具数量（仅在非离线模式下）
    if (userID != "$#SINGLE#$") {
        ItemSystem::instance().setNetworkIO(otherNetDataIO.get());

        // 从服务器加载道具数量
        std::vector<int> props = otherNetDataIO->getPropNums(userID);
        if (props.size() == 4) {
            // 设置道具数量到ItemSystem
            ItemSystem::instance().setItemCounts(props);
            qDebug() << "[GameWindow] Loaded props from server:"
                     << props[0] << props[1] << props[2] << props[3];
        } else {
            qDebug() << "[GameWindow] Failed to load props from server, using local data";
        }

        qDebug() << "[GameWindow] ItemSystem network sync enabled";
    } else {
        qDebug() << "[GameWindow] ItemSystem running in offline mode";
    }
    qDebug() << "[GameWindow] ItemSystem initialized for user:" << QString::fromStdString(userID);

    logWindow = new LogWindow();
    // logWindow->show();

    achievementsWidget = new AchievementsWidget(this, this);
    menuWidget = new MenuWidget(this, this);
    connect(menuWidget, &MenuWidget::openAchievements, this, [this]() { this->switchWidget(achievementsWidget); });
    connect(achievementsWidget, &AchievementsWidget::backToMenu, this, [this]() { this->switchWidget(menuWidget); });
    playMenuWidget = new PlayMenuWidget(this, this);
    settingWidget = new SettingWidget(this, this);
    storeWidget = new StoreWidget(this, this);
    rankListWidget = new RankListWidget(this, this);
    singleModeGameWidget = new SingleModeGameWidget(this, this);
    whirlwindModeGameWidget = new WhirlwindModeGameWidget(this, this);
    multiplayerModeGameWidget = new MultiplayerModeGameWidget(this, this, userID);
    puzzleModeGameWidget = new PuzzleModeGameWidget(this, this);
    finalWidget = new FinalWidget(this, this);
    multiGameWaitWidget = new MultiGameWaitWidget(this, this);
    aboutWidget = new AboutWidget(this, this);

    // 连接排行榜信号
    connect(menuWidget, &MenuWidget::openLeaderboard, this, [this]() { this->switchWidget(rankListWidget); });
    connect(rankListWidget, &RankListWidget::backToMenu, this, [this]() { this->switchWidget(menuWidget); });
    connect(aboutWidget, &AboutWidget::backToMenu, this, [this]() { this->switchWidget(menuWidget); });

    achievementsWidget->hide();
    playMenuWidget->hide();
    settingWidget->hide();
    storeWidget->hide();
    rankListWidget->hide();
    singleModeGameWidget->hide();
    whirlwindModeGameWidget->hide();
    multiplayerModeGameWidget->hide();
    puzzleModeGameWidget->hide();
    finalWidget->hide();
    multiGameWaitWidget->hide();
    aboutWidget->hide();

    connect(menuWidget, &MenuWidget::startGame, [this]() {
        switchWidget(playMenuWidget);
    });

    connect(playMenuWidget, &PlayMenuWidget::backToMenu, [this]() {
        switchWidget(menuWidget);
    });

    // 连接 PlayMenuWidget 信号到 SingleModeGameWidget
    connect(playMenuWidget, &PlayMenuWidget::startNormalMode, [this]() {
        singleModeGameWidget->reset(1);
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

    connect(menuWidget, &MenuWidget::openAbout, this, [this]() {
        switchWidget(aboutWidget);
    });
    connect(menuWidget, &MenuWidget::openStore, this, [this]() {
        switchWidget(storeWidget); // 点击商店时切换到商店界面
    });
    
    connect(playMenuWidget, &PlayMenuWidget::startRotateMode, [this]() {
        whirlwindModeGameWidget->reset(2);
        switchWidget(whirlwindModeGameWidget);
    });

    connect(playMenuWidget, &PlayMenuWidget::startPuzzleMode, [this]() {
        puzzleModeGameWidget->reset(1);
        switchWidget(puzzleModeGameWidget);
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
}

GameWindow::~GameWindow() {
    if (logWindow) {
        delete logWindow;
        logWindow = nullptr;
    }
    if (otherNetDataIO) {
        otherNetDataIO.reset();
    }
}

std::string GameWindow::getUserID() {
    return userID;
}
void GameWindow::setUserID(std::string userID) {
    this->userID = userID;
}

std::string GameWindow::getIp() const {
    return ip;
}

void GameWindow::setIp(const std::string& ip) {
    this->ip = ip;
}

std::string GameWindow::getPort() const {
    return port;
}

void GameWindow::setPort(const std::string& port) {
    this->port = port;
}

void GameWindow::switchWidget(QWidget* widget)
{
    BGMManager::instance().stop();
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
    QString bgmPath;
    if (widget == menuWidget) {
        bgmPath = QString::fromStdString(ResourceUtils::getPath("sounds/menu_bgm.mp3"));
    } else if (widget == achievementsWidget) {
        bgmPath = QString::fromStdString(ResourceUtils::getPath("sounds/achievements_bgm.ogg"));
    } else if (widget == playMenuWidget) {
        bgmPath = QString::fromStdString(ResourceUtils::getPath("sounds/playmenu_bgm.mp3"));
    } else if (widget == settingWidget) {
        // 设置界面可以播放单独的背景音乐或暂停
        bgmPath = QString::fromStdString(ResourceUtils::getPath("sounds/setting_bgm.mp3"));
    } else if (widget == storeWidget) {
        bgmPath = QString::fromStdString(ResourceUtils::getPath("sounds/store_bgm.ogg"));
    } else if (widget == rankListWidget) {
        bgmPath = QString::fromStdString(ResourceUtils::getPath("sounds/rank_bgm.ogg"));
    } else if (widget == singleModeGameWidget || widget == whirlwindModeGameWidget || widget == multiplayerModeGameWidget) {
        bgmPath = QString::fromStdString(ResourceUtils::getPath("sounds/game_bgm.mp3"));
    }
    
    if (!bgmPath.isEmpty()) {
        BGMManager::instance().play(bgmPath);
    }
    currentWidget = widget;

}
