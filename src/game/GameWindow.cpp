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
#include <QString>
#include <QDateTime>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <string>
GameWindow::GameWindow(QWidget* parent, std::string userID) : QMainWindow(parent) {
    this->userID = userID;
    
    // 初始化网络管理器
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &GameWindow::onAchievementsReplyFinished);
    
    achievementsWidget = new AchievementsWidget(this, this);
    menuWidget = new MenuWidget(this, this);
    // 点击成就按钮时，先从云端获取数据，再切换界面
    connect(menuWidget, &MenuWidget::openAchievements, this, [this]() { 
        this->fetchAchievementsFromCloud();
    });
    // 成就数据加载完成后，切换到成就界面
    connect(this, &GameWindow::achievementsLoaded, this, [this]() {
        this->switchWidget(achievementsWidget);
    });
    connect(achievementsWidget, &AchievementsWidget::backToMenu, this, [this]() { this->switchWidget(menuWidget); });
    playMenuWidget = new PlayMenuWidget(this, this);
    settingWidget = new SettingWidget(this, this);
    storeWidget = new StoreWidget(this, this);
    rankListWidget = new RankListWidget(this, this);

    switchWidget(menuWidget);
    
    resize(1600, 1000);
    setWindowTitle("宝石迷阵");
    
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
        // 不要让 setCentralWidget 删除之前的 widget
        if (currentWidget->parent() == this) {
            currentWidget->setParent(nullptr);
        }
    }
    setCentralWidget(widget);
    widget->show();
    currentWidget = widget;

    // If showing achievements, refresh the list
    if (widget == achievementsWidget && achievementsWidget) {
        achievementsWidget->updateView();
    }
}

void GameWindow::fetchAchievementsFromCloud() {
    // 清空当前成就数组
    clearAchievements();
    
    // 构建请求 URL（根据用户ID获取成就）
    // TODO: 替换为实际的服务器地址
    QString serverUrl = QString("http://your-server.com/api/achievements?userId=%1")
                            .arg(QString::fromStdString(userID));
    
    QUrl url(serverUrl);
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    qDebug() << "Fetching achievements from:" << serverUrl;
    
    // 发起 GET 请求
    networkManager->get(request);
}

void GameWindow::onAchievementsReplyFinished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error:" << reply->errorString();
        // 网络错误时，使用默认/本地成就数据
        loadDefaultAchievements();
        emit achievementsLoaded();
        reply->deleteLater();
        return;
    }
    
    QByteArray responseData = reply->readAll();
    qDebug() << "Received achievements data:" << responseData;
    
    // 解析 JSON 响应
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    if (doc.isNull() || !doc.isArray()) {
        qDebug() << "Invalid JSON response, using default achievements";
        loadDefaultAchievements();
        emit achievementsLoaded();
        reply->deleteLater();
        return;
    }
    
    QJsonArray achievementsArray = doc.array();
    
    // 遍历 JSON 数组，填充成就数据
    for (const QJsonValue& val : achievementsArray) {
        if (!val.isObject()) continue;
        
        QJsonObject obj = val.toObject();
        QString title = obj["title"].toString();
        QString description = obj["description"].toString();
        bool unlocked = obj["unlocked"].toBool(false);
        QString difficultyStr = obj["difficulty"].toString("Easy");
        QString completedAtStr = obj["completedAt"].toString();
        
        AchievementData ad(title, description, unlocked);
        
        // 设置难度
        if (difficultyStr == "Easy") {
            ad.setDifficulty(AchievementData::Difficulty::Easy);
        } else if (difficultyStr == "Medium") {
            ad.setDifficulty(AchievementData::Difficulty::Medium);
        } else if (difficultyStr == "Hard") {
            ad.setDifficulty(AchievementData::Difficulty::Hard);
        } else if (difficultyStr == "Ultimate") {
            ad.setDifficulty(AchievementData::Difficulty::Ultimate);
        }
        
        // 设置完成时间
        if (!completedAtStr.isEmpty()) {
            ad.setCompletedAt(QDateTime::fromString(completedAtStr, Qt::ISODate));
        }
        
        addAchievement(ad);
    }
    
    qDebug() << "Loaded" << achievementsContainer.size() << "achievements from cloud";
    
    emit achievementsLoaded();
    reply->deleteLater();
}

