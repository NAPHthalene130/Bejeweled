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
#include "data/AchievementSystem.h"
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

    // 初始化金币系统
    CoinSystem::instance().initialize(userID);
    qDebug() << "[GameWindow] CoinSystem initialized for user:" << QString::fromStdString(userID);

    // 初始化道具系统
    ItemSystem::instance().initialize(userID);
    qDebug() << "[GameWindow] ItemSystem initialized for user:" << QString::fromStdString(userID);

    otherNetDataIO = std::make_unique<OtherNetDataIO>(this);
    // logWindow = new LogWindow();
    // logWindow->show();
        // ===== 初始化成就系统 =====
    AchievementSystem::instance().initialize(this, userID);
    qDebug() << "[GameWindow] AchievementSystem initialized";


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

        // ===== 连接成就解锁信号（可选，用于显示通知）=====
    connect(&AchievementSystem::instance(), &AchievementSystem::achievementUnlocked,
            this, [](int index, const QString& title) {
        qDebug() << "Achievement unlocked:" << index << title;
        // 可在此显示解锁通知弹窗
    });

        // ============================================================
    // 在 GameWindow.cpp 构造函数中
    // 找到 "Add test achievements" 那段代码（大约第105-117行）
    // 完全删除并替换成下面的代码：
    // ============================================================

    // 添加真实成就（按顺序 0-9）

    // [0] 初试锋芒 - Easy
    // [0] 初试锋芒
    AchievementData achievement0("初试锋芒", 
        "成功完成首次宝石消除，踏上璀璨之旅的第一步。", false);
    achievement0.setDifficulty(AchievementData::Difficulty::Easy);
    addAchievement(achievement0);

    // [1] 奇异宝石创造者
    AchievementData achievement1("奇异宝石创造者", 
        "首次合成特殊宝石，开启高阶消除策略之门。", false);
    achievement1.setDifficulty(AchievementData::Difficulty::Easy);
    addAchievement(achievement1);

    // [2] 连击艺术
    AchievementData achievement2("连击艺术", 
        "在单局中累计实现 1 次三连消，展现精准操控与节奏感。", false);
    achievement2.setDifficulty(AchievementData::Difficulty::Medium);
    addAchievement(achievement2);

    // [3] 金币盈囊
    AchievementData achievement3("金币盈囊", 
        "累计通过消除金币宝石收获 100 金币，连小卖部老板都认可你的商业眼光。", false);
    achievement3.setDifficulty(AchievementData::Difficulty::Medium);
    addAchievement(achievement3);

    // [4] 四消突破
    AchievementData achievement4("四消突破", 
        "首次达成四连消，解锁更高效的消除技巧。", false);
    achievement4.setDifficulty(AchievementData::Difficulty::Medium);
    addAchievement(achievement4);

    // [5] 极速掌控
    AchievementData achievement5("极速掌控", 
        "在单人模式下 5 分钟内完成一局，速度与效率并存。", false);
    achievement5.setDifficulty(AchievementData::Difficulty::Medium);
    addAchievement(achievement5);

    // [6] 随机谜题征服者
    AchievementData achievement6("随机谜题征服者", 
        "在随机生成的解谜棋盘上首次通关，实力与运气皆不凡。", false);
    achievement6.setDifficulty(AchievementData::Difficulty::Medium);
    addAchievement(achievement6);

    // [7] 旋风试炼
    AchievementData achievement7("旋风试炼", 
        "成功在旋风模式坚持2分钟，挑战手速与思维的双重巅峰。", false);
    achievement7.setDifficulty(AchievementData::Difficulty::Hard);
    addAchievement(achievement7);

    // [8] 破除界限
    AchievementData achievement8("破除界限", 
        "首次达成六连消，突破常规消除的极限，操作令人叹服。", false);
    achievement8.setDifficulty(AchievementData::Difficulty::Hard);
    addAchievement(achievement8);

    // [9] 成就收藏家
    AchievementData achievement9("成就收藏家", 
        "解锁全部成就，见证从新手到大师的完整旅程，你的名字将镌刻在宝石传奇的编年史中。", false);
    achievement9.setDifficulty(AchievementData::Difficulty::Ultimate);
    addAchievement(achievement9);

    // 刷新成就界面
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
        QWidget* oldWidget = takeCentralWidget();
        if (oldWidget) {
            oldWidget->setParent(this);
            oldWidget->hide();
        }
    }

    // currentWidget 只是用于跟踪当前显示的部件，不需要额外的 setParent(nullptr) 操作
    // 因为 takeCentralWidget 已经处理了 ownership
    if (currentWidget && currentWidget != widget) {
        currentWidget->hide();
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
        bgmPath = QString::fromStdString(ResourceUtils::getPath("sounds/store_bgm.mp3"));
    } else if (widget == rankListWidget) {
        bgmPath = QString::fromStdString(ResourceUtils::getPath("sounds/rank_bgm.mp3"));
    } else if (widget == singleModeGameWidget || widget == whirlwindModeGameWidget || widget == multiplayerModeGameWidget) {
        bgmPath = QString::fromStdString(ResourceUtils::getPath("sounds/game_bgm.mp3"));
    }
    
    if (!bgmPath.isEmpty()) {
        BGMManager::instance().play(bgmPath);
    }
    currentWidget = widget;

}
