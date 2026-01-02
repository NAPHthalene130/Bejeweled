#include "AchievementSystem.h"
#include "../GameWindow.h"
#include "../gameWidgets/AchievementsWidget.h"
#include "OtherNetDataIO.h"
#include "AchievementData.h"
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>  // 临时调试用

AchievementSystem& AchievementSystem::instance() {
    static AchievementSystem instance;
    return instance;
}

AchievementSystem::AchievementSystem() {
    for (int i = 0; i < 10; ++i) {
        achievements[i] = false;
    }
}

void AchievementSystem::initialize(GameWindow* gw, const std::string& uid) {
    gameWindow = gw;
    userId = uid;
    initialized = true;
    
    // ★★★ 离线模式判断：userId 为 "$#SINGLE#$" 或为空 ★★★
    offlineMode = (uid == "$#SINGLE#$" || uid.empty());
    
    qDebug() << "[AchievementSystem] Initialized for user:" << QString::fromStdString(uid);
    qDebug() << "[AchievementSystem] Offline mode:" << offlineMode;
    
    // 从服务端同步成就（离线模式会自动跳过）
    syncFromServer();
}

void AchievementSystem::syncFromServer() {
    // 离线模式下跳过网络同步
    if (offlineMode) {
        qDebug() << "[AchievementSystem] Offline mode, skip sync from server";
        return;
    }
    
    if (!gameWindow || userId.empty()) {
        qDebug() << "[AchievementSystem] Cannot sync: gameWindow or userId not set";
        return;
    }

    OtherNetDataIO* netIO = gameWindow->getOtherNetDataIO();
    if (!netIO) {
        qDebug() << "[AchievementSystem] Cannot sync: OtherNetDataIO not available";
        return;
    }

    // 调用 getAchievementStr (Type 10)
    std::string serverStr = netIO->getAchievementStr(userId);
    qDebug() << "[AchievementSystem] Got from server:" << QString::fromStdString(serverStr);

    if (serverStr.length() == 10) {
        // 合并服务端成就（或运算）
        mergeAchievements(serverStr);
        
        // 更新GameWindow中的成就显示
        for (int i = 0; i < 10; ++i) {
            updateGameWindowAchievement(i, achievements[i]);
        }
    } else if (serverStr.empty()) {
        // 服务端没有数据，上传本地数据
        syncToServer();
    }
}

void AchievementSystem::syncToServer() {
    // 离线模式下跳过网络同步
    if (offlineMode) {
        qDebug() << "[AchievementSystem] Offline mode, skip sync to server";
        return;
    }
    
    if (!gameWindow || userId.empty()) {
        qDebug() << "[AchievementSystem] Cannot sync to server: not initialized";
        return;
    }

    OtherNetDataIO* netIO = gameWindow->getOtherNetDataIO();
    if (!netIO) {
        qDebug() << "[AchievementSystem] Cannot sync to server: OtherNetDataIO not available";
        return;
    }

    std::string achievementStr = achievementsToString();
    qDebug() << "[AchievementSystem] Uploading to server:" << QString::fromStdString(achievementStr);
    
    // 调用 setAchievementStr (Type 11)
    // 服务端会自动做或运算合并
    bool success = netIO->setAchievementStr(userId, achievementStr);
    if (success) {
        qDebug() << "[AchievementSystem] Successfully synced to server";
    } else {
        qDebug() << "[AchievementSystem] Failed to sync to server";
    }
}

bool AchievementSystem::isUnlocked(AchievementIndex index) const {
    int idx = static_cast<int>(index);
    if (idx >= 0 && idx < 10) {
        return achievements[idx];
    }
    return false;
}

void AchievementSystem::unlock(AchievementIndex index) {
    int idx = static_cast<int>(index);
    if (idx < 0 || idx >= 10) return;
    
    // 已解锁则跳过
    if (achievements[idx]) return;
    
    achievements[idx] = true;
    qDebug() << "[AchievementSystem] Achievement unlocked:" << idx;
    
    // 更新GameWindow中的成就状态
    updateGameWindowAchievement(idx, true);
    
    // 获取成就标题
    QString title;
    if (gameWindow) {
        const auto& achievementList = gameWindow->getAchievements();
        if (idx < static_cast<int>(achievementList.size())) {
            title = achievementList[idx].getTitle();
        }
    }
    
    // 发送信号
    emit achievementUnlocked(idx, title);
    
    // 同步到服务端
    syncToServer();
    
    // 检查是否可以解锁"成就收藏家"
    if (idx != 9) {
        checkAllAchievementsUnlocked();
    }
}

// ========== 触发接口实现 ==========

void AchievementSystem::triggerFirstElimination() {
    if (!isUnlocked(AchievementIndex::FIRST_ELIMINATION)) {
        qDebug() << "[AchievementSystem] Trigger: First Elimination";
        unlock(AchievementIndex::FIRST_ELIMINATION);
    }
}

void AchievementSystem::triggerSpecialGemCreated() {
    if (!isUnlocked(AchievementIndex::SPECIAL_GEM_CREATOR)) {
        qDebug() << "[AchievementSystem] Trigger: Special Gem Created";
        unlock(AchievementIndex::SPECIAL_GEM_CREATOR);
    }
}

