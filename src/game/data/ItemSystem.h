#ifndef ITEM_SYSTEM_H
#define ITEM_SYSTEM_H

#include <QObject>
#include <string>
#include <map>

class OtherNetDataIO;

/**
 * @brief 道具类型枚举
 */
enum class ItemType {
    FREEZE_TIME,      // 冻结时间
    HAMMER,           // 锤子
    RESET_BOARD,      // 重置全部宝石
    CLEAR_ALL         // 消除全部宝石
};

/**
 * @brief 道具信息结构
 */
struct ItemInfo {
    ItemType type;
    std::string name;
    std::string description;
    int price;
    std::string icon;
};

/**
 * @brief 道具系统
 * 管理道具的购买、使用和库存
 * 单例模式，全局访问
 */
class ItemSystem : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     */
    static ItemSystem& instance();

    /**
     * @brief 初始化道具系统
     * @param userId 用户ID
     */
    void initialize(const std::string& userId);

    /**
     * @brief 设置网络IO对象（用于在线模式）
     * @param netIO OtherNetDataIO指针
     */
    void setNetworkIO(OtherNetDataIO* netIO);

    /**
     * @brief 检查是否为离线模式
     * @return true表示离线模式，false表示在线模式
     */
    bool isOfflineMode() const;

    /**
     * @brief 直接设置道具数量（用于从服务器加载）
     * @param propNums 道具数量数组 [FREEZE_TIME, HAMMER, RESET_BOARD, CLEAR_ALL]
     */
    void setItemCounts(const std::vector<int>& propNums);

    /**
     * @brief 获取道具信息
     * @param type 道具类型
     */
    ItemInfo getItemInfo(ItemType type) const;

    /**
     * @brief 获取所有道具信息
     */
    std::map<ItemType, ItemInfo> getAllItems() const;

    /**
     * @brief 购买道具
     * @param type 道具类型
     * @return 是否购买成功
     */
    bool purchaseItem(ItemType type);

    /**
     * @brief 获取道具数量
     * @param type 道具类型
     */
    int getItemCount(ItemType type) const;

    /**
     * @brief 使用道具
     * @param type 道具类型
     * @return 是否使用成功
     */
    bool useItem(ItemType type);

    /**
     * @brief 保存道具数据到数据库
     */
    void saveToDatabase();

    /**
     * @brief 从数据库加载道具数据
     */
    void loadFromDatabase();

    /**
     * @brief 重置道具系统（清除当前用户数据）
     */
    void reset();

signals:
    /**
     * @brief 道具购买成功信号
     * @param type 道具类型
     */
    void itemPurchased(ItemType type);

    /**
     * @brief 道具使用信号
     * @param type 道具类型
     */
    void itemUsed(ItemType type);

    /**
     * @brief 道具数量变化信号
     * @param type 道具类型
     * @param newCount 新的数量
     */
    void itemCountChanged(ItemType type, int newCount);

private:
    ItemSystem();
    ~ItemSystem() = default;
    ItemSystem(const ItemSystem&) = delete;
    ItemSystem& operator=(const ItemSystem&) = delete;

    void initializeItems();

    std::string m_currentUserId;
    bool m_initialized;

    // 网络IO对象（用于在线模式）
    OtherNetDataIO* m_networkIO;

    // 道具信息
    std::map<ItemType, ItemInfo> m_itemInfos;

    // 道具库存
    std::map<ItemType, int> m_itemCounts;
};

#endif // ITEM_SYSTEM_H
