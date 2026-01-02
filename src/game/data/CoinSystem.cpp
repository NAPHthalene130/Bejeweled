#include "CoinSystem.h"
#include "CoinDatabase.h"
#include "OtherNetDataIO.h"
#include <QDebug>
#include <QSettings>

CoinSystem& CoinSystem::instance() {
    static CoinSystem instance;
    return instance;
}

CoinSystem::CoinSystem()
    : QObject(nullptr)
    , m_currentUserId("")
    , m_currentCoins(0)
    , m_initialized(false)
    , m_networkIO(nullptr)
    , m_dbSaveCallback(nullptr)
    , m_dbLoadCallback(nullptr)
{
}

void CoinSystem::initialize(const std::string& userId) {
    if (userId.empty()) {
        qWarning() << "[CoinSystem] Cannot initialize with empty userId";
        return;
    }

    m_currentUserId = userId;
    m_currentCoins = 0;
    m_initialized = true;

    qDebug() << "[CoinSystem] Initialized for user:" << QString::fromStdString(userId);

    // 判断是否为离线模式
    if (isOfflineMode()) {
        qDebug() << "[CoinSystem] Offline mode detected, will use local storage";
        // 离线模式：从本地加载
        loadFromDatabase();
    } else {
        qDebug() << "[CoinSystem] Online mode detected, will load from server in GameWindow";
        // 在线模式：不在这里加载，由GameWindow通过setCoins设置
    }
}

std::string CoinSystem::getCurrentUserId() const {
    return m_currentUserId;
}

void CoinSystem::setNetworkIO(OtherNetDataIO* netIO) {
    m_networkIO = netIO;
    qDebug() << "[CoinSystem] Network IO set:" << (netIO != nullptr ? "enabled" : "disabled");
}

bool CoinSystem::isOfflineMode() const {
    return m_currentUserId == "$#SINGLE#$";
}

void CoinSystem::addCoins(int amount, bool autoSave) {
    if (!m_initialized) {
        qWarning() << "[CoinSystem] Not initialized, cannot add coins";
        return;
    }

    if (amount <= 0) {
        qWarning() << "[CoinSystem] Invalid coin amount:" << amount;
        return;
    }

    m_currentCoins += amount;

    qDebug() << "[CoinSystem] Added" << amount << "coins. Total:" << m_currentCoins;

    emit coinsAdded(amount);
    emit coinsChanged(m_currentCoins);

    if (autoSave) {
        saveToDatabase();
    }
}

int CoinSystem::getCoins() const {
    return m_currentCoins;
}

void CoinSystem::setCoins(int amount, bool autoSave) {

    if (!m_initialized) {
        qWarning() << "[CoinSystem] Not initialized, cannot set coins";
        return;
    }

    if (amount < 0) {
        qWarning() << "[CoinSystem] Invalid coin amount:" << amount;
        return;
    }

    m_currentCoins = amount;

    qDebug() << "[CoinSystem] Set coins to:" << m_currentCoins;

    emit coinsChanged(m_currentCoins);

    if (autoSave) {
        saveToDatabase();
    }
}

bool CoinSystem::deductCoins(int amount) {
    if (!m_initialized) {
        qWarning() << "[CoinSystem] Not initialized, cannot deduct coins";
        return false;
    }

    if (amount <= 0) {
        qWarning() << "[CoinSystem] Invalid coin amount:" << amount;
        return false;
    }

    if (m_currentCoins < amount) {
        qWarning() << "[CoinSystem] Insufficient coins. Have:" << m_currentCoins << "Need:" << amount;
        return false;
    }

    m_currentCoins -= amount;

    qDebug() << "[CoinSystem] Deducted" << amount << "coins. Remaining:" << m_currentCoins;

    emit coinsChanged(m_currentCoins);
    saveToDatabase();

    return true;
}

