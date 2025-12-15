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

private:
    std::string userID;
    QWidget* currentWidget = nullptr;

    AchievementsWidget* achievementsWidget = nullptr;
    MenuWidget* menuWidget = nullptr;
    PlayMenuWidget* playMenuWidget = nullptr;
    SettingWidget* settingWidget = nullptr;
    StoreWidget* storeWidget = nullptr;
    RankListWidget* rankListWidget = nullptr;
    SingleModeGameWidget* singleModeGameWidget = nullptr;

};

#endif // GAME_WINDOW_H