void AchievementSystem::triggerCombo(int comboCount) {
    // 三连消及以上计数
    if (comboCount >= 3) {
        sessionComboCount++;
        qDebug() << "[AchievementSystem] Session combo count:" << sessionComboCount;
        
        // 单局累计3次三连消
        if (sessionComboCount >= 3 && !isUnlocked(AchievementIndex::COMBO_ARTIST)) {
            qDebug() << "[AchievementSystem] Trigger: Combo Artist";
            unlock(AchievementIndex::COMBO_ARTIST);
        }

        // 四连消
        if (sessionComboCount >= 4 && !isUnlocked(AchievementIndex::FOUR_MATCH)) {
            qDebug() << "[AchievementSystem] Trigger: Four Match";
            unlock(AchievementIndex::FOUR_MATCH);
        }
        
        // 六连消
        if (sessionComboCount >= 6 && !isUnlocked(AchievementIndex::SIX_MATCH)) {
            qDebug() << "[AchievementSystem] Trigger: Six Match";
            unlock(AchievementIndex::SIX_MATCH);
        }
    }
}

void AchievementSystem::triggerCoinEarned(int coins) {
    totalCoinsEarned += coins;
    qDebug() << "[AchievementSystem] Total coins earned:" << totalCoinsEarned;
    
    // 累计100金币
    if (totalCoinsEarned >= 100 && !isUnlocked(AchievementIndex::COIN_COLLECTOR)) {
        qDebug() << "[AchievementSystem] Trigger: Coin Collector";
        unlock(AchievementIndex::COIN_COLLECTOR);
    }
}

void AchievementSystem::triggerMatchCount(int matchCount) {
    qDebug() << "[AchievementSystem] Match count:" << matchCount;
    // 三连消及以上计数
    if (matchCount >= 3) {
        sessionComboCount1++;
        qDebug() << "[AchievementSystem] Session combo count:" << sessionComboCount1;
        
        // 单局累计3次三连消
        return ;

    }
    
}

void AchievementSystem::triggerSingleModeComplete(int timeSeconds) {
    qDebug() << "[AchievementSystem] Single mode complete in" << timeSeconds << "seconds";
    
    // 5分钟 = 300秒内完成
    if (timeSeconds <= 300 && !isUnlocked(AchievementIndex::SPEED_MASTER)) {
        qDebug() << "[AchievementSystem] Trigger: Speed Master";
        unlock(AchievementIndex::SPEED_MASTER);
    }
}

void AchievementSystem::triggerPuzzleModeComplete() {
    if (!isUnlocked(AchievementIndex::PUZZLE_CONQUEROR)) {
        qDebug() << "[AchievementSystem] Trigger: Puzzle Conqueror";
        unlock(AchievementIndex::PUZZLE_CONQUEROR);
    }
}

void AchievementSystem::triggerWhirlwindSurvival(int timeSeconds) {
    qDebug() << "[AchievementSystem] Whirlwind survival:" << timeSeconds << "seconds";
    
    // 坚持2分钟 = 120秒
    if (timeSeconds >= 120 && !isUnlocked(AchievementIndex::WHIRLWIND_TRIAL)) {
        qDebug() << "[AchievementSystem] Trigger: Whirlwind Trial";
        unlock(AchievementIndex::WHIRLWIND_TRIAL);
    }
}

void AchievementSystem::resetSessionStats() {
    sessionComboCount = 0;
    qDebug() << "[AchievementSystem] Session stats reset";
}

std::string AchievementSystem::getAchievementString() const {
    return achievementsToString();
}

// ========== 辅助方法 ==========

void AchievementSystem::checkAllAchievementsUnlocked() {
    // 检查前9个成就是否都已解锁
    bool allUnlocked = true;
    for (int i = 0; i < 9; ++i) {
        if (!achievements[i]) {
            allUnlocked = false;
            break;
        }
    }
    
    if (allUnlocked && !achievements[9]) {
        qDebug() << "[AchievementSystem] All achievements unlocked! Unlocking Achievement Collector";
        unlock(AchievementIndex::ACHIEVEMENT_COLLECTOR);
    }
}

void AchievementSystem::updateGameWindowAchievement(int index, bool unlocked) {
    if (!gameWindow) return;
    
    auto& achievementList = gameWindow->getAchievements();
    if (index < 0 || index >= static_cast<int>(achievementList.size())) return;
    
    if (unlocked && !achievementList[index].isUnlocked()) {
        achievementList[index].setUnlocked(true);
        achievementList[index].setCompletedAt(QDateTime::currentDateTime());
        qDebug() << "[AchievementSystem] Updated GameWindow achievement:" << index;
        
        // 刷新成就界面
        auto* achievementsWidget = gameWindow->getAchievementsWidget();
        if (achievementsWidget) {
            achievementsWidget->updateView();
        }
    }
}

std::string AchievementSystem::achievementsToString() const {
    std::string result;
    result.reserve(10);
    for (int i = 0; i < 10; ++i) {
        result += (achievements[i] ? '1' : '0');
    }
    return result;
}

void AchievementSystem::stringToAchievements(const std::string& str) {
    if (str.length() != 10) {
        qDebug() << "[AchievementSystem] Invalid achievement string length:" << str.length();
        return;
    }
    
    for (int i = 0; i < 10; ++i) {
        achievements[i] = (str[i] == '1');
    }
}

void AchievementSystem::mergeAchievements(const std::string& serverStr) {
    if (serverStr.length() != 10) return;
    
    // 或运算：本地为1或服务端为1，结果都是1
    for (int i = 0; i < 10; ++i) {
        if (serverStr[i] == '1') {
            achievements[i] = true;
        }
    }
    
    qDebug() << "[AchievementSystem] Merged achievements:" << QString::fromStdString(achievementsToString());
}
