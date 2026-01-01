# 金币系统使用说明

## 概述

金币系统允许在单人模式和旋风模式中随机生成金币宝石，当玩家消除这些宝石时会获得金币。金币数据会自动保存到本地存储，并可选择性地同步到网络服务器。

## 系统架构

### 核心组件

1. **CoinSystem** (`src/game/data/CoinSystem.h/cpp`)
   - 单例模式的金币管理核心
   - 管理当前用户的金币余额
   - 提供添加、扣除、保存、加载金币的接口

2. **CoinDatabase** (`src/game/data/CoinDatabase.h/cpp`)
   - 金币数据持久化层
   - 支持本地存储（QSettings）
   - 支持网络同步（通过回调函数）

3. **Gemstone** (`src/game/components/Gemstone.h/cpp`)
   - 添加了金币标记功能
   - 金币宝石会显示金色的旋转硬币特效
   - 支持设置金币价值（1-5）

## 使用方法

### 1. 初始化金币系统

在游戏启动或用户登录时初始化：

```cpp
#include "game/data/CoinSystem.h"

// 初始化金币系统
CoinSystem::instance().initialize("user123");

// 金币会自动从数据库加载
```

### 2. 生成金币宝石

在游戏模式中生成金币宝石：

```cpp
// SingleModeGameWidget 或 WhirlwindModeGameWidget 中
// 在reset()函数中会自动生成1-3个金币宝石
void SingleModeGameWidget::reset(int mode) {
    // ... 其他初始化代码 ...

    // 生成金币宝石 (随机1-3个)
    int coinCount = QRandomGenerator::global()->bounded(1, 4);
    generateCoinGems(coinCount);
}
```

### 3. 手动生成金币宝石

```cpp
// 生成5个金币宝石
gameWidget->generateCoinGems(5);
```

### 4. 获取金币余额

```cpp
int coins = CoinSystem::instance().getCoins();
qDebug() << "Current coins:" << coins;
```

### 5. 添加金币

```cpp
// 添加10个金币并自动保存
CoinSystem::instance().addCoins(10, true);

// 添加金币但不自动保存
CoinSystem::instance().addCoins(10, false);
// 稍后手动保存
CoinSystem::instance().saveToDatabase();
```

### 6. 扣除金币

```cpp
// 扣除50个金币
bool success = CoinSystem::instance().deductCoins(50);
if (success) {
    qDebug() << "Purchase successful!";
} else {
    qDebug() << "Insufficient coins!";
}
```

### 7. 监听金币变化

```cpp
// 连接金币变化信号
connect(&CoinSystem::instance(), &CoinSystem::coinsChanged,
        this, [](int newAmount) {
    qDebug() << "Coins changed to:" << newAmount;
    // 更新UI显示
});

// 监听金币增加事件
connect(&CoinSystem::instance(), &CoinSystem::coinsAdded,
        this, [](int amount) {
    qDebug() << "Gained" << amount << "coins!";
    // 显示特效
});
```

## 数据库集成

### 本地存储

金币数据默认使用QSettings保存在本地：
- Windows: 注册表 `HKEY_CURRENT_USER\Software\GemMatch\CoinDatabase`
- Linux/Mac: `~/.config/GemMatch/CoinDatabase.conf`

### 网络同步

如果需要将金币同步到服务器，可以设置网络回调：

```cpp
#include "game/data/CoinDatabase.h"

// 设置网络保存回调
CoinDatabase::instance().setNetworkSaveCallback(
    [](const std::string& userId, int coins) -> bool {
        // 发送HTTP请求到服务器
        // POST /api/coins/save
        // { "userId": userId, "coins": coins }

        // 返回是否成功
        return serverSaveSuccess;
    }
);

// 设置网络加载回调
CoinDatabase::instance().setNetworkLoadCallback(
    [](const std::string& userId) -> int {
        // 从服务器获取金币数据
        // GET /api/coins/load?userId=xxx

        // 返回金币数量，失败返回-1
        return serverCoins;
    }
);

// 启用网络同步
CoinDatabase::instance().setNetworkSyncEnabled(true);
```

