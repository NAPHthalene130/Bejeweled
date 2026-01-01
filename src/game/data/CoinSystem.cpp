#include "CoinSystem.h"
#include "CoinDatabase.h"
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

    // 从数据库加载金币
    loadFromDatabase();
}

std::string CoinSystem::getCurrentUserId() const {
    return m_currentUserId;
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
    } else {
        // 使用CoinDatabase保存
        qDebug() << "[CoinSystem] Saving to CoinDatabase:" << m_currentCoins << "coins";
        CoinDatabase::instance().saveCoinData(m_currentUserId, m_currentCoins);
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
    } else {
        // 使用CoinDatabase加载
        loadedCoins = CoinDatabase::instance().loadCoinData(m_currentUserId);
        qDebug() << "[CoinSystem] Loaded from CoinDatabase:" << loadedCoins << "coins";
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
    qDebug() << "[CoinSystem] Reset";
}
