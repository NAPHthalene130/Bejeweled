#include <QCoreApplication>
#include <QDebug>
#include "game/data/CoinSystem.h"
#include "game/data/CoinDatabase.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "========================================";
    qDebug() << "Testing Coin System";
    qDebug() << "========================================";

    // Test 1: Initialize coin system
    qDebug() << "\n[Test 1] Initializing coin system...";
    CoinSystem::instance().initialize("test_user_001");
    qDebug() << "Initial coins:" << CoinSystem::instance().getCoins();

    // Test 2: Add coins
    qDebug() << "\n[Test 2] Adding coins...";
    CoinSystem::instance().addCoins(10, false);
    qDebug() << "After adding 10:" << CoinSystem::instance().getCoins();

    CoinSystem::instance().addCoins(25, false);
    qDebug() << "After adding 25:" << CoinSystem::instance().getCoins();

    // Test 3: Save to database
    qDebug() << "\n[Test 3] Saving to database...";
    CoinSystem::instance().saveToDatabase();

    // Test 4: Deduct coins
    qDebug() << "\n[Test 4] Deducting coins...";
    bool success = CoinSystem::instance().deductCoins(15);
    qDebug() << "Deduct 15 success:" << success;
    qDebug() << "Remaining coins:" << CoinSystem::instance().getCoins();

    // Test 5: Try to deduct more than available
    qDebug() << "\n[Test 5] Try to deduct more than available...";
    success = CoinSystem::instance().deductCoins(100);
    qDebug() << "Deduct 100 success:" << success;
    qDebug() << "Remaining coins:" << CoinSystem::instance().getCoins();

    // Test 6: Reset and reload
    qDebug() << "\n[Test 6] Reset and reload...";
    CoinSystem::instance().reset();
    CoinSystem::instance().initialize("test_user_001");
    qDebug() << "Loaded coins:" << CoinSystem::instance().getCoins();

    // Test 7: CoinDatabase
    qDebug() << "\n[Test 7] Testing CoinDatabase directly...";
    CoinDatabase::instance().saveCoinData("test_user_002", 500);
    int loaded = CoinDatabase::instance().loadCoinData("test_user_002");
    qDebug() << "Saved 500, loaded:" << loaded;

    qDebug() << "\n========================================";
    qDebug() << "All tests completed!";
    qDebug() << "========================================";

    return 0;
}
