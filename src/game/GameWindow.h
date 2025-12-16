#ifndef GAME_WINDOW_H
#define GAME_WINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <string>

class AchievementsWidget;
class MenuWidget;
class PlayMenuWidget;
class SettingWidget;
class StoreWidget;
class RankListWidget;

#include <vector>
#include "data/AchievementData.h"

class GameWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit GameWindow(QWidget* parent = nullptr, std::string userID = "");
    std::string getUserID();
    void setUserID(std::string userID);
    void switchWidget(QWidget* widget);

    // Achievements access
    std::vector<AchievementData>& getAchievements() { return achievementsContainer; }
    void addAchievement(const AchievementData& a) { achievementsContainer.push_back(a); }
    void clearAchievements() { achievementsContainer.clear(); }
    
    // 从云端获取成就数据
    void fetchAchievementsFromCloud();
    
signals:
    void achievementsLoaded(); // 成就数据加载完成信号

private slots:
    void onAchievementsReplyFinished(QNetworkReply* reply);

private:
    void loadDefaultAchievements(); // 加载默认成就（网络失败时使用）
    
    std::string userID;
    QWidget* currentWidget = nullptr;
    QNetworkAccessManager* networkManager = nullptr;

    AchievementsWidget* achievementsWidget = nullptr;
    MenuWidget* menuWidget = nullptr;
    PlayMenuWidget* playMenuWidget = nullptr;
    SettingWidget* settingWidget = nullptr;
    StoreWidget* storeWidget = nullptr;
    RankListWidget* rankListWidget = nullptr;

    std::vector<AchievementData> achievementsContainer;

};

#endif // GAME_WINDOW_H
