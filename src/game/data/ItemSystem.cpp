#include "ItemSystem.h"
#include "CoinSystem.h"
#include <QDebug>
#include <QSettings>

ItemSystem& ItemSystem::instance() {
    static ItemSystem instance;
    return instance;
}

ItemSystem::ItemSystem()
    : QObject(nullptr)
    , m_currentUserId("")
    , m_initialized(false)
{
    initializeItems();
}

void ItemSystem::initialize(const std::string& userId) {
    if (userId.empty()) {
        qWarning() << "[ItemSystem] Cannot initialize with empty userId";
        return;
    }

    m_currentUserId = userId;
    m_initialized = true;

    // 清空库存
    m_itemCounts.clear();
    for (const auto& pair : m_itemInfos) {
        m_itemCounts[pair.first] = 0;
    }

    qDebug() << "[ItemSystem] Initialized for user:" << QString::fromStdString(userId);

    // 从数据库加载道具数据
    loadFromDatabase();
}

void ItemSystem::initializeItems() {
    // 冻结时间 - 5金币
    m_itemInfos[ItemType::FREEZE_TIME] = {
        ItemType::FREEZE_TIME,
        "冻结时间",
        "暂停游戏时间10秒",
        5,
        "resources/item/FREEXE_TIME.jpg"
    };

    // 锤子 - 3金币
    m_itemInfos[ItemType::HAMMER] = {
        ItemType::HAMMER,
        "锤子",
        "锤掉一个方块",
        3,
        "resources/item/HAMMER.jpg"
    };

    // 重置全部宝石 - 5金币
    m_itemInfos[ItemType::RESET_BOARD] = {
        ItemType::RESET_BOARD,
        "重置棋盘",
        "重新生成所有宝石",
        5,
        "resources/item/RESET_BOARD.jpg"
    };

    // 消除全部宝石 - 10金币
    m_itemInfos[ItemType::CLEAR_ALL] = {
        ItemType::CLEAR_ALL,
        "清空棋盘",
        "消除所有宝石",
        10,
        "resources/item/CLEAR_ALL.jpg"
    };
}

ItemInfo ItemSystem::getItemInfo(ItemType type) const {
    auto it = m_itemInfos.find(type);
    if (it != m_itemInfos.end()) {
        return it->second;
    }
    qWarning() << "[ItemSystem] Item type not found:" << static_cast<int>(type);
    return ItemInfo();
}

std::map<ItemType, ItemInfo> ItemSystem::getAllItems() const {
    return m_itemInfos;
}

bool ItemSystem::purchaseItem(ItemType type) {
    if (!m_initialized) {
        qWarning() << "[ItemSystem] Not initialized, cannot purchase item";
        return false;
    }

    auto it = m_itemInfos.find(type);
    if (it == m_itemInfos.end()) {
        qWarning() << "[ItemSystem] Item type not found:" << static_cast<int>(type);
        return false;
    }

    const ItemInfo& info = it->second;

    // 检查金币是否足够
    if (!CoinSystem::instance().deductCoins(info.price)) {
        qWarning() << "[ItemSystem] Not enough coins to purchase" << QString::fromStdString(info.name);
        return false;
    }

    // 增加道具数量
    m_itemCounts[type]++;

    qDebug() << "[ItemSystem] Purchased" << QString::fromStdString(info.name)
             << "for" << info.price << "coins. New count:" << m_itemCounts[type];

    emit itemPurchased(type);
    emit itemCountChanged(type, m_itemCounts[type]);

    // 保存到数据库
    saveToDatabase();

    return true;
}

int ItemSystem::getItemCount(ItemType type) const {
    auto it = m_itemCounts.find(type);
    if (it != m_itemCounts.end()) {
        return it->second;
    }
    return 0;
}

bool ItemSystem::useItem(ItemType type) {
    if (!m_initialized) {
        qWarning() << "[ItemSystem] Not initialized, cannot use item";
        return false;
    }

    auto it = m_itemCounts.find(type);
    if (it == m_itemCounts.end() || it->second <= 0) {
        qWarning() << "[ItemSystem] Item not available:" << static_cast<int>(type);
        return false;
    }

    // 减少道具数量
    m_itemCounts[type]--;

    auto infoIt = m_itemInfos.find(type);
    if (infoIt != m_itemInfos.end()) {
        qDebug() << "[ItemSystem] Used" << QString::fromStdString(infoIt->second.name)
                 << ". Remaining count:" << m_itemCounts[type];
    }

    emit itemUsed(type);
    emit itemCountChanged(type, m_itemCounts[type]);

    // 保存到数据库
    saveToDatabase();

    return true;
}

void ItemSystem::saveToDatabase() {
    if (!m_initialized) {
        qWarning() << "[ItemSystem] Not initialized, cannot save";
        return;
    }

    QSettings settings("BejeweledGame", "ItemData");
    settings.beginGroup(QString::fromStdString(m_currentUserId));

    for (const auto& pair : m_itemCounts) {
        QString key = QString("item_%1").arg(static_cast<int>(pair.first));
        settings.setValue(key, pair.second);
    }

    settings.endGroup();
    qDebug() << "[ItemSystem] Saved item data for user:" << QString::fromStdString(m_currentUserId);
}

void ItemSystem::loadFromDatabase() {
    if (!m_initialized) {
        qWarning() << "[ItemSystem] Not initialized, cannot load";
        return;
    }

    QSettings settings("BejeweledGame", "ItemData");
    settings.beginGroup(QString::fromStdString(m_currentUserId));

    for (auto& pair : m_itemCounts) {
        QString key = QString("item_%1").arg(static_cast<int>(pair.first));
        pair.second = settings.value(key, 0).toInt();
    }

    settings.endGroup();
    qDebug() << "[ItemSystem] Loaded item data for user:" << QString::fromStdString(m_currentUserId);

    // 发送所有道具数量变化信号
    for (const auto& pair : m_itemCounts) {
        emit itemCountChanged(pair.first, pair.second);
    }
}

void ItemSystem::reset() {
    m_currentUserId = "";
    m_initialized = false;
    m_itemCounts.clear();
    qDebug() << "[ItemSystem] Reset";
}
