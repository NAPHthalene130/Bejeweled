#include "StoreWidget.h"
#include "../GameWindow.h"
#include "MenuWidget.h"
#include "../components/MenuButton.h"
#include "../data/CoinSystem.h"
#include "../data/ItemSystem.h"
#include "../../utils/BackgroundManager.h"
#include "../../utils/ResourceUtils.h"
#include "../data/AchievementSystem.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QShowEvent>
#include <QMessageBox>
#include <QTimer>
#include <QGraphicsOpacityEffect>

StoreWidget::StoreWidget(QWidget* parent, GameWindow* gameWindow)
    : QWidget(parent), gameWindow(gameWindow) {
    setupUI();

    // 连接金币系统的信号
    connect(&CoinSystem::instance(), &CoinSystem::coinsChanged,
            this, &StoreWidget::updateCoinDisplay);

    // 连接道具系统的信号
    connect(&ItemSystem::instance(), &ItemSystem::itemCountChanged,
            this, &StoreWidget::updateItemDisplay);
}

void StoreWidget::setupUI() {
    setMinimumSize(1280, 720);

    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(80, 40, 80, 40); // 减小垂直边距以适配720p
    mainLayout->setSpacing(20);

    // 顶部金币显示区域
    QWidget* coinWidget = new QWidget(this);
    coinWidget->setFixedHeight(100);
    coinWidget->setStyleSheet(R"(
        QWidget {
            background-color: rgba(20, 24, 40, 180);
            border: 1px solid rgba(255, 255, 255, 50);
            border-radius: 16px;
        }
    )");

    auto* coinShadow = new QGraphicsDropShadowEffect(coinWidget);
    coinShadow->setBlurRadius(30);
    coinShadow->setOffset(0, 10);
    coinShadow->setColor(QColor(0, 0, 0, 140));
    coinWidget->setGraphicsEffect(coinShadow);

    QHBoxLayout* coinLayout = new QHBoxLayout(coinWidget);
    coinLayout->setContentsMargins(40, 20, 40, 20);

    QLabel* coinIcon = new QLabel("💰", coinWidget);
    QFont iconFont = coinIcon->font();
    iconFont.setPointSize(36);
    coinIcon->setFont(iconFont);
    coinIcon->setStyleSheet("background: transparent;");

    coinLabel = new QLabel("金币: 0", coinWidget);
    QFont coinFont = coinLabel->font();
    coinFont.setFamily("Microsoft YaHei");
    coinFont.setPointSize(24);
    coinFont.setBold(true);
    coinLabel->setFont(coinFont);
    coinLabel->setStyleSheet("color: rgba(255, 220, 100, 255); background: transparent;");

    coinLayout->addWidget(coinIcon);
    coinLayout->addWidget(coinLabel);
    coinLayout->addStretch();

    // 主面板
    mainPanel = new QWidget(this);
    mainPanel->setStyleSheet(R"(
        QWidget {
            background-color: rgba(20, 24, 40, 165);
            border: 1px solid rgba(255, 255, 255, 40);
            border-radius: 22px;
        }
    )");

    auto* panelShadow = new QGraphicsDropShadowEffect(mainPanel);
    panelShadow->setBlurRadius(40);
    panelShadow->setOffset(0, 14);
    panelShadow->setColor(QColor(0, 0, 0, 140));
    mainPanel->setGraphicsEffect(panelShadow);

    QVBoxLayout* panelLayout = new QVBoxLayout(mainPanel);
    panelLayout->setContentsMargins(40, 30, 40, 30);
    panelLayout->setSpacing(20);

    // 商店标题
    QLabel* titleLabel = new QLabel("商店", mainPanel);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    titleFont.setFamily("Microsoft YaHei");
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignHCenter);
    titleLabel->setStyleSheet("color: rgba(255, 255, 255, 230); background: transparent;");

    // 道具网格
    QWidget* itemsContainer = new QWidget(mainPanel);
    itemsContainer->setStyleSheet("background: transparent;");
    QGridLayout* itemsGrid = new QGridLayout(itemsContainer);
    itemsGrid->setSpacing(20);
    itemsGrid->setContentsMargins(0, 0, 0, 0);

    // 获取所有道具
    auto allItems = ItemSystem::instance().getAllItems();

    // 按照指定顺序添加道具
    std::vector<ItemType> itemOrder = {
        ItemType::FREEZE_TIME,
        ItemType::HAMMER,
        ItemType::RESET_BOARD,
        ItemType::CLEAR_ALL
    };

    int row = 0, col = 0;
    for (ItemType type : itemOrder) {
        auto it = allItems.find(type);
        if (it != allItems.end()) {
            QWidget* card = createItemCard(it->second);
            itemsGrid->addWidget(card, row, col);

            col++;
            if (col >= 4) {  // 每行4个道具，适配16:9宽屏布局
                col = 0;
                row++;
            }
        }
    }

    // 返回按钮
    MenuButton* backButton = new MenuButton(220, 60, 18, QColor(120, 220, 255), "返回主菜单", mainPanel);
    connect(backButton, &QPushButton::clicked, this, [this]() {
        if (gameWindow) {
            auto* menuWidget = gameWindow->getMenuWidget();
            if (menuWidget) {
                gameWindow->switchWidget(menuWidget);
            }
        }
    });

    panelLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    panelLayout->addWidget(itemsContainer, 1);
    panelLayout->addWidget(backButton, 0, Qt::AlignHCenter);

    mainLayout->addWidget(coinWidget);
    mainLayout->addWidget(mainPanel, 1);

    // 初始化金币显示
    updateCoinDisplay(CoinSystem::instance().getCoins());
}

// 添加弹幕提示实现
void StoreWidget::showFloatingMessage(const QString& text, bool isSuccess) {
    // 创建提示标签
    QLabel* msgLabel = new QLabel(text, this);
    msgLabel->setStyleSheet(QString(R"(
        QLabel {
            background-color: %1;
            color: white;
            padding: 12px 24px;
            border-radius: 20px;
            font-family: "Microsoft YaHei";
            font-size: 16px;
            font-weight: bold;
        }
    )").arg(isSuccess ? "rgba(70, 180, 70, 200)" : "rgba(220, 80, 80, 200)"));

    // 设置阴影效果

    // 设置位置（屏幕顶部居中）
    msgLabel->adjustSize();
    int x = (width() - msgLabel->width()) / 2;
    int y = 50; // 距离顶部50像素
    msgLabel->setGeometry(x, y, msgLabel->width(), msgLabel->height());
    msgLabel->show();

    // 2秒后开始淡出并销毁
    QTimer::singleShot(2000, this, [this, msgLabel]() {
        // 创建淡出动画
        auto* opacityEffect = new QGraphicsOpacityEffect(msgLabel);
        msgLabel->setGraphicsEffect(opacityEffect);
        auto* animation = new QPropertyAnimation(opacityEffect, "opacity");
        animation->setDuration(500);
        animation->setStartValue(1.0);
        animation->setEndValue(0.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);

        // 动画结束后删除标签
        connect(animation, &QPropertyAnimation::finished, 
                this, [this, msgLabel]() { removeFloatingMessage(msgLabel); });
    });
}

// 辅助函数：安全删除标签
void StoreWidget::removeFloatingMessage(QLabel* label) {
    if (label && label->parent() == this) {
        label->deleteLater();
    }
}

void StoreWidget::updateCoinDisplay(int newCoins) {
    if (coinLabel) {
        coinLabel->setText(QString("金币: %1").arg(newCoins));
        AchievementSystem::instance().triggerCoinEarned(newCoins);
    }
}

void StoreWidget::updateItemDisplay(ItemType type, int newCount) {
    auto it = itemCountLabels.find(type);
    if (it != itemCountLabels.end() && it->second) {
        it->second->setText(QString("拥有: %1").arg(newCount));
    }
}

QWidget* StoreWidget::createItemCard(const ItemInfo& info) {
    QWidget* card = new QWidget(mainPanel);
    // 设置大小策略为Expanding，使其能够随窗口调整大小
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // 设置最小尺寸以保证内容不被挤压，宽度减小以适配4列布局
    card->setMinimumSize(220, 260); 
    card->setStyleSheet(R"(
        QWidget {
            background-color: rgba(40, 50, 80, 180);
            border: 2px solid rgba(100, 150, 255, 100);
            border-radius: 18px;
        }
    )");

    auto* cardShadow = new QGraphicsDropShadowEffect(card);
    cardShadow->setBlurRadius(20);
    cardShadow->setOffset(0, 8);
    cardShadow->setColor(QColor(0, 0, 0, 100));
    card->setGraphicsEffect(cardShadow);

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(16, 16, 16, 16);
    cardLayout->setSpacing(10);

    // 道具图标
    QLabel* iconLabel = new QLabel(card);
    QPixmap iconPixmap(QString::fromStdString(info.icon));
    if (!iconPixmap.isNull()) {
        iconLabel->setPixmap(iconPixmap.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    iconLabel->setAlignment(Qt::AlignHCenter);
    iconLabel->setStyleSheet("background: transparent; border: none;");

    // 道具名称
    QLabel* nameLabel = new QLabel(QString::fromStdString(info.name), card);
    QFont nameFont = nameLabel->font();
    nameFont.setFamily("Microsoft YaHei");
    nameFont.setPointSize(18);
    nameFont.setBold(true);
    nameLabel->setFont(nameFont);
    nameLabel->setAlignment(Qt::AlignHCenter);
    nameLabel->setStyleSheet("color: rgba(255, 255, 255, 230); background: transparent; border: none;");

    // 道具描述
    QLabel* descLabel = new QLabel(QString::fromStdString(info.description), card);
    QFont descFont = descLabel->font();
    descFont.setFamily("Microsoft YaHei");
    descFont.setPointSize(12);
    descLabel->setFont(descFont);
    descLabel->setAlignment(Qt::AlignHCenter);
    descLabel->setStyleSheet("color: rgba(200, 200, 200, 200); background: transparent; border: none;");
    descLabel->setWordWrap(true);

    // 拥有数量
    QLabel* countLabel = new QLabel(QString("拥有: %1").arg(ItemSystem::instance().getItemCount(info.type)), card);
    QFont countFont = countLabel->font();
    countFont.setFamily("Microsoft YaHei");
    countFont.setPointSize(13);
    countLabel->setFont(countFont);
    countLabel->setAlignment(Qt::AlignHCenter);
    countLabel->setStyleSheet("color: rgba(150, 255, 150, 230); background: transparent; border: none;");
    itemCountLabels[info.type] = countLabel;

    // 购买按钮
    QPushButton* buyButton = new QPushButton(QString("💰 %1 金币").arg(info.price), card);
    QFont btnFont = buyButton->font();
    btnFont.setFamily("Microsoft YaHei");
    btnFont.setPointSize(14);
    btnFont.setBold(true);
    buyButton->setFont(btnFont);
    buyButton->setFixedHeight(40);
    buyButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(80, 180, 255, 220),
                stop:1 rgba(50, 120, 200, 220));
            color: white;
            border: none;
            border-radius: 8px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(100, 200, 255, 240),
                stop:1 rgba(70, 140, 220, 240));
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(60, 160, 235, 200),
                stop:1 rgba(40, 100, 180, 200));
        }
    )");

    connect(buyButton, &QPushButton::clicked, this, [this, info]() {
        onPurchaseClicked(info.type);
    });

    cardLayout->addWidget(iconLabel);
    cardLayout->addWidget(nameLabel);
    cardLayout->addWidget(descLabel);
    cardLayout->addWidget(countLabel);
    cardLayout->addWidget(buyButton);

    return card;
}

void StoreWidget::onPurchaseClicked(ItemType type) {
    // 购买道具
    if (ItemSystem::instance().purchaseItem(type)) {
        // 购买成功 - 显示成功弹幕
        ItemInfo info = ItemSystem::instance().getItemInfo(type);
        showFloatingMessage(QString("成功购买 %1！").arg(QString::fromStdString(info.name)), true);
    } else {
        // 购买失败 - 显示失败弹幕
        ItemInfo info = ItemSystem::instance().getItemInfo(type);
        showFloatingMessage(QString("金币不足！购买 %1 需要 %2 金币。")
            .arg(QString::fromStdString(info.name))
            .arg(info.price), false);
    }
}

void StoreWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    // 每次显示时刷新金币数
    updateCoinDisplay(CoinSystem::instance().getCoins());
}
