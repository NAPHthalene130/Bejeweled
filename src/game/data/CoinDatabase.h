#ifndef COIN_DATABASE_H
#define COIN_DATABASE_H

#include <string>
#include <functional>
#include <QObject>
#include "GameNetData.h"

/**
 * @brief 金币数据库接口
 * 提供金币数据的持久化功能
 * 通过网络或本地数据库保存和加载金币数据
 */
class CoinDatabase : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     */
    static CoinDatabase& instance();

    /**
     * @brief 保存金币到数据库
     * @param userId 用户ID
     * @param coinAmount 金币数量
     * @return 是否保存成功
     */
    bool saveCoinData(const std::string& userId, int coinAmount);

    /**
     * @brief 从数据库加载金币
     * @param userId 用户ID
     * @return 金币数量，失败返回-1
     */
    int loadCoinData(const std::string& userId);

    /**
     * @brief 设置网络保存回调
     * 用于通过网络同步金币数据到服务器
     * @param callback 回调函数，参数为(userId, coinAmount)，返回是否成功
     */
    void setNetworkSaveCallback(std::function<bool(const std::string&, int)> callback);

    /**
     * @brief 设置网络加载回调
     * 用于从服务器加载金币数据
     * @param callback 回调函数，参数为userId，返回金币数量
     */
    void setNetworkLoadCallback(std::function<int(const std::string&)> callback);

    /**
     * @brief 启用/禁用网络同步
     */
    void setNetworkSyncEnabled(bool enabled);

    /**
     * @brief 检查是否启用网络同步
     */
    bool isNetworkSyncEnabled() const;

signals:
    /**
     * @brief 数据保存完成信号
     */
    void dataSaved(bool success);

    /**
     * @brief 数据加载完成信号
     */
    void dataLoaded(bool success, int coinAmount);

private:
    CoinDatabase();
    ~CoinDatabase() = default;
    CoinDatabase(const CoinDatabase&) = delete;
    CoinDatabase& operator=(const CoinDatabase&) = delete;

    bool m_networkSyncEnabled;
    std::function<bool(const std::string&, int)> m_networkSaveCallback;
    std::function<int(const std::string&)> m_networkLoadCallback;
};

#endif // COIN_DATABASE_H
