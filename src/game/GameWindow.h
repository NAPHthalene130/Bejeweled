#ifndef GAME_WINDOW_H
#define GAME_WINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <string>

class AchievementsWidget;
class MenuWidget;
class PlayMenuWidget;
class SettingWidget;
class StoreWidget;
class RankListWidget;
class SingleModeGameWidget;
class WhirlwindModeGameWidget;
class MultiplayerModeGameWidget;
class PuzzleModeGameWidget;
class FinalWidget;
class MultiGameWaitWidget;
class NetDataIO;
class LogWindow;
class OtherNetDataIO;

#include <vector>
#include "data/AchievementData.h"

class GameWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit GameWindow(QWidget* parent = nullptr, std::string userID = "");
    ~GameWindow(); // Added destructor
    std::string getUserID();
    void setUserID(std::string userID);
    void switchWidget(QWidget* widget);
    
    // Public getter for singleModeGameWidget if needed, or just make it public/accessible via switch
    SingleModeGameWidget* getSingleModeGameWidget() const { return singleModeGameWidget; }
    WhirlwindModeGameWidget* getWhirlwindModeGameWidget() const { return whirlwindModeGameWidget; }
    MultiplayerModeGameWidget* getMultiplayerModeGameWidget() const { return multiplayerModeGameWidget; }
    PuzzleModeGameWidget* getPuzzleModeGameWidget() const { return puzzleModeGameWidget; }
    PlayMenuWidget* getPlayMenuWidget() const { return playMenuWidget; }
    FinalWidget* getFinalWidget() const { return finalWidget; }
    MultiGameWaitWidget* getMultiGameWaitWidget() const { return multiGameWaitWidget; }
    void setMultiGameWaitWidget(MultiGameWaitWidget* widget) { multiGameWaitWidget = widget; }

    // NetDataIO access
    NetDataIO* getNetDataIO() const { return netDataIO; }
    void setNetDataIO(NetDataIO* netDataIO) { this->netDataIO = netDataIO; }

    LogWindow* getLogWindow() const { return logWindow; }

    // Achievements access
    std::vector<AchievementData>& getAchievements() { return achievementsContainer; }
    void addAchievement(const AchievementData& a) { achievementsContainer.push_back(a); }
    MenuWidget* getMenuWidget() const { return menuWidget; }

    std::string getIp() const;
    void setIp(const std::string& ip);

    std::string getPort() const;
    void setPort(const std::string& port);

    std::string getGemstoneStyle() const { return gemstoneStyle; }
    void setGemstoneStyle(const std::string& style) { gemstoneStyle = style; }

    int getDifficulty() const { return difficulty; }
    void setDifficulty(int diff) { difficulty = diff; }
    OtherNetDataIO* getOtherNetDataIO() const { return otherNetDataIO.get(); }

    int getMoney() const;
    void setMoney(int money);

    std::vector<int> getPropNums() const;
    void setPropNums(const std::vector<int>& propNums);

    std::string getAchievementStr() const;
    void setAchievementStr(const std::string& achievementStr);

    std::vector<std::vector<std::pair<std::string, int>>> getRankLists() const;
    void setRankLists(const std::vector<std::vector<std::pair<std::string, int>>>& ranks);
private:
    std::string userID;
    std::string ip = "127.0.0.1";
    std::string port = "10090";
    int difficulty = 6;
    QWidget* currentWidget = nullptr;
    AchievementsWidget* achievementsWidget = nullptr;
    MenuWidget* menuWidget = nullptr;
    PlayMenuWidget* playMenuWidget = nullptr;
    SettingWidget* settingWidget = nullptr;
    StoreWidget* storeWidget = nullptr;
    RankListWidget* rankListWidget = nullptr;
    SingleModeGameWidget* singleModeGameWidget = nullptr;
    WhirlwindModeGameWidget* whirlwindModeGameWidget = nullptr;
    MultiplayerModeGameWidget* multiplayerModeGameWidget = nullptr;
    PuzzleModeGameWidget* puzzleModeGameWidget = nullptr;
    FinalWidget* finalWidget = nullptr;
    MultiGameWaitWidget* multiGameWaitWidget = nullptr;
    NetDataIO* netDataIO = nullptr;
    LogWindow* logWindow = nullptr;
    std::string gemstoneStyle;
    std::vector<AchievementData> achievementsContainer;
    
    std::unique_ptr<OtherNetDataIO> otherNetDataIO;
    int money = 0;
    std::vector<int> propNums = {0,0,0,0};
    std::string achievementStr = "0000000000";
    std::vector<std::vector<std::pair<std::string, int>>> achievements;

};

#endif // GAME_WINDOW_H
