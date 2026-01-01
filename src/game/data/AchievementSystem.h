#ifndef ACHIEVEMENTSYSTEM_H
#define ACHIEVEMENTSYSTEM_H

#include <string>
#include <vector>
#include <QObject>

class GameWindow;

/**
 * 成就索引定义（与服务端字符串位置对应）
 * 0: 初试锋芒 - 首次宝石消除
 * 1: 奇异宝石创造者 - 首次合成特殊宝石
 * 2: 连击艺术 - 单局3次三连消
 * 3: 金币盈囊 - 累计100金币
 * 4: 四消突破 - 首次四连消
 * 5: 极速掌控 - 单人模式5分钟内完成
 * 6: 随机谜题征服者 - 解谜模式首次通关
 * 7: 旋风试炼 - 旋风模式坚持2分钟
 * 8: 破除界限 - 首次六连消
 * 9: 成就收藏家 - 解锁全部成就
 */
enum class AchievementIndex {
    FIRST_ELIMINATION = 0,      // 初试锋芒
    SPECIAL_GEM_CREATOR = 1,    // 奇异宝石创造者
    COMBO_ARTIST = 2,           // 连击艺术
    COIN_COLLECTOR = 3,         // 金币盈囊
    FOUR_MATCH = 4,             // 四消突破
    SPEED_MASTER = 5,           // 极速掌控
    PUZZLE_CONQUEROR = 6,       // 随机谜题征服者
    WHIRLWIND_TRIAL = 7,        // 旋风试炼
    SIX_MATCH = 8,              // 破除界限
    ACHIEVEMENT_COLLECTOR = 9   // 成就收藏家
};

class AchievementSystem : public QObject {
    Q_OBJECT

public:
    static AchievementSystem& instance();

    // 初始化，需要传入GameWindow和用户ID
    void initialize(GameWindow* gameWindow, const std::string& userId);

    // 从服务端同步成就数据（调用 OtherNetDataIO::getAchievementStr，Type 10）
    void syncFromServer();

    // 上传成就数据到服务端（调用 OtherNetDataIO::setAchievementStr，Type 11）
    void syncToServer();

    // 检查成就是否已解锁
    bool isUnlocked(AchievementIndex index) const;

    // 解锁成就
    void unlock(AchievementIndex index);

    // ========== 触发接口 ==========
    
    void triggerFirstElimination();           // 首次消除
    void triggerSpecialGemCreated();          // 合成特殊宝石
    void triggerCombo(int comboCount);        // 连击统计
    void triggerCoinEarned(int coins);        // 获得金币
    void triggerMatchCount(int matchCount);   // 消除数量（检测四连消、六连消）
    void triggerSingleModeComplete(int timeSeconds);  // 单人模式完成
    void triggerPuzzleModeComplete();         // 解谜模式通关
    void triggerWhirlwindSurvival(int timeSeconds);   // 旋风模式坚持时间

    // 重置单局统计（每局开始时调用）
    void resetSessionStats();

    // 获取成就字符串
    std::string getAchievementString() const;

    // 设置离线模式（离线模式下跳过网络同步）
    void setOfflineMode(bool offline) { offlineMode = offline; }
    bool isOfflineMode() const { return offlineMode; }

signals:
    void achievementUnlocked(int index, const QString& title);

private:
    AchievementSystem();
    ~AchievementSystem() = default;
    AchievementSystem(const AchievementSystem&) = delete;
    AchievementSystem& operator=(const AchievementSystem&) = delete;

    void checkAllAchievementsUnlocked();
    void updateGameWindowAchievement(int index, bool unlocked);
    std::string achievementsToString() const;
    void stringToAchievements(const std::string& str);
    void mergeAchievements(const std::string& serverStr);

    GameWindow* gameWindow = nullptr;
    std::string userId;
    
    bool achievements[10] = {false};
    int sessionComboCount = 0;
    int totalCoinsEarned = 0;
    bool initialized = false;
    bool offlineMode = false;  // 离线模式标志
};

#endif // ACHIEVEMENTSYSTEM_H