void CoinSystem::saveToDatabase() {
    if (!m_initialized) {
        qWarning() << "[CoinSystem] Not initialized, cannot save";
        return;
    }

    if (m_dbSaveCallback) {
        qDebug() << "[CoinSystem] Saving to database via callback:" << m_currentCoins
                 << "coins for user:" << QString::fromStdString(m_currentUserId);
        m_dbSaveCallback(m_currentUserId, m_currentCoins);
    } else if (isOfflineMode()) {
        // 离线模式：使用QSettings本地存储
        QSettings settings("BejeweledGame", "CoinData");
        settings.setValue(QString::fromStdString(m_currentUserId), m_currentCoins);
        qDebug() << "[CoinSystem] Saved coin data locally for user:" << QString::fromStdString(m_currentUserId);
    } else {
        // 在线模式：同步到服务器
        if (m_networkIO) {
            bool success = m_networkIO->setMoney(m_currentUserId, m_currentCoins);
            if (success) {
                qDebug() << "[CoinSystem] Synced coin data to server for user:" << QString::fromStdString(m_currentUserId);
            } else {
                qWarning() << "[CoinSystem] Failed to sync coin data to server, falling back to local storage";
                // 失败时回退到本地存储
                QSettings settings("BejeweledGame", "CoinData");
                settings.setValue(QString::fromStdString(m_currentUserId), m_currentCoins);
            }
        } else {
            qWarning() << "[CoinSystem] Network IO not set, using local storage";
            QSettings settings("BejeweledGame", "CoinData");
            settings.setValue(QString::fromStdString(m_currentUserId), m_currentCoins);
        }
    }
}

void CoinSystem::loadFromDatabase() {
    if (!m_initialized) {
        qWarning() << "[CoinSystem] Not initialized, cannot load";
        return;
    }

    int loadedCoins = 0;

    if (m_dbLoadCallback) {
        loadedCoins = m_dbLoadCallback(m_currentUserId);
        qDebug() << "[CoinSystem] Loaded from database via callback:" << loadedCoins
                 << "coins for user:" << QString::fromStdString(m_currentUserId);
    } else if (isOfflineMode()) {
        // 离线模式：从QSettings本地加载
        QSettings settings("BejeweledGame", "CoinData");
        loadedCoins = settings.value(QString::fromStdString(m_currentUserId), 0).toInt();
        qDebug() << "[CoinSystem] Loaded coin data locally for user:" << QString::fromStdString(m_currentUserId);
    } else {
        // 在线模式：从服务器加载
        if (m_networkIO) {
            loadedCoins = m_networkIO->getMoney(m_currentUserId);
            if (loadedCoins >= 0) {
                qDebug() << "[CoinSystem] Loaded coin data from server for user:" << QString::fromStdString(m_currentUserId);
            } else {
                qWarning() << "[CoinSystem] Failed to load from server, falling back to local storage";
                // 失败时回退到本地存储
                QSettings settings("BejeweledGame", "CoinData");
                loadedCoins = settings.value(QString::fromStdString(m_currentUserId), 0).toInt();
            }
        } else {
            qWarning() << "[CoinSystem] Network IO not set, using local storage";
            QSettings settings("BejeweledGame", "CoinData");
            loadedCoins = settings.value(QString::fromStdString(m_currentUserId), 0).toInt();
        }
    }

    if (loadedCoins < 0) {
        loadedCoins = 0;
    }

    m_currentCoins = loadedCoins;
    emit coinsChanged(m_currentCoins);
}

void CoinSystem::setDatabaseSaveCallback(std::function<void(const std::string&, int)> callback) {
    m_dbSaveCallback = callback;
    qDebug() << "[CoinSystem] Database save callback registered";
}

void CoinSystem::setDatabaseLoadCallback(std::function<int(const std::string&)> callback) {
    m_dbLoadCallback = callback;
    qDebug() << "[CoinSystem] Database load callback registered";
}

void CoinSystem::reset() {
    m_currentUserId = "";
    m_currentCoins = 0;
    m_initialized = false;
    m_networkIO = nullptr;
    qDebug() << "[CoinSystem] Reset";
}
