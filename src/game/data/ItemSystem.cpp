#include "ItemSystem.h"
#include "CoinSystem.h"
#include "OtherNetDataIO.h"
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
    , m_networkIO(nullptr)
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

    // 判断是否为离线模式
    if (isOfflineMode()) {
        qDebug() << "[ItemSystem] Offline mode detected, will use local storage";
        // 离线模式：从本地加载
        loadFromDatabase();
    } else {
        qDebug() << "[ItemSystem] Online mode detected, will load from server in GameWindow";
        // 在线模式：不在这里加载，由GameWindow通过setItemCounts设置
    }
}

void ItemSystem::setNetworkIO(OtherNetDataIO* netIO) {
    m_networkIO = netIO;
    qDebug() << "[ItemSystem] Network IO set:" << (netIO != nullptr ? "enabled" : "disabled");
}

bool ItemSystem::isOfflineMode() const {
    return m_currentUserId == "$#SINGLE#$";
}

void ItemSystem::setItemCounts(const std::vector<int>& propNums) {
    if (!m_initialized) {
        qWarning() << "[ItemSystem] Not initialized, cannot set item counts";
        return;
    }

    if (propNums.size() != 4) {
        qWarning() << "[ItemSystem] Invalid propNums size:" << propNums.size();
        return;
    }

    // 设置道具数量
    m_itemCounts[ItemType::FREEZE_TIME] = propNums[0];
    m_itemCounts[ItemType::HAMMER] = propNums[1];
    m_itemCounts[ItemType::RESET_BOARD] = propNums[2];
    m_itemCounts[ItemType::CLEAR_ALL] = propNums[3];

    qDebug() << "[ItemSystem] Item counts set from server:"
             << "FREEZE_TIME=" << propNums[0]
             << "HAMMER=" << propNums[1]
             << "RESET_BOARD=" << propNums[2]
             << "CLEAR_ALL=" << propNums[3];

    // 发送所有道具数量变化信号以更新UI
    for (const auto& pair : m_itemCounts) {
        emit itemCountChanged(pair.first, pair.second);
    }
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

    // 转换道具数量为vector格式 (按照ItemType枚举顺序)
    std::vector<int> propNums(4, 0);
    propNums[0] = getItemCount(ItemType::FREEZE_TIME);
    propNums[1] = getItemCount(ItemType::HAMMER);
    propNums[2] = getItemCount(ItemType::RESET_BOARD);
    propNums[3] = getItemCount(ItemType::CLEAR_ALL);

    if (isOfflineMode()) {
        // 离线模式：使用QSettings本地存储
        QSettings settings("BejeweledGame", "ItemData");
        settings.beginGroup(QString::fromStdString(m_currentUserId));

        for (const auto& pair : m_itemCounts) {
            QString key = QString("item_%1").arg(static_cast<int>(pair.first));
            settings.setValue(key, pair.second);
        }

        settings.endGroup();
        qDebug() << "[ItemSystem] Saved item data locally for user:" << QString::fromStdString(m_currentUserId);
    } else {
        // 在线模式：同步到服务器
        if (m_networkIO) {
            bool success = m_networkIO->setPropNums(m_currentUserId, propNums);
            if (success) {
                qDebug() << "[ItemSystem] Synced item data to server for user:" << QString::fromStdString(m_currentUserId);
            } else {
                qWarning() << "[ItemSystem] Failed to sync item data to server, falling back to local storage";
                // 失败时回退到本地存储
                QSettings settings("BejeweledGame", "ItemData");
                settings.beginGroup(QString::fromStdString(m_currentUserId));
                for (const auto& pair : m_itemCounts) {
                    QString key = QString("item_%1").arg(static_cast<int>(pair.first));
                    settings.setValue(key, pair.second);
                }
                settings.endGroup();
            }
        } else {
            qWarning() << "[ItemSystem] Network IO not set, using local storage";
            QSettings settings("BejeweledGame", "ItemData");
            settings.beginGroup(QString::fromStdString(m_currentUserId));
            for (const auto& pair : m_itemCounts) {
                QString key = QString("item_%1").arg(static_cast<int>(pair.first));
                settings.setValue(key, pair.second);
            }
            settings.endGroup();
        }
    }
}

void ItemSystem::loadFromDatabase() {
    if (!m_initialized) {
        qWarning() << "[ItemSystem] Not initialized, cannot load";
        return;
    }

    if (isOfflineMode()) {
        // 离线模式：从QSettings本地加载
        QSettings settings("BejeweledGame", "ItemData");
        settings.beginGroup(QString::fromStdString(m_currentUserId));

        for (auto& pair : m_itemCounts) {
            QString key = QString("item_%1").arg(static_cast<int>(pair.first));
            pair.second = settings.value(key, 0).toInt();
        }

        settings.endGroup();
        qDebug() << "[ItemSystem] Loaded item data locally for user:" << QString::fromStdString(m_currentUserId);
    } else {
        // 在线模式：从服务器加载
        if (m_networkIO) {
            std::vector<int> propNums = m_networkIO->getPropNums(m_currentUserId);

            if (propNums.size() == 4) {
                // 按照顺序解析道具数量
                m_itemCounts[ItemType::FREEZE_TIME] = propNums[0];
                m_itemCounts[ItemType::HAMMER] = propNums[1];
                m_itemCounts[ItemType::RESET_BOARD] = propNums[2];
                m_itemCounts[ItemType::CLEAR_ALL] = propNums[3];

                qDebug() << "[ItemSystem] Loaded item data from server for user:" << QString::fromStdString(m_currentUserId);
            } else {
                qWarning() << "[ItemSystem] Failed to load from server, falling back to local storage";
                // 失败时回退到本地存储
                QSettings settings("BejeweledGame", "ItemData");
                settings.beginGroup(QString::fromStdString(m_currentUserId));
                for (auto& pair : m_itemCounts) {
                    QString key = QString("item_%1").arg(static_cast<int>(pair.first));
                    pair.second = settings.value(key, 0).toInt();
                }
                settings.endGroup();
            }
        } else {
            qWarning() << "[ItemSystem] Network IO not set, using local storage";
            QSettings settings("BejeweledGame", "ItemData");
            settings.beginGroup(QString::fromStdString(m_currentUserId));
            for (auto& pair : m_itemCounts) {
                QString key = QString("item_%1").arg(static_cast<int>(pair.first));
                pair.second = settings.value(key, 0).toInt();
            }
            settings.endGroup();
        }
    }

    // 发送所有道具数量变化信号
    for (const auto& pair : m_itemCounts) {
        emit itemCountChanged(pair.first, pair.second);
    }
}

void ItemSystem::reset() {
    m_currentUserId = "";
    m_initialized = false;
    m_networkIO = nullptr;
    m_itemCounts.clear();
    qDebug() << "[ItemSystem] Reset";
}