### 使用自定义数据库回调

如果想使用自己的数据库逻辑：

```cpp
// 设置自定义保存回调
CoinSystem::instance().setDatabaseSaveCallback(
    [](const std::string& userId, int coins) {
        // 自定义保存逻辑
        myDatabase.saveUserCoins(userId, coins);
    }
);

// 设置自定义加载回调
CoinSystem::instance().setDatabaseLoadCallback(
    [](const std::string& userId) -> int {
        // 自定义加载逻辑
        return myDatabase.loadUserCoins(userId);
    }
);
```

## 游戏集成示例

### 完整的用户登录流程

```cpp
void GameWindow::onUserLogin(const std::string& userId) {
    // 1. 初始化金币系统
    CoinSystem::instance().initialize(userId);

    // 2. 连接UI更新
    connect(&CoinSystem::instance(), &CoinSystem::coinsChanged,
            this, &GameWindow::updateCoinDisplay);

    // 3. 显示当前金币
    updateCoinDisplay(CoinSystem::instance().getCoins());
}

void GameWindow::updateCoinDisplay(int coins) {
    coinLabel->setText(QString("Coins: %1").arg(coins));
}
```

### 游戏结束时保存

```cpp
void GameWindow::onGameEnd() {
    // 确保数据已保存
    CoinSystem::instance().saveToDatabase();
}
```

## 金币宝石特效

金币宝石会显示以下特效：
- **金色硬币标识**：在宝石上方悬浮的金色硬币
- **旋转动画**：硬币围绕Y轴旋转
- **浮动动画**：硬币上下浮动

金币价值范围：1-5个金币

## 调试

启用调试输出：

```cpp
// 所有金币相关操作都会输出到qDebug
// 标签格式：[CoinSystem]、[CoinDatabase]、[SingleMode]、[WhirlwindMode]
```

查看日志：
```
[CoinSystem] Initialized for user: user123
[CoinSystem] Loaded from CoinDatabase: 100 coins
[SingleMode] Generated coin gem at (3,5) with value: 3
[SingleMode] Collected coin with value: 3 Total coins: 103
[CoinSystem] Added 3 coins. Total: 103
[CoinDatabase] Saved to local storage: 103 coins for user: user123
```

## 注意事项

1. **初始化顺序**：必须先调用 `CoinSystem::instance().initialize()` 才能使用其他功能
2. **线程安全**：CoinSystem是单例，但不是线程安全的，请在主线程使用
3. **数据一致性**：金币会在每次操作后自动保存，无需手动调用save
4. **网络失败处理**：网络同步失败时，数据仍会保存在本地
5. **金币重复生成**：每次调用reset()会生成新的金币宝石，旧的会被清除

## API 参考

### CoinSystem

- `initialize(userId)` - 初始化系统
- `addCoins(amount, autoSave)` - 添加金币
- `getCoins()` - 获取金币数量
- `setCoins(amount, autoSave)` - 设置金币数量
- `deductCoins(amount)` - 扣除金币
- `saveToDatabase()` - 保存到数据库
- `loadFromDatabase()` - 从数据库加载

### CoinDatabase

- `saveCoinData(userId, amount)` - 保存金币数据
- `loadCoinData(userId)` - 加载金币数据
- `setNetworkSyncEnabled(enabled)` - 启用/禁用网络同步
- `setNetworkSaveCallback(callback)` - 设置网络保存回调
- `setNetworkLoadCallback(callback)` - 设置网络加载回调

### Gemstone

- `isCoinGem()` - 是否是金币宝石
- `setCoinGem(isCoin)` - 设置为金币宝石
- `getCoinValue()` - 获取金币价值
- `setCoinValue(value)` - 设置金币价值
