#ifndef BACKGROUND_MANAGER_H
#define BACKGROUND_MANAGER_H

#include <string>

class BackgroundManager {
public:
    static BackgroundManager& instance();

    const std::string& getAuthBackground() const;
    void setAuthBackground(const std::string& authBackground);

    const std::string& getFinalWidgetBackground() const;
    void setFinalWidgetBackground(const std::string& finalWidgetBackground);

    const std::string& getSettingbackground() const;
    void setSettingbackground(const std::string& settingbackground);

    const std::string& getAchievementBackground() const;
    void setAchievementBackground(const std::string& achievementBackground);

private:
    BackgroundManager();

    std::string authBackground;
    std::string finalWidgetBackground;
    std::string settingbackground;
    std::string achievementBackground;
};

#endif // BACKGROUND_MANAGER_H
