#include "StoreWidget.h"
#include "../GameWindow.h"
#include "MenuWidget.h"
#include "../components/MenuButton.h"
#include "../data/CoinSystem.h"
#include "../../utils/BackgroundManager.h"
#include "../../utils/ResourceUtils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QShowEvent>

StoreWidget::StoreWidget(QWidget* parent, GameWindow* gameWindow)
    : QWidget(parent), gameWindow(gameWindow) {
    setupUI();

    // 连接金币系统的信号
    connect(&CoinSystem::instance(), &CoinSystem::coinsChanged,
            this, &StoreWidget::updateCoinDisplay);
}

void StoreWidget::setupUI() {
    setMinimumSize(1600, 1000);

    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(80, 60, 80, 60);
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
    panelLayout->setContentsMargins(56, 44, 56, 44);
    panelLayout->setSpacing(28);

    // 商店标题
    QLabel* titleLabel = new QLabel("商店", mainPanel);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    titleFont.setFamily("Microsoft YaHei");
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignHCenter);
    titleLabel->setStyleSheet("color: rgba(255, 255, 255, 230); background: transparent;");

    // 商品区域（占位）
    QLabel* comingSoonLabel = new QLabel("商品即将上线...", mainPanel);
    QFont comingSoonFont = comingSoonLabel->font();
    comingSoonFont.setPointSize(18);
    comingSoonFont.setFamily("Microsoft YaHei");
    comingSoonLabel->setFont(comingSoonFont);
    comingSoonLabel->setAlignment(Qt::AlignHCenter);
    comingSoonLabel->setStyleSheet("color: rgba(255, 255, 255, 180); background: transparent;");

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
    panelLayout->addStretch(1);
    panelLayout->addWidget(comingSoonLabel, 0, Qt::AlignHCenter);
    panelLayout->addStretch(2);
    panelLayout->addWidget(backButton, 0, Qt::AlignHCenter);

    mainLayout->addWidget(coinWidget);
    mainLayout->addWidget(mainPanel, 1);

    // 初始化金币显示
    updateCoinDisplay(CoinSystem::instance().getCoins());
}

void StoreWidget::updateCoinDisplay(int newCoins) {
    if (coinLabel) {
        coinLabel->setText(QString("金币: %1").arg(newCoins));
    }
}

void StoreWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    // 每次显示时刷新金币数
    updateCoinDisplay(CoinSystem::instance().getCoins());
}
