#ifndef COIN_SYSTEM_H
#define COIN_SYSTEM_H

#include <string>
#include <map>
#include <functional>
#include <QObject>

/**
 * @brief 金币系统
 * 管理用户金币的收集、存储和持久化
 * 单例模式，全局访问
 */
class CoinSystem : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     */
    static CoinSystem& instance();

    /**
     * @brief 初始化金币系统
     * @param userId 用户ID
     */
    void initialize(const std::string& userId);

    /**
     * @brief 获取当前用户ID
     */
    std::string getCurrentUserId() const;

    /**
     * @brief 添加金币
     * @param amount 金币数量
     * @param autoSave 是否自动保存到数据库
     */
    void addCoins(int amount, bool autoSave = true);

    /**
     * @brief 获取当前金币数量
     */
    int getCoins() const;

    /**
     * @brief 设置金币数量
     * @param amount 金币数量
     * @param autoSave 是否自动保存到数据库
     */
    void setCoins(int amount, bool autoSave = true);

    /**
     * @brief 扣除金币
     * @param amount 金币数量
     * @return 是否扣除成功（余额不足时返回false）
     */
    bool deductCoins(int amount);

    /**
     * @brief 保存金币到数据库
     */
    void saveToDatabase();

    /**
     * @brief 从数据库加载金币
     */
    void loadFromDatabase();

    /**
     * @brief 设置数据库保存回调
     * @param callback 回调函数，参数为(userId, coinAmount)
     */
    void setDatabaseSaveCallback(std::function<void(const std::string&, int)> callback);

    /**
     * @brief 设置数据库加载回调
     * @param callback 回调函数，参数为userId，返回金币数量
     */
    void setDatabaseLoadCallback(std::function<int(const std::string&)> callback);

    /**
     * @brief 重置金币系统（清除当前用户数据）
     */
    void reset();

signals:
    /**
     * @brief 金币数量变化信号
     * @param newAmount 新的金币数量
     */
    void coinsChanged(int newAmount);

    /**
     * @brief 金币增加信号
     * @param amount 增加的数量
     */
    void coinsAdded(int amount);

private:
    CoinSystem();
    ~CoinSystem() = default;
    CoinSystem(const CoinSystem&) = delete;
    CoinSystem& operator=(const CoinSystem&) = delete;

    std::string m_currentUserId;
    int m_currentCoins;
    bool m_initialized;

    // 数据库回调函数
    std::function<void(const std::string&, int)> m_dbSaveCallback;
    std::function<int(const std::string&)> m_dbLoadCallback;
};

#endif // COIN_SYSTEM_H
