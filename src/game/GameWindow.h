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
class FinalWidget;
class MultiGameWaitWidget;
class NetDataIO;
class LogWindow;

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
private:
    std::string userID;
    std::string ip = "127.0.0.1";
    std::string port = "10090";
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
    FinalWidget* finalWidget = nullptr;
    MultiGameWaitWidget* multiGameWaitWidget = nullptr;
    NetDataIO* netDataIO = nullptr;
    LogWindow* logWindow = nullptr;
    std::string gemstoneStyle;
    std::vector<AchievementData> achievementsContainer;

};

#endif // GAME_WINDOW_H
