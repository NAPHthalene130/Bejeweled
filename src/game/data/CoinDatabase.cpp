#include "CoinDatabase.h"
#include <QSettings>
#include <QDebug>

CoinDatabase& CoinDatabase::instance() {
    static CoinDatabase instance;
    return instance;
}

CoinDatabase::CoinDatabase()
    : QObject(nullptr)
    , m_networkSyncEnabled(false)
    , m_networkSaveCallback(nullptr)
    , m_networkLoadCallback(nullptr)
{
    qDebug() << "[CoinDatabase] Initialized";
}

bool CoinDatabase::saveCoinData(const std::string& userId, int coinAmount) {
    if (userId.empty()) {
        qWarning() << "[CoinDatabase] Cannot save with empty userId";
        return false;
    }

    bool success = true;

    // 1. 总是保存到本地存储（QSettings）
    QSettings settings("GemMatch", "CoinDatabase");
    QString key = "UserCoins/" + QString::fromStdString(userId);
    settings.setValue(key, coinAmount);
    settings.sync();

    qDebug() << "[CoinDatabase] Saved to local storage:" << coinAmount
             << "coins for user:" << QString::fromStdString(userId);

    // 2. 如果启用网络同步，调用网络回调
    if (m_networkSyncEnabled && m_networkSaveCallback) {
        qDebug() << "[CoinDatabase] Attempting network sync...";
        bool networkSuccess = m_networkSaveCallback(userId, coinAmount);

        if (networkSuccess) {
            qDebug() << "[CoinDatabase] Network sync successful";
        } else {
            qWarning() << "[CoinDatabase] Network sync failed, data saved locally only";
            success = false;
        }
    }

    emit dataSaved(success);
    return success;
}

int CoinDatabase::loadCoinData(const std::string& userId) {
    if (userId.empty()) {
        qWarning() << "[CoinDatabase] Cannot load with empty userId";
        return -1;
    }

    int coinAmount = 0;
    bool loadedFromNetwork = false;

    // 1. 如果启用网络同步，优先从网络加载
    if (m_networkSyncEnabled && m_networkLoadCallback) {
        qDebug() << "[CoinDatabase] Attempting to load from network...";
        int networkCoins = m_networkLoadCallback(userId);

        if (networkCoins >= 0) {
            coinAmount = networkCoins;
            loadedFromNetwork = true;
            qDebug() << "[CoinDatabase] Loaded from network:" << coinAmount << "coins";

            // 更新本地存储
            QSettings settings("GemMatch", "CoinDatabase");
            QString key = "UserCoins/" + QString::fromStdString(userId);
            settings.setValue(key, coinAmount);
        } else {
            qWarning() << "[CoinDatabase] Network load failed, falling back to local storage";
        }
    }

    // 2. 如果没有从网络加载成功，从本地存储加载
    if (!loadedFromNetwork) {
        QSettings settings("GemMatch", "CoinDatabase");
        QString key = "UserCoins/" + QString::fromStdString(userId);
        coinAmount = settings.value(key, 0).toInt();
        qDebug() << "[CoinDatabase] Loaded from local storage:" << coinAmount << "coins";
    }

    emit dataLoaded(true, coinAmount);
    return coinAmount;
}

void CoinDatabase::setNetworkSaveCallback(std::function<bool(const std::string&, int)> callback) {
    m_networkSaveCallback = callback;
    qDebug() << "[CoinDatabase] Network save callback registered";
}

void CoinDatabase::setNetworkLoadCallback(std::function<int(const std::string&)> callback) {
    m_networkLoadCallback = callback;
    qDebug() << "[CoinDatabase] Network load callback registered";
}

void CoinDatabase::setNetworkSyncEnabled(bool enabled) {
    m_networkSyncEnabled = enabled;
    qDebug() << "[CoinDatabase] Network sync" << (enabled ? "enabled" : "disabled");
}

bool CoinDatabase::isNetworkSyncEnabled() const {
    return m_networkSyncEnabled;
}