void GameWindow::loadDefaultAchievements() {
    // 清空已有数据
    clearAchievements();
    
    // 定义 60 个默认成就
    const std::vector<std::pair<QString, QString>> infos = {
        {"初学者", "完成第一局游戏"},
        {"熟练玩家", "累计玩 10 局游戏"},
        {"连胜达人 I", "连续胜利 3 次"},
        {"连胜达人 II", "连续胜利 5 次"},
        {"高速通关", "在 90 秒内完成一局"},
        {"极速通关", "在 60 秒内完成一局"},
        {"完美无缺", "一局中没有一次失误"},
        {"爆发得分", "单局获得 1000 分"},
        {"高手在此", "单局获得 5000 分"},
        {"收藏家 I", "收集 10 个特殊宝石"},
        {"收藏家 II", "收集 50 个特殊宝石"},
        {"收藏家 III", "收集 100 个特殊宝石"},
        {"道具使用者", "使用任意道具 10 次"},
        {"策略大师", "在一局中使用 3 个不同道具并获胜"},
        {"连击新星", "在一局中连续消除 5 次"},
        {"连击达人", "在一局中连续消除 10 次"},
        {"超级连击", "在一局中连续消除 20 次"},
        {"百万富翁", "累积获得 10000 分"},
        {"富可敌国", "累积获得 50000 分"},
        {"社交达人", "邀请 5 个好友加入游戏"},
        {"新人引导", "完成新手教程"},
        {"每日常客 I", "连续登录 3 天"},
        {"每日常客 II", "连续登录 7 天"},
        {"铁人三项", "完成 3 个不同挑战"},
        {"挑战王", "完成每日挑战 10 次"},
        {"探险家", "发现隐藏关卡"},
        {"完赛者", "完成 50 局游戏"},
        {"百战归来", "完成 100 局游戏"},
        {"逆转大师", "在落后时逆转获胜"},
        {"守护者", "在合作模式中救援队友 3 次"},
        {"团队核心", "在多人模式中获得 MVP"},
        {"成就猎人 I", "解锁 5 个成就"},
        {"成就猎人 II", "解锁 15 个成就"},
        {"成就猎人 III", "解锁 30 个成就"},
        {"探索之光", "解锁一个隐藏成就"},
        {"收藏大全", "收集全部基础宝石"},
        {"兑换达人", "在商店中消费 1000 游戏币"},
        {"幸运星", "首次获得稀有掉落"},
        {"节日庆典", "在活动期间完成专属任务"},
        {"时间管理", "累计在线 10 小时"},
        {"任务完成者", "完成 200 个小任务"},
        {"新手保鲜", "在 5 分钟内完成一局"},
        {"大师之路 I", "达成 2000 分里程碑"},
        {"大师之路 II", "达成 5000 分里程碑"},
        {"冠军", "登上排行榜前 10 名"},
        {"无畏挑战", "在困难模式中获胜"},
        {"终极收藏", "解锁所有普通成就"},
        {"荣耀殿堂", "解锁 40 个成就"},
        {"传说", "解锁全部成就"},
        {"极限操作", "连续完成 3 次极限消除"},
        {"幸运翻倍", "获得双倍奖励 5 次"},
        {"好友助力", "与好友组队获胜 10 次"},
        {"时光旅行者", "参与所有限时活动"},
        {"收藏终极者", "收集所有稀有宝石"},
        {"挑战极限", "在极限模式下获胜"},
        {"全能王者", "所有模式均获胜一次"},
        {"社交之星", "好友数量达到 20"},
        {"活动达人", "参与 10 次官方活动"},
        {"彩虹收集者", "集齐所有彩虹宝石"},
        {"超越自我", "打破个人最高分纪录"}
    };

    for (size_t i = 0; i < infos.size(); ++i) {
        QString t = infos[i].first;
        QString d = infos[i].second;
        // 默认只有第一个成就解锁
        bool unlocked = (i == 0);
        AchievementData ad(t, d, unlocked);
        if (unlocked) ad.setCompletedAt(QDateTime::currentDateTime());
        // 难度分布: 1-25 简单, 26-40 中等, 41-49 困难, 50+ 终极
        int idx = (int)i + 1;
        if (idx <= 25) ad.setDifficulty(AchievementData::Difficulty::Easy);
        else if (idx <= 40) ad.setDifficulty(AchievementData::Difficulty::Medium);
        else if (idx <= 49) ad.setDifficulty(AchievementData::Difficulty::Hard);
        else ad.setDifficulty(AchievementData::Difficulty::Ultimate);
        addAchievement(ad);
    }
    
    qDebug() << "Loaded" << achievementsContainer.size() << "default achievements";
}