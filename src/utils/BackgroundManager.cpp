#include "BackgroundManager.h"

BackgroundManager& BackgroundManager::instance() {
    static BackgroundManager instance;
    return instance;
}

BackgroundManager::BackgroundManager()
    : authBackground("images/auth_bg.png"),
      finalWidgetBackground("images/final_bg.png"),
      settingbackground("images/setting_bg.png"),
      achievementBackground("images/achievement_bg.png") {}

const std::string& BackgroundManager::getAuthBackground() const {
    return authBackground;
}

void BackgroundManager::setAuthBackground(const std::string& authBackground) {
    this->authBackground = authBackground;
}

const std::string& BackgroundManager::getFinalWidgetBackground() const {
    return finalWidgetBackground;
}

void BackgroundManager::setFinalWidgetBackground(const std::string& finalWidgetBackground) {
    this->finalWidgetBackground = finalWidgetBackground;
}

const std::string& BackgroundManager::getSettingbackground() const {
    return settingbackground;
}

void BackgroundManager::setSettingbackground(const std::string& settingbackground) {
    this->settingbackground = settingbackground;
}

const std::string& BackgroundManager::getAchievementBackground() const {
    return achievementBackground;
}

void BackgroundManager::setAchievementBackground(const std::string& achievementBackground) {
    this->achievementBackground = achievementBackground;
}
