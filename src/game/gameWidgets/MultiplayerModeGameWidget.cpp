#include "MultiplayerModeGameWidget.h"
#include "../data/NetDataIO.h"
#include "FinalWidget.h"
#include "MenuWidget.h"
#include "../GameWindow.h"
#include "../components/Gemstone.h"
#include "../components/SelectedCircle.h"
#include "../data/GameNetData.h"
#include "../../utils/LogWindow.h"
#include "../../utils/AudioManager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QDialog>
#include <QPropertyAnimation>
#include <QPointer>
#include <QParallelAnimationGroup>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>
#include <Qt3DCore/QTransform>
#include <QRandomGenerator>
#include <QVector3D>
#include <QMouseEvent>
#include <QTextEdit>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QHideEvent>
#include <QPainter>
#include <QPainterPath>
#include <QDateTime>
#include <QApplication>

#include <json.hpp>
#include <cmath>
#include <limits>
#include <iostream>
#include <queue>
#include <set>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class GameBackDialog : public QDialog {
public:
    explicit GameBackDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        setAttribute(Qt::WA_TranslucentBackground);
        setModal(true);
        setFixedSize(420, 240);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(26, 24, 26, 22);
        layout->setSpacing(14);

        auto* titleLabel = new QLabel("提示", this);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(16);
        titleFont.setBold(true);
        titleFont.setFamily("Microsoft YaHei");
        titleLabel->setFont(titleFont);
        titleLabel->setAlignment(Qt::AlignHCenter);
        titleLabel->setStyleSheet("color: rgba(255,255,255,235); background: transparent;");

        auto* contentLabel = new QLabel("是否返回至主菜单", this);
        QFont contentFont = contentLabel->font();
        contentFont.setPointSize(12);
        contentFont.setFamily("Microsoft YaHei");
        contentLabel->setFont(contentFont);
        contentLabel->setAlignment(Qt::AlignHCenter);
        contentLabel->setWordWrap(true);
        contentLabel->setStyleSheet("color: rgba(255,255,255,210); background: transparent;");

        auto* btnRow = new QHBoxLayout();
        btnRow->setSpacing(14);

        auto* yesBtn = new QPushButton("是", this);
        auto* noBtn = new QPushButton("否", this);

        QSize btnSize(150, 44);
        yesBtn->setFixedSize(btnSize);
        noBtn->setFixedSize(btnSize);
        yesBtn->setCursor(Qt::PointingHandCursor);
        noBtn->setCursor(Qt::PointingHandCursor);

        yesBtn->setStyleSheet(R"(
            QPushButton {
                color: white;
                border-radius: 12px;
                border: 1px solid rgba(255,255,255,60);
                background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(80,180,255,220), stop:1 rgba(80,120,255,220));
                font-family: 'Microsoft YaHei';
                font-size: 14px;
                font-weight: bold;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(100,200,255,240), stop:1 rgba(95,140,255,240));
            }
            QPushButton:pressed {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(70,160,220,240), stop:1 rgba(70,110,220,240));
            }
        )");

        noBtn->setStyleSheet(R"(
            QPushButton {
                color: rgba(255,255,255,235);
                border-radius: 12px;
                border: 1px solid rgba(255,255,255,60);
                background: rgba(255,255,255,22);
                font-family: 'Microsoft YaHei';
                font-size: 14px;
                font-weight: bold;
            }
            QPushButton:hover {
                background: rgba(255,255,255,30);
            }
            QPushButton:pressed {
                background: rgba(255,255,255,18);
            }
        )");

        connect(yesBtn, &QPushButton::clicked, this, &QDialog::accept);
        connect(noBtn, &QPushButton::clicked, this, &QDialog::reject);

        btnRow->addStretch(1);
        btnRow->addWidget(noBtn);
        btnRow->addWidget(yesBtn);
        btnRow->addStretch(1);

        layout->addWidget(titleLabel);
        layout->addWidget(contentLabel);
        layout->addStretch(1);
        layout->addLayout(btnRow);
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QRect r = rect().adjusted(8, 8, -8, -8);
        QPainterPath path;
        path.addRoundedRect(r, 18, 18);

        QLinearGradient bg(r.topLeft(), r.bottomRight());
        bg.setColorAt(0, QColor(25, 30, 45, 235));
        bg.setColorAt(1, QColor(15, 18, 30, 235));
        painter.fillPath(path, bg);

        QPen pen(QColor(255, 255, 255, 45));
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawPath(path);
    }
};

MultiplayerModeGameWidget::MultiplayerModeGameWidget(QWidget* parent, GameWindow* gameWindow, const std::string& userId)
    : QWidget(parent), gameWindow(gameWindow), canOpe(false), nowTimeHave(0), mode(1),
      firstSelectedGemstone(nullptr), secondSelectedGemstone(nullptr), selectedNum(0),
      myUserId(userId), isStop(false) {
    
    // 初始化定时器
    timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (isFinishing) return;
        nowTimeHave = gameTimeKeeper.totalSeconds();
        updateTimeBoard();
    });
    
    // 设置主背景颜色
    setStyleSheet("background-color: rgb(40, 40, 45);");

    // 初始化3D窗口
    game3dWindow = new Qt3DExtras::Qt3DWindow();
    game3dWindow->defaultFrameGraph()->setClearColor(QColor(40, 40, 45)); // 深灰色背景
    
    // 不重复注册 InputAspect，Qt3DWindow 已默认注册

    // 设置3D场景
    setup3DScene();
    
    // 创建3D窗口容器
    container3d = QWidget::createWindowContainer(game3dWindow);
    // container3d->setFixedSize(1000, 1000); // 移除固定大小
    container3d->setMinimumSize(600, 600); // 设置最小大小
    container3d->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container3d->setFocusPolicy(Qt::StrongFocus);
    container3d->setMouseTracking(true); // 启用鼠标追踪
    container3d->setAttribute(Qt::WA_Hover, true); // 启用hover事件
    
    // 布局 - 左侧居中
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(50, 0, 50, 0); // 添加一些边距
    
    // 将容器对齐到左侧，垂直居中
    mainLayout->addWidget(container3d, 1); // 这里的1表示拉伸因子
    
    // 中间布局用于放置状态信息
    QVBoxLayout* centerLayout = new QVBoxLayout();
    centerLayout->setContentsMargins(0, 50, 0, 50);
    mainLayout->addLayout(centerLayout);

    // 信息卡片
    auto* infoCard = new QWidget(this);
    infoCard->setFixedWidth(300);
    infoCard->setStyleSheet(R"(
        QWidget {
            background-color: rgba(255, 255, 255, 18);
            border: 1px solid rgba(255, 255, 255, 40);
            border-radius: 16px;
        }
    )");
    auto* infoShadow = new QGraphicsDropShadowEffect(infoCard);
    infoShadow->setBlurRadius(18);
    infoShadow->setOffset(0, 7);
    infoShadow->setColor(QColor(0, 0, 0, 120));
    infoCard->setGraphicsEffect(infoShadow);

    auto* infoLayout = new QVBoxLayout(infoCard);
    infoLayout->setContentsMargins(18, 16, 18, 16);
    infoLayout->setSpacing(10);

    scoreBoardLabel = new QLabel(infoCard);
    QFont scoreFont = scoreBoardLabel->font();
    scoreFont.setFamily("Microsoft YaHei");
    scoreFont.setPointSize(16);
    scoreFont.setBold(true);
    scoreBoardLabel->setFont(scoreFont);
    scoreBoardLabel->setStyleSheet("color: rgba(255,255,255,235); background: transparent;");

    timeBoardLabel = new QLabel(infoCard);
    QFont timeFont = timeBoardLabel->font();
    timeFont.setFamily("Microsoft YaHei");
    timeFont.setPointSize(12);
    timeFont.setBold(true);
    timeBoardLabel->setFont(timeFont);
    timeBoardLabel->setStyleSheet("color: rgba(255,255,255,200); background: transparent;");

    // Connection status label
    connectionStatusLabel = new QLabel(infoCard);
    QFont statusFont = connectionStatusLabel->font();
    statusFont.setFamily("Microsoft YaHei");
    statusFont.setPointSize(11);
    connectionStatusLabel->setFont(statusFont);
    connectionStatusLabel->setStyleSheet("color: rgba(255,200,100,220); background: transparent;");
    connectionStatusLabel->setText("连接中...");

    // Waiting for players label
    waitingLabel = new QLabel(infoCard);
    QFont waitingFont = waitingLabel->font();
    waitingFont.setFamily("Microsoft YaHei");
    waitingFont.setPointSize(11);
    waitingLabel->setFont(waitingFont);
    waitingLabel->setStyleSheet("color: rgba(255,220,120,220); background: transparent;");
    waitingLabel->setText("等待其他玩家...");
    waitingLabel->setVisible(false);

    infoLayout->addWidget(scoreBoardLabel);
    infoLayout->addWidget(timeBoardLabel);
    infoLayout->addWidget(connectionStatusLabel);
    infoLayout->addWidget(waitingLabel);
    centerLayout->addWidget(infoCard, 0, Qt::AlignTop | Qt::AlignHCenter);
    
    // 返回按钮
    backToMenuButton = new QPushButton("返回菜单", this);
    backToMenuButton->setFixedSize(180, 54);
    backToMenuButton->setCursor(Qt::PointingHandCursor);
    backToMenuButton->setStyleSheet(R"(
        QPushButton {
            color: white;
            border-radius: 14px;
            border: 1px solid rgba(255,255,255,55);
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(255,140,120,220), stop:1 rgba(255,95,120,220));
            font-family: 'Microsoft YaHei';
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(255,160,130,235), stop:1 rgba(255,115,140,235));
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(220,110,100,235), stop:1 rgba(220,75,95,235));
        }
    )");

    auto* backShadow = new QGraphicsDropShadowEffect(backToMenuButton);
    backShadow->setBlurRadius(18);
    backShadow->setOffset(0, 8);
    backShadow->setColor(QColor(0, 0, 0, 120));
    backToMenuButton->setGraphicsEffect(backShadow);
    centerLayout->addStretch(1);
    centerLayout->addWidget(backToMenuButton, 0, Qt::AlignBottom | Qt::AlignHCenter);

    // mainLayout->addStretch(1); // 移除多余的stretch，让左侧3D窗口占满剩余空间
    
    rightPanel = new QWidget(this);
    rightPanel->setFixedWidth(450); 
    rightPanel->setStyleSheet(R"(
        QWidget {
            background-color: rgba(12, 14, 24, 120);
            border: 1px solid rgba(255, 255, 255, 35);
            border-radius: 18px;
        }
    )");
    auto* panelShadow = new QGraphicsDropShadowEffect(rightPanel);
    panelShadow->setBlurRadius(26);
    panelShadow->setOffset(0, 10);
    panelShadow->setColor(QColor(0, 0, 0, 140));
    rightPanel->setGraphicsEffect(panelShadow);

    auto* panelLayout = new QVBoxLayout(rightPanel);
    panelLayout->setContentsMargins(18, 18, 18, 18);
    panelLayout->setSpacing(16);

    // Other players' board panel - 使用网格布局以支持两个并排的小棋盘
    otherPlayersPanelWidget = new QWidget(rightPanel);
    otherPlayersPanelWidget->setStyleSheet("QWidget { background: transparent; }");
    otherPlayersPanelLayout = new QVBoxLayout(otherPlayersPanelWidget);
    otherPlayersPanelLayout->setContentsMargins(0, 10, 0, 10);
    otherPlayersPanelLayout->setSpacing(12);
    panelLayout->addWidget(otherPlayersPanelWidget, 1, Qt::AlignCenter); 

    // Initialize Player 1 Window (Top)
    player1ScoreLabel = new QLabel("玩家1: 0分", rightPanel);
    player1ScoreLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold; background: transparent;");
    otherPlayersPanelLayout->addWidget(player1ScoreLabel, 0, Qt::AlignHCenter);

    player1Window = new Qt3DExtras::Qt3DWindow();
    setupSmall3DWindow(player1Window, &player1RootEntity, &player1Camera);
    player1Container = QWidget::createWindowContainer(player1Window);
    // player1Container->setFixedSize(400, 400); // 移除固定大小
    player1Container->setMinimumSize(200, 200); // 设置最小大小
    player1Container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    otherPlayersPanelLayout->addWidget(player1Container, 1); // 自适应

    // Initialize Player 2 Window (Bottom)
    player2ScoreLabel = new QLabel("玩家2: 0分", rightPanel);
    player2ScoreLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold; background: transparent;");
    otherPlayersPanelLayout->addWidget(player2ScoreLabel, 0, Qt::AlignHCenter);

    player2Window = new Qt3DExtras::Qt3DWindow();
    setupSmall3DWindow(player2Window, &player2RootEntity, &player2Camera);
    player2Container = QWidget::createWindowContainer(player2Window);
    // player2Container->setFixedSize(400, 400); // 移除固定大小
    player2Container->setMinimumSize(200, 200); // 设置最小大小
    player2Container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    otherPlayersPanelLayout->addWidget(player2Container, 1); // 自适应


    panelLayout->addStretch(0);  // 去掉多余的stretch，让棋盘区域占用更多空间

    mainLayout->addWidget(rightPanel, 0, Qt::AlignRight | Qt::AlignVCenter);


    connect(backToMenuButton, &QPushButton::clicked, this, [this]() {
        GameBackDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            if (timer && timer->isActive()) timer->stop();
            gameTimeKeeper.pause();
            if (inactivityTimer) inactivityTimer->stop();
            if (this->gameWindow && this->gameWindow->getMenuWidget()) {
                this->gameWindow->switchWidget(this->gameWindow->getMenuWidget());
            }
        }
    });

    panelLayout->addWidget(backToMenuButton, 0, Qt::AlignRight | Qt::AlignBottom);

    focusInfoLabel = new QLabel(rightPanel);
    focusInfoLabel->setVisible(false);
    debugText = new QTextEdit(rightPanel);
    debugText->setVisible(false);
    debugText->setReadOnly(true);
    debugTimer = new QTimer(this);

    setLayout(mainLayout);
    // mainLayout->addWidget(rightPanel, 0, Qt::AlignRight | Qt::AlignVCenter); // 移除重复添加
    container3d->installEventFilter(this);
    game3dWindow->installEventFilter(this); // 关键：在3D窗口上安装事件过滤器

    // 初始化无操作计时器
    inactivityTimer = new QTimer(this);
    inactivityTimer->setInterval(inactivityTimeout);
    inactivityTimer->setSingleShot(true);
    
    // 超时后高亮可消除的宝石
    connect(inactivityTimer, &QTimer::timeout, this, &MultiplayerModeGameWidget::highlightMatches);

    // 任何用户操作后重置计时器
    connect(this, &MultiplayerModeGameWidget::userActionOccurred, [this]() {
        resetInactivityTimer();
    });

    inactivityTimer->stop();

    updateScoreBoard();
    updateTimeBoard();
    appendDebug("MultiplayerModeGameWidget initialized - EventFilter installed on both container and 3D window");

    // Initialize sync timer for periodic board synchronization (every 5 seconds)
    syncTimer = new QTimer(this);
    // syncTimer->setInterval(5000);  // 5 seconds
    // connect(syncTimer, &QTimer::timeout, this, &MultiplayerModeGameWidget::sendBoardSyncMessage);
}

void MultiplayerModeGameWidget::setStop(bool stop) {
    isStop = stop;
    if (isStop) {
        if (timer && timer->isActive()) timer->stop();
        gameTimeKeeper.pause();
        if (inactivityTimer) inactivityTimer->stop();
        if (syncTimer && syncTimer->isActive()) syncTimer->stop();
        canOpe = false; // Disable operation
    }
}


void MultiplayerModeGameWidget::GameTimeKeeper::reset() {
    accumulatedMs = 0;
    if (isRunning) {
        timer.restart();
    }
}

void MultiplayerModeGameWidget::GameTimeKeeper::start() {
    if (!isRunning) {
        timer.start();
        isRunning = true;
    }
}

void MultiplayerModeGameWidget::GameTimeKeeper::pause() {
    if (isRunning) {
        accumulatedMs += timer.elapsed();
        isRunning = false;
    }
}

int MultiplayerModeGameWidget::GameTimeKeeper::totalSeconds() const {
    qint64 total = accumulatedMs;
    if (isRunning) {
        total += timer.elapsed();
    }
    return total / 1000;
}

QString MultiplayerModeGameWidget::GameTimeKeeper::displayText() const {
    int total = totalSeconds();
    int m = total / 60;
    int s = total % 60;
    return QString("游戏进行时间：%1:%2")
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}

void MultiplayerModeGameWidget::updateScoreBoard() {
    if (!scoreBoardLabel) return;
    scoreBoardLabel->setText(QString("当前分数：%1").arg(gameScore));
}

void MultiplayerModeGameWidget::updateTimeBoard() {
    if (!timeBoardLabel) return;
    timeBoardLabel->setText(gameTimeKeeper.displayText());
}

void MultiplayerModeGameWidget::triggerFinishIfNeeded() {
    if (isFinishing) return;
    if (gameScore < targetScore) return;
    finishToFinalWidget();
}

void MultiplayerModeGameWidget::finishToFinalWidget() {
    if (isFinishing) return;
    isFinishing = true;
    canOpe = false;

    if (timer && timer->isActive()) timer->stop();
    gameTimeKeeper.pause();
    if (inactivityTimer) inactivityTimer->stop();
    clearHighlights();
    if (selectionRing1) selectionRing1->setVisible(false);
    if (selectionRing2) selectionRing2->setVisible(false);

    int total = gameTimeKeeper.totalSeconds();
    int m = total / 60;
    int s = total % 60;
    QString timeText = QString("%1:%2")
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));

    QString gradeText = QString("本局得分：%1\n用时：%2\n评价：Excellent!")
        .arg(gameScore)
        .arg(timeText);

    QTimer::singleShot(650, this, [this, gradeText]() {
        if (!gameWindow) return;
        auto* finalWidget = gameWindow->getFinalWidget();
        if (!finalWidget) return;
        finalWidget->setGradeContent(gradeText.toStdString());
        gameWindow->switchWidget(finalWidget);
    });
}

// 查找所有需要消除的宝石（三连或更多）
std::vector<std::pair<int, int>> MultiplayerModeGameWidget::findMatches() {
    std::vector<std::pair<int, int>> matches;
    std::vector<std::vector<bool>> marked(8, std::vector<bool>(8, false));

    // 检查水平方向
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 6; ++j) {  // 最多检查到j=5，这样j+2不会越界
            Gemstone* gem1 = gemstoneContainer[i][j];
            Gemstone* gem2 = gemstoneContainer[i][j+1];
            Gemstone* gem3 = gemstoneContainer[i][j+2];

            if (gem1 && gem2 && gem3 &&
                gem1->getType() == gem2->getType() &&
                gem2->getType() == gem3->getType()) {

                // 标记这三个位置
                marked[i][j] = true;
                marked[i][j+1] = true;
                marked[i][j+2] = true;

                // 继续检查是否有更多连续的
                int k = j + 3;
                while (k < 8 && gemstoneContainer[i][k] &&
                       gemstoneContainer[i][k]->getType() == gem1->getType()) {
                    marked[i][k] = true;
                    k++;
                }
            }
        }
    }

    // 检查垂直方向
    for (int j = 0; j < 8; ++j) {
        for (int i = 0; i < 6; ++i) {  // 最多检查到i=5
            Gemstone* gem1 = gemstoneContainer[i][j];
            Gemstone* gem2 = gemstoneContainer[i+1][j];
            Gemstone* gem3 = gemstoneContainer[i+2][j];

            if (gem1 && gem2 && gem3 &&
                gem1->getType() == gem2->getType() &&
                gem2->getType() == gem3->getType()) {

                // 标记这三个位置
                marked[i][j] = true;
                marked[i+1][j] = true;
                marked[i+2][j] = true;

                // 继续检查是否有更多连续的
                int k = i + 3;
                while (k < 8 && gemstoneContainer[k][j] &&
                       gemstoneContainer[k][j]->getType() == gem1->getType()) {
                    marked[k][j] = true;
                    k++;
                }
            }
        }
    }

    // 收集所有被标记的位置
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (marked[i][j]) {
                matches.push_back({i, j});
            }
        }
    }

    return matches;
}


void MultiplayerModeGameWidget::removeMatches(const std::vector<std::pair<int, int>>& matches) {
    if (matches.empty()) {
        appendDebug("No matches to remove");
        return;
    }

    appendDebug(QString("Removing %1 gemstones").arg(matches.size()));

    // 将匹配分组
    auto groups = groupMatches(matches);
    
    int removedCount = 0;
    
    for (const auto& group : groups) {
        // 检查是否包含特殊宝石
        bool hasSpecial = hasSpecialGem(group);
        
        if (hasSpecial) {
            // 【修复】收集所有需要触发的特殊宝石位置
            std::vector<std::pair<int, int>> specialPositions;
            for (const auto& pos : group) {
                Gemstone* gem = gemstoneContainer[pos.first][pos.second];
                if (gem && gem->isSpecial()) {
                    specialPositions.push_back(pos);
                }
            }
            
            // 【修复】先消除组内的非特殊宝石
            for (const auto& pos : group) {
                int row = pos.first;
                int col = pos.second;
                Gemstone* gem = gemstoneContainer[row][col];
                
                if (gem && !gem->isSpecial()) {
                    removedCount++;
                    eliminateAnime(gem);
                    gemstoneContainer[row][col] = nullptr;
                }
            }
            
            // 【修复】然后触发所有特殊宝石（支持连锁）
            for (const auto& specialPos : specialPositions) {
                Gemstone* specialGem = gemstoneContainer[specialPos.first][specialPos.second];
                if (specialGem && specialGem->isSpecial()) {
                    remove3x3Area(specialPos.first, specialPos.second);
                }
            }
        } else if (group.size() >= 4) {
            // 【修复】4连或更多：保留第2颗宝石作为特殊宝石
            appendDebug(QString("Found %1-match, creating special gem").arg(group.size()));
            
            // 对组内位置排序（按行优先，然后列）
            std::vector<std::pair<int, int>> sortedGroup = group;
            std::sort(sortedGroup.begin(), sortedGroup.end());
            
            // 保留第2颗（索引1）作为特殊宝石
            std::pair<int, int> specialPos = sortedGroup[1];
            
            for (const auto& pos : sortedGroup) {
                int row = pos.first;
                int col = pos.second;
                Gemstone* gem = gemstoneContainer[row][col];
                
                if (gem) {
                    if (pos == specialPos) {
                        // 保留并设为特殊宝石
                        gem->setSpecial(true);
                        appendDebug(QString("Special gem created at (%1,%2)").arg(row).arg(col));
                    } else {
                        // 移除其他宝石
                        removedCount++;
                        eliminateAnime(gem);
                        gemstoneContainer[row][col] = nullptr;
                    }
                }
            }
        } else {
            // 普通3连：正常消除
            for (const auto& pos : group) {
                int row = pos.first;
                int col = pos.second;
                Gemstone* gem = gemstoneContainer[row][col];
                
                if (gem) {
                    removedCount++;
                    eliminateAnime(gem);
                    gemstoneContainer[row][col] = nullptr;
                }
            }
        }
    }

    if (removedCount > 0) {
        comboCount++;
        int comboBonus = comboCount > 1 ? (comboCount - 1) * 5 : 0;
        gameScore += removedCount * 10 + comboBonus;
        updateScoreBoard();
        triggerFinishIfNeeded();
    }
}

// ============================================================================
// 替换原有的 remove3x3Area 函数
// ============================================================================

void MultiplayerModeGameWidget::remove3x3Area(int centerRow, int centerCol) {
    appendDebug(QString("Removing 3x3 area centered at (%1,%2)").arg(centerRow).arg(centerCol));
    
    // 【修复】收集范围内的特殊宝石，用于连锁触发
    std::vector<std::pair<int, int>> chainSpecialGems;
    
    // 消除以(centerRow, centerCol)为中心的3×3区域
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            int r = centerRow + dr;
            int c = centerCol + dc;
            
            // 检查边界
            if (r < 0 || r >= 8 || c < 0 || c >= 8) {
                continue;
            }
            
            Gemstone* gem = gemstoneContainer[r][c];
            if (gem) {
                // 【修复】检查是否是另一个特殊宝石（不是中心的那个）
                if (gem->isSpecial() && !(r == centerRow && c == centerCol)) {
                    // 记录位置，稍后触发连锁
                    chainSpecialGems.push_back({r, c});
                    appendDebug(QString("Found chain special gem at (%1,%2)").arg(r).arg(c));
                }
                
                // 消除宝石
                eliminateAnime(gem);
                gemstoneContainer[r][c] = nullptr;
                
                // 增加分数
                gameScore += 10;
            }
        }
    }
    
    updateScoreBoard();
    
    // 【修复】递归触发范围内的其他特殊宝石
    for (const auto& pos : chainSpecialGems) {
        appendDebug(QString("Chain triggering at (%1,%2)").arg(pos.first).arg(pos.second));
        remove3x3AreaChain(pos.first, pos.second);
    }
}

// ============================================================================
// 新增函数 - 添加到 MultiplayerModeGameWidget.cpp 中
// ============================================================================

void MultiplayerModeGameWidget::remove3x3AreaChain(int centerRow, int centerCol) {
    appendDebug(QString("Chain removing 3x3 area at (%1,%2)").arg(centerRow).arg(centerCol));
    
    // 收集范围内的特殊宝石
    std::vector<std::pair<int, int>> chainSpecialGems;
    
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            int r = centerRow + dr;
            int c = centerCol + dc;
            
            if (r < 0 || r >= 8 || c < 0 || c >= 8) {
                continue;
            }
            
            Gemstone* gem = gemstoneContainer[r][c];
            if (gem) {
                // 检查是否是特殊宝石
                if (gem->isSpecial()) {
                    chainSpecialGems.push_back({r, c});
                    appendDebug(QString("Chain found special gem at (%1,%2)").arg(r).arg(c));
                }
                
                eliminateAnime(gem);
                gemstoneContainer[r][c] = nullptr;
                gameScore += 10;
            }
        }
    }
    
    updateScoreBoard();
    
    // 递归触发连锁
    for (const auto& pos : chainSpecialGems) {
        remove3x3AreaChain(pos.first, pos.second);
    }
}




void MultiplayerModeGameWidget::eliminate() {
    if (isStop) return;
    if (isFinishing) return;
    // 查找所有匹配
    std::vector<std::pair<int, int>> matches = findMatches();
    if (!matches.empty()) {
        // //DEBUG
        // QDialog* dialog = new QDialog(this);
        // dialog->setWindowTitle("匹配消除");
        // dialog->setModal(true);
        // dialog->exec();
        // //DEBUG

        comboCount++; // 增加连续消除计数
        appendDebug(QString("Found %1 matches to eliminate").arg(matches.size()));
        AudioManager::instance().playEliminateSound(comboCount);


        // 禁止操作
        canOpe = false;
        
        // 移除匹配的宝石
        removeMatches(matches);

        if (isFinishing) return;

        // 等待消除动画完成后执行下落（500ms）
        QTimer::singleShot(600, this, [this]() {
            drop();
        });
    } else {
        comboCount = 0;
        // 没有匹配了，恢复操作
        canOpe = true;
        resetInactivityTimer();

        // 消除步骤结束，发送最终棋盘状态 (Type=4)
        sendNowBoard();
        
        appendDebug("No matches found, game can continue");
    }
}

void MultiplayerModeGameWidget::drop() {
    if (isStop) return;
    if (isFinishing) return;
    appendDebug("Starting drop animation");

    QParallelAnimationGroup* dropAnimGroup = new QParallelAnimationGroup();
    bool hasDrops = false;

    for (int col = 0; col < 8; ++col) {
        int writePos = 7; // 从底部开始写入
        for (int row = 7; row >= 0; --row) {
            if (gemstoneContainer[row][col] != nullptr) {
                if (row < writePos) {
                    Gemstone* gem = gemstoneContainer[row][col];
                    gemstoneContainer[writePos][col] = gem;
                    gemstoneContainer[row][col] = nullptr;

                    QVector3D targetPos = getPosition(writePos, col);
                    QPropertyAnimation* dropAnim = new QPropertyAnimation(gem->transform(), "translation");
                    dropAnim->setDuration(500);
                    dropAnim->setStartValue(gem->transform()->translation());
                    dropAnim->setEndValue(targetPos);
                    dropAnimGroup->addAnimation(dropAnim);

                    hasDrops = true;
                }
                writePos--;
            }
        }
    }

    if (hasDrops) {
        appendDebug("Drop animation started");
        connect(dropAnimGroup, &QParallelAnimationGroup::finished, this, [this]() {
            if (isFinishing) return;
            appendDebug("Drop animation finished, filling new gemstones");
            resetGemstoneTable();
            resetInactivityTimer();
        });
        dropAnimGroup->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        appendDebug("No drops needed, filling new gemstones");
        resetGemstoneTable();
        resetInactivityTimer();
    }
}

void MultiplayerModeGameWidget::resetGemstoneTable() {
    if (isStop) return;
    if (isFinishing) return;
    appendDebug("Filling empty positions with new gemstones");

    QParallelAnimationGroup* fillAnimGroup = new QParallelAnimationGroup(this);
    bool hasFills = false;
    std::vector<std::pair<int, int>> newGemstones;  // Track (column, type) of new gemstones

    // 遍历所有位置，找到空位并填充新宝石
    for (int col = 0; col < 8; ++col) {
        for (int row = 0; row < 8; ++row) {
            if (gemstoneContainer[row][col] == nullptr) {
                // 创建新宝石，避免立即形成三连
                int type = QRandomGenerator::global()->bounded(difficulty);

                // 检查左边两个
                if (col >= 2 && gemstoneContainer[row][col-1] && gemstoneContainer[row][col-2]) {
                    int type1 = gemstoneContainer[row][col-1]->getType();
                    int type2 = gemstoneContainer[row][col-2]->getType();
                    if (type1 == type2 && type == type1) {
                        type = (type + 1) % difficulty;
                    }
                }

                // 检查上边两个
                if (row >= 2 && gemstoneContainer[row-1][col] && gemstoneContainer[row-2][col]) {
                    int type1 = gemstoneContainer[row-1][col]->getType();
                    int type2 = gemstoneContainer[row-2][col]->getType();
                    if (type1 == type2 && type == type1) {
                        type = (type + 1) % difficulty;
                    }
                }

                Gemstone* gem = new Gemstone(type, "default", rootEntity);

                // 从上方一个位置开始（制造下落效果）
                QVector3D startPos = getPosition(row - 3, col); // 从更高的位置开始
                QVector3D targetPos = getPosition(row, col);

                gem->transform()->setTranslation(startPos);

                // 连接点击信号
                connect(gem, &Gemstone::clicked, this, &MultiplayerModeGameWidget::handleGemstoneClicked);
                connect(gem, &Gemstone::pickEvent, this, [this](const QString& info) {
                    appendDebug(QString("Gemstone %1").arg(info));
                });

                gemstoneContainer[row][col] = gem;

                // Track new gemstone info for network message
                newGemstones.push_back({col, type});

                // 创建下落动画
                QPropertyAnimation* fillAnim = new QPropertyAnimation(gem->transform(), "translation");
                fillAnim->setDuration(500);
                fillAnim->setStartValue(startPos);
                fillAnim->setEndValue(targetPos);
                fillAnimGroup->addAnimation(fillAnim);

                hasFills = true;
            }
        }
    }

    // Send generate message to server (type=3)
    //测试 先禁用
    if (!newGemstones.empty()) {
        // GameNetData generateData;
        // generateData.setType(3);
        // generateData.setID(myUserId);
        // generateData.setCoordinates(newGemstones);
        // sendNetData(generateData);
    }

    if (hasFills) {
        appendDebug("Fill animation started");
        connect(fillAnimGroup, &QParallelAnimationGroup::finished, this, [this]() {
            if (isFinishing) return;
            appendDebug("Fill animation finished, checking for new matches");

            // 发送棋盘同步数据 (Type=4)
            // 当宝石下落完全（哪怕下落完全后可以消除），都要发送数据去让其它棋盘能同步数据
            sendNowBoard();

            // 填充完成后，递归检查是否有新的匹配
            eliminate();
        });
        fillAnimGroup->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        appendDebug("No fills needed, checking for new matches");

        // 发送棋盘同步数据 (Type=4)
        // sendBoardSyncMessage();

        // 没有填充，直接检查匹配
        eliminate();
    }
}

void MultiplayerModeGameWidget::eliminateAnime(Gemstone* gemstone) {
    if (isStop) return;
    if (!gemstone) return;
    
    // Parent animation to gemstone so it dies when gemstone dies
    QPropertyAnimation* animation = new QPropertyAnimation(gemstone->transform(), "scale", gemstone);
    animation->setDuration(500); // 持续缩小直到不见
    animation->setStartValue(gemstone->transform()->scale());
    animation->setEndValue(0.0f);
    
    // Use QPointer to prevent double deletion if gemstone is deleted externally (e.g. by sync)
    QPointer<Gemstone> gemPtr(gemstone);
    connect(animation, &QPropertyAnimation::finished, [gemPtr]() {
        if (gemPtr) {
            gemPtr->setParent((Qt3DCore::QNode*)nullptr);
            gemPtr->deleteLater();
        }
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void MultiplayerModeGameWidget::switchGemstoneAnime(Gemstone* gemstone1, Gemstone* gemstone2) {
    if (isStop) return;
    if (!gemstone1 || !gemstone2) return;
    
    QVector3D pos1 = gemstone1->transform()->translation();
    QVector3D pos2 = gemstone2->transform()->translation();
    
    QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
    
    QPropertyAnimation* anim1 = new QPropertyAnimation(gemstone1->transform(), "translation");
    anim1->setDuration(500); // 0.5s
    anim1->setStartValue(pos1);
    anim1->setEndValue(pos2);
    
    QPropertyAnimation* anim2 = new QPropertyAnimation(gemstone2->transform(), "translation");
    anim2->setDuration(500); // 0.5s
    anim2->setStartValue(pos2);
    anim2->setEndValue(pos1);
    
    group->addAnimation(anim1);
    group->addAnimation(anim2);
    
    // Safety: if either gemstone dies (e.g. sync), stop the animation group to prevent crash
    connect(gemstone1, &QObject::destroyed, group, &QParallelAnimationGroup::stop);
    connect(gemstone2, &QObject::destroyed, group, &QParallelAnimationGroup::stop);
    
    connect(group, &QParallelAnimationGroup::finished, [this]() {
        syncGemstonePositions();
    });
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void MultiplayerModeGameWidget::handleGemstoneClicked(Gemstone* gem) {
    if (isStop) return;
    if (!gem) {
        appendDebug("handleGemstoneClicked: gem is null!");
        return;
    }
    if (!canOpe) return;
    emit userActionOccurred(); // 发送用户操作信号

    appendDebug(QString("Gemstone clicked! Type=%1 Mode=%2 CanOpe=%3 SelectedNum=%4")
        .arg(gem->getType()).arg(mode).arg(canOpe).arg(selectedNum));

    if (mode != 1 || !canOpe) {
        appendDebug(QString("Click ignored: mode=%1 canOpe=%2").arg(mode).arg(canOpe));
        return;
    }

    // 避免重复选择同一个
    if (gem == firstSelectedGemstone || gem == secondSelectedGemstone) {
        appendDebug("Same gemstone clicked, ignoring");
        return;
    }

    if (selectedNum == 0) {
        selectedNum = 1;
        firstSelectedGemstone = gem;
        // 显示第一个选择框
        selectionRing1->setPosition(gem->transform()->translation());
        selectionRing1->setVisible(true);
        appendDebug(QString("First gemstone selected at (%.2f, %.2f)")
            .arg(gem->transform()->translation().x())
            .arg(gem->transform()->translation().y()));
    } else if (selectedNum == 1) {
        selectedNum = 2;
        secondSelectedGemstone = gem;
        // 显示第二个选择框
        selectionRing2->setPosition(gem->transform()->translation());
        selectionRing2->setVisible(true);
        appendDebug(QString("Second gemstone selected at (%.2f, %.2f)")
            .arg(gem->transform()->translation().x())
            .arg(gem->transform()->translation().y()));

        // 自动触发交换逻辑
        int row1 = -1, col1 = -1, row2 = -1, col2 = -1;
        if (findGemstonePosition(firstSelectedGemstone, row1, col1) &&
            findGemstonePosition(secondSelectedGemstone, row2, col2)) {

            appendDebug(QString("Found positions: (%1,%2) and (%3,%4)")
                .arg(row1).arg(col1).arg(row2).arg(col2));

            if (areAdjacent(row1, col1, row2, col2)) {
                appendDebug("Gems are adjacent, performing swap!");
                performSwap(firstSelectedGemstone, secondSelectedGemstone, row1, col1, row2, col2);
            } else {
                appendDebug("Gems are NOT adjacent, clearing selection");
                // 不相邻，清除选择
                firstSelectedGemstone = nullptr;
                secondSelectedGemstone = nullptr;
                selectedNum = 0;
                selectionRing1->setVisible(false);
                selectionRing2->setVisible(false);
            }
        } else {
            appendDebug("ERROR: Could not find gemstone positions!");
        }
    }
}

void MultiplayerModeGameWidget::mousePressEvent(QMouseEvent* event) {
    if (isStop) return;
    if (event->button() == Qt::RightButton) {
        if (mode == 1) {
            // 取消选择
            if (firstSelectedGemstone) {
                firstSelectedGemstone = nullptr;
                selectionRing1->setVisible(false);
            }
            if (secondSelectedGemstone) {
                secondSelectedGemstone = nullptr;
                selectionRing2->setVisible(false);
            }
            selectedNum = 0;
            this->setWindowTitle("Selection Cleared");
        }
    }
    QWidget::mousePressEvent(event);
}

void MultiplayerModeGameWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (container3d) {
        container3d->setFocus(Qt::OtherFocusReason);
        container3d->raise();
    }
    refreshDebugStatus();
    appendDebug("showEvent");
    updateScoreBoard();
    updateTimeBoard();
    if (!isFinishing && timer && !timer->isActive()) {
        timer->start();
        gameTimeKeeper.start();
    }
    resetInactivityTimer();
}

void MultiplayerModeGameWidget::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (timer && timer->isActive()) {
        timer->stop();
    }
    gameTimeKeeper.pause();
    if (inactivityTimer) {
        inactivityTimer->stop();
    }
    clearHighlights();
}

bool MultiplayerModeGameWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == container3d) {
        if (event->type() == QEvent::FocusIn) {
            refreshDebugStatus();
            appendDebug("container3d FocusIn");
        } else if (event->type() == QEvent::FocusOut) {
            refreshDebugStatus();
            appendDebug("container3d FocusOut");
        }
    } else if (obj == game3dWindow) {
        // 处理来自3D窗口的事件
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            appendDebug(QString("game3dWindow MouseButtonPress at (%1, %2)").arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y()));

            // 手动处理点击 - 转换屏幕坐标到世界坐标
            handleManualClick(mouseEvent->pos());
            refreshDebugStatus();
            return false; // 不消费事件，让Qt3D也能处理
        } else if (event->type() == QEvent::MouseMove) {
            // 追踪鼠标移动以确认事件被接收
            static int moveCount = 0;
            if (++moveCount % 50 == 0) { // 每50次移动输出一次
                appendDebug("Mouse moving over 3D window");
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

MultiplayerModeGameWidget::~MultiplayerModeGameWidget() {
    clearHighlights();
    if (inactivityTimer) delete inactivityTimer;

    // 清理主3D场景
    if (rootEntity) {
        delete rootEntity;
        rootEntity = nullptr;
    }
    if (game3dWindow) {
        delete game3dWindow;
        game3dWindow = nullptr;
    }

    if (debugTimer && debugTimer->isActive()) {
        debugTimer->stop();
    }
    
    // 清理网络同步定时器
    if (syncTimer && syncTimer->isActive()) {
        syncTimer->stop();
    }

    // 清理玩家1的小窗口资源
    if (player1Window) {
        player1Window->setRootEntity(nullptr);
        delete player1Window;
        player1Window = nullptr;
    }
    if (player1RootEntity) {
        delete player1RootEntity;
        player1RootEntity = nullptr;
    }

    // 清理玩家2的小窗口资源
    if (player2Window) {
        player2Window->setRootEntity(nullptr);
        delete player2Window;
        player2Window = nullptr;
    }
    if (player2RootEntity) {
        delete player2RootEntity;
        player2RootEntity = nullptr;
    }
}

#include <Qt3DRender/QPickingSettings>
#include <Qt3DRender/QRenderSettings>

void MultiplayerModeGameWidget::setup3DScene() {
    // 根实体
    rootEntity = new Qt3DCore::QEntity();

    // 设置根实体
    game3dWindow->setRootEntity(rootEntity);

    // 尝试配置picking（如果renderSettings可用）
    Qt3DRender::QRenderSettings *renderSettings = game3dWindow->renderSettings();
    if (renderSettings) {
        Qt3DRender::QPickingSettings *pickingSettings = renderSettings->pickingSettings();
        if (pickingSettings) {
            pickingSettings->setPickMethod(Qt3DRender::QPickingSettings::TrianglePicking);
            pickingSettings->setPickResultMode(Qt3DRender::QPickingSettings::NearestPick);
            pickingSettings->setFaceOrientationPickingMode(Qt3DRender::QPickingSettings::FrontAndBackFace);
        }
    }

    // 相机
    cameraEntity = game3dWindow->camera();
    cameraEntity->lens()->setPerspectiveProjection(45.0f, 1.0f, 0.1f, 1000.0f);
    cameraEntity->setPosition(QVector3D(0.0f, 0.0f, 20.0f));
    cameraEntity->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));

    // 灯光
    lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight* light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor(Qt::white);
    light->setIntensity(1.0f);
    lightEntity->addComponent(light);

    Qt3DCore::QTransform* lightTransform = new Qt3DCore::QTransform();
    lightTransform->setTranslation(QVector3D(0.0f, 0.0f, 20.0f));
    lightEntity->addComponent(lightTransform);

    // 初始化选择框
    selectionRing1 = new SelectedCircle(rootEntity);
    selectionRing2 = new SelectedCircle(rootEntity);

    qDebug() << "[MultiplayerModeGameWidget] 3D Scene setup complete - InputSettings and PickingSettings configured";
}

QVector3D MultiplayerModeGameWidget::getPosition(int row, int col) const {
    float startX = -3.5f * 1.5f; // 居中网格
    float startY = 3.5f * 1.5f;
    float spacing = 1.5f;
    
    // 设置位置：[0][0] 是左上角
    // i 是行 (Y), j 是列 (X)
    // 在3D中：X向右增加，Y向上增加。
    // 所以列 j 映射到 X，行 i 映射到 -Y（向下）
    
    float x = startX + col * spacing;
    float y = startY - row * spacing;
    
    return QVector3D(x, y, 0.0f);
}

void MultiplayerModeGameWidget::syncGemstonePositions() {
    for (int i = 0; i < gemstoneContainer.size(); ++i) {
        for (int j = 0; j < gemstoneContainer[i].size(); ++j) {
            Gemstone* gem = gemstoneContainer[i][j];
            if (gem) {
                // 停止该宝石上可能正在运行的任何位置动画
                // 注意：这里我们假设直接设置位置会覆盖正在进行的动画，
                // 或者动画已经结束。如果动画还在运行，直接设置可能会导致冲突，
                // 但Qt3D的transform通常是即时的。
                // 更好的做法是确保没有动画在控制它。
                
                gem->transform()->setTranslation(getPosition(i, j));
            }
        }
    }
}

Qt3DExtras::Qt3DWindow* MultiplayerModeGameWidget::getGame3dWindow() const {
    return game3dWindow;
}

std::vector<std::vector<Gemstone*>> MultiplayerModeGameWidget::getGemstoneContainer() const {
    return gemstoneContainer;
}

void MultiplayerModeGameWidget::setGemstoneContainer(const std::vector<std::vector<Gemstone*>>& container) {
    this->gemstoneContainer = container;
}

std::string MultiplayerModeGameWidget::getStyle() const {
    return style;
}

void MultiplayerModeGameWidget::setStyle(const std::string& style) {
    this->style = style;
}

bool MultiplayerModeGameWidget::getCanOpe() const {
    return canOpe;
}

void MultiplayerModeGameWidget::setCanOpe(bool canOpe) {
    this->canOpe = canOpe;
}

QTimer* MultiplayerModeGameWidget::getTimer() const {
    return timer;
}

void MultiplayerModeGameWidget::setTimer(QTimer* timer) {
    this->timer = timer;
}

int MultiplayerModeGameWidget::getNowTimeHave() const {
    return nowTimeHave;
}

void MultiplayerModeGameWidget::setNowTimeHave(int time) {
    this->nowTimeHave = time;
}

int MultiplayerModeGameWidget::getMode() const {
    return mode;
}

void MultiplayerModeGameWidget::setMode(int mode) {
    this->mode = mode;
}

void MultiplayerModeGameWidget::reset(int mode) {
    this->isStop = false;
    this->mode = mode;
    this->canOpe = true;
    this->isFinishing = false;
    this->gameScore = 0;
    this->targetScore = 10000;
    this->gameTimeKeeper.reset();
    this->nowTimeHave = 0;
    updateScoreBoard();
    updateTimeBoard();
    appendDebug(QString("reset mode=%1").arg(mode));
    
    // 清除现有的宝石（如果有）
    for (auto& row : gemstoneContainer) {
        for (auto* gem : row) {
            if (gem) {
                gem->setParent((Qt3DCore::QNode*)nullptr); // 从场景中分离
                delete gem;
            }
        }
    }
    gemstoneContainer.clear();
    
    // 重建8x8网格
    gemstoneContainer.resize(8);

    for (int i = 0; i < 8; ++i) {
        gemstoneContainer[i].resize(8);
        for (int j = 0; j < 8; ++j) {
            int type = QRandomGenerator::global()->bounded(difficulty);

            // 避免在初始化时创建三连
            // 检查左边两个
            if (j >= 2 && gemstoneContainer[i][j-1] && gemstoneContainer[i][j-2]) {
                int type1 = gemstoneContainer[i][j-1]->getType();
                int type2 = gemstoneContainer[i][j-2]->getType();
                if (type1 == type2 && type == type1) {
                    // 会形成三连，换一个类型
                    type = (type + 1) % difficulty;
                }
            }

            // 检查上边两个
            if (i >= 2 && gemstoneContainer[i-1][j] && gemstoneContainer[i-2][j]) {
                int type1 = gemstoneContainer[i-1][j]->getType();
                int type2 = gemstoneContainer[i-2][j]->getType();
                if (type1 == type2 && type == type1) {
                    // 会形成三连，换一个类型
                    type = (type + 1) % difficulty;
                }
            }

            Gemstone* gem = new Gemstone(type, "default", rootEntity);

            gem->transform()->setTranslation(getPosition(i, j));

            // 连接点击信号
            connect(gem, &Gemstone::clicked, this, &MultiplayerModeGameWidget::handleGemstoneClicked);
            connect(gem, &Gemstone::pickEvent, this, [this](const QString& info) { appendDebug(QString("Gemstone %1").arg(info)); });

            gemstoneContainer[i][j] = gem;
        }
    }
    appendDebug("created 8x8 gemstones with no initial matches");
    
    // 重置选择状态
    selectedNum = 0;
    firstSelectedGemstone = nullptr;
    secondSelectedGemstone = nullptr;

    // 重置定时器
    if (timer->isActive()) {
        timer->stop();
    }
    gameTimeKeeper.pause();
}

void MultiplayerModeGameWidget::appendDebug(const QString& text) {
    if (!debugText) return; // 防止在初始化之前调用
    debugText->append(QString("[%1] %2").arg(QDateTime::currentDateTime().toString("hh:mm:ss")).arg(text));
}

void MultiplayerModeGameWidget::refreshDebugStatus() {
    if (!focusInfoLabel) return;
    bool hasFocusContainer = container3d ? container3d->hasFocus() : false;
    QWidget* active = QApplication::activeWindow();
    QString activeTitle = active ? active->windowTitle() : QString("null");
    focusInfoLabel->setText(QString("ContainerFocus=%1 | ActiveWindow=%2").arg(hasFocusContainer ? "true" : "false").arg(activeTitle));
}

// 找到宝石在容器中的位置
bool MultiplayerModeGameWidget::findGemstonePosition(Gemstone* gem, int& row, int& col) const {
    if (!gem) return false;

    for (int i = 0; i < gemstoneContainer.size(); ++i) {
        for (int j = 0; j < gemstoneContainer[i].size(); ++j) {
            if (gemstoneContainer[i][j] == gem) {
                row = i;
                col = j;
                return true;
            }
        }
    }
    return false;
}

// 检查两个位置是否相邻（上下左右）
bool MultiplayerModeGameWidget::areAdjacent(int row1, int col1, int row2, int col2) const {
    int rowDiff = std::abs(row1 - row2);
    int colDiff = std::abs(col1 - col2);

    // 相邻的条件：要么行相同且列相差1，要么列相同且行相差1
    return (rowDiff == 1 && colDiff == 0) || (rowDiff == 0 && colDiff == 1);
}

// 执行交换
void MultiplayerModeGameWidget::performSwap(Gemstone* gem1, Gemstone* gem2, int row1, int col1, int row2, int col2) {
    if (isStop) return;
    if (!gem1 || !gem2) return;

    // Send swap message to server (type=1)
    GameNetData swapData;
    swapData.setType(1);
    swapData.setID(myUserId);
    std::vector<std::pair<int, int>> coords = {{row1, col1}, {row2, col2}};
    swapData.setCoordinates(coords);
    sendNetData(swapData);

    // 先在逻辑容器中交换
    gemstoneContainer[row1][col1] = gem2;
    gemstoneContainer[row2][col2] = gem1;

    // 播放交换动画
    QVector3D pos1 = gem1->transform()->translation();
    QVector3D pos2 = gem2->transform()->translation();

    QParallelAnimationGroup* group = new QParallelAnimationGroup();

    QPropertyAnimation* anim1 = new QPropertyAnimation(gem1->transform(), "translation");
    anim1->setDuration(500);
    anim1->setStartValue(pos1);
    anim1->setEndValue(pos2);

    QPropertyAnimation* anim2 = new QPropertyAnimation(gem2->transform(), "translation");
    anim2->setDuration(500);
    anim2->setStartValue(pos2);
    anim2->setEndValue(pos1);

    group->addAnimation(anim1);
    group->addAnimation(anim2);

    connect(group, &QParallelAnimationGroup::finished, this, [this, gem1, gem2, row1, col1, row2, col2]() {
        appendDebug("Swap animation finished, checking for matches");

        // 检查是否有匹配
        std::vector<std::pair<int, int>> matches = findMatches();

        if (!matches.empty()) {
            // 有匹配，触发消除
            appendDebug(QString("Found matches after swap, starting elimination"));
            eliminate();
        } else {
            // 没有匹配，交换回来
            appendDebug("No matches found, swapping back");

            // 在逻辑容器中交换回来
            gemstoneContainer[row1][col1] = gem1;
            gemstoneContainer[row2][col2] = gem2;

            // 播放交换回来的动画
            QVector3D pos1 = gem1->transform()->translation();
            QVector3D pos2 = gem2->transform()->translation();

            QParallelAnimationGroup* swapBackGroup = new QParallelAnimationGroup();

            QPropertyAnimation* backAnim1 = new QPropertyAnimation(gem1->transform(), "translation");
            backAnim1->setDuration(500);
            backAnim1->setStartValue(pos1);
            backAnim1->setEndValue(pos2);

            QPropertyAnimation* backAnim2 = new QPropertyAnimation(gem2->transform(), "translation");
            backAnim2->setDuration(500);
            backAnim2->setStartValue(pos2);
            backAnim2->setEndValue(pos1);

            swapBackGroup->addAnimation(backAnim1);
            swapBackGroup->addAnimation(backAnim2);

            connect(swapBackGroup, &QParallelAnimationGroup::finished, this, [this]() {
                canOpe = true; // 恢复操作
            });

            swapBackGroup->start(QAbstractAnimation::DeleteWhenStopped);
        }
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);

    // 清除选择状态
    firstSelectedGemstone = nullptr;
    secondSelectedGemstone = nullptr;
    selectedNum = 0;
    selectionRing1->setVisible(false);
    selectionRing2->setVisible(false);

    appendDebug(QString("Swapped gems at (%1,%2) and (%3,%4)").arg(row1).arg(col1).arg(row2).arg(col2));
}

// 手动处理鼠标点击 - 将屏幕坐标转换为世界坐标并找到最近的宝石
void MultiplayerModeGameWidget::handleManualClick(const QPoint& screenPos) {
    // 获取当前容器大小
    float screenWidth = static_cast<float>(container3d->width());
    float screenHeight = static_cast<float>(container3d->height());

    // 相机参数：FOV=45度，distance=20
    // 计算在z=0平面上的可视范围
    float fovRadians = 45.0f * M_PI / 180.0f;  // 转换为弧度
    float cameraDistance = 20.0f;
    float halfHeight = cameraDistance * std::tan(fovRadians / 2.0f);  // z=0平面上的半高度
    float halfWidth = halfHeight * (screenWidth / screenHeight);  // 根据宽高比调整

    // 将屏幕坐标归一化到 [-1, 1]
    float normalizedX = (screenPos.x() - screenWidth / 2.0f) / (screenWidth / 2.0f);
    float normalizedY = -(screenPos.y() - screenHeight / 2.0f) / (screenHeight / 2.0f);  // Y轴反向

    // 转换到世界坐标（z=0平面）
    float worldX = normalizedX * halfWidth;
    float worldY = normalizedY * halfHeight;

    appendDebug(QString("Click at screen(%1,%2) -> normalized(%3,%4) -> world(%5, %6)")
        .arg(screenPos.x()).arg(screenPos.y())
        .arg(normalizedX, 0, 'f', 2).arg(normalizedY, 0, 'f', 2)
        .arg(worldX, 0, 'f', 2).arg(worldY, 0, 'f', 2));

    // 找到最接近这个位置的宝石
    Gemstone* closestGem = nullptr;
    float minDistance = std::numeric_limits<float>::max();
    int closestRow = -1, closestCol = -1;

    for (int i = 0; i < gemstoneContainer.size(); ++i) {
        for (int j = 0; j < gemstoneContainer[i].size(); ++j) {
            Gemstone* gem = gemstoneContainer[i][j];
            if (gem) {
                QVector3D gemPos = gem->transform()->translation();
                float dx = gemPos.x() - worldX;
                float dy = gemPos.y() - worldY;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance < minDistance) {
                    minDistance = distance;
                    closestGem = gem;
                    closestRow = i;
                    closestCol = j;
                }
            }
        }
    }

    // 如果找到了足够近的宝石（距离 < 0.8，稍微放宽一点）
    if (closestGem && minDistance < 0.8f) {
        appendDebug(QString("Found gemstone at (%1,%2), distance=%3")
            .arg(closestRow).arg(closestCol).arg(minDistance, 0, 'f', 2));
        handleGemstoneClicked(closestGem);
    } else {
        appendDebug(QString("No gemstone found near click (min distance=%1)")
            .arg(minDistance, 0, 'f', 2));
    }
}
// 重置无操作计时器————————————————————————————————————————————————————————————————————————————————————————————————————————————
void MultiplayerModeGameWidget::resetInactivityTimer() {
    clearHighlights();
    if (!inactivityTimer) return;
    if (!isVisible()) {
        inactivityTimer->stop();
        return;
    }
    if (gemstoneContainer.size() != 8) {
        inactivityTimer->stop();
        return;
    }
    for (const auto& row : gemstoneContainer) {
        if (row.size() != 8) {
            inactivityTimer->stop();
            return;
        }
        for (auto* gem : row) {
            if (!gem) {
                inactivityTimer->stop();
                return;
            }
        }
    }
    inactivityTimer->start(inactivityTimeout);
}

std::vector<std::pair<int, int>> MultiplayerModeGameWidget::findPossibleMatches() {
    std::vector<std::pair<int, int>> matches;
    if (gemstoneContainer.size() != 8) return matches;
    for (const auto& row : gemstoneContainer) {
        if (row.size() != 8) return matches;
    }
    std::vector<std::vector<bool>> marked(8, std::vector<bool>(8, false));
    const int dx[4] = {0,0,1,-1};
    const int dy[4] = {1,-1,0,0};
    // 检查水平方向
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 7; ++j) {  // 最多检查到j=5，这样j+2不会越界
            Gemstone* gem1 = gemstoneContainer[i][j];
            Gemstone* gem2 = gemstoneContainer[i][j+1];

            if (gem1 && gem2 && gem1->getType() == gem2->getType()) {
                int L = j-1,R=j+2;
                for(int k = 0;k < 4; k++) {
                    int Dx = i+dx[k],Dy1 = L+dy[k],Dy2 = R+dy[k];
                    if(L>0) {
                        if(Dx >= 0 && Dx < 8 && Dy1 >= 0 && Dy1 < 8 && k!=0) {
                            Gemstone* g = gemstoneContainer[Dx][Dy1];
                            if(g && g->getType() == gem1->getType()) {
                                marked[Dx][Dy1] = true;
                            }
                        }
                    }
                    if(R<8) {
                        if(Dx >= 0 && Dx < 8 && Dy2 >= 0 && Dy2 < 8 && k!=1) {
                            Gemstone* g = gemstoneContainer[Dx][Dy2];
                            if(g && g->getType() == gem1->getType()) {
                                marked[Dx][Dy2] = true;
                            }
                        }
                    }
                }
            }
        }
    }

    // 检查垂直方向
    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 8; ++j) {  // 最多检查到j=5，这样j+2不会越界
            Gemstone* gem1 = gemstoneContainer[i][j];
            Gemstone* gem2 = gemstoneContainer[i+1][j];

            if (gem1 && gem2 && gem1->getType() == gem2->getType()) {
                int L = i-1,R=i+2;
                for(int k = 0;k < 4; k++) {
                    int Dx1 = L+dx[k],Dx2 = R+dx[k],Dy = j+dy[k];
                    if(L>0) {
                        if(Dx1 >= 0 && Dx1 < 8 && Dy >= 0 && Dy < 8 && k!=2) {
                            Gemstone* g = gemstoneContainer[Dx1][Dy];
                            if(g && g->getType() == gem1->getType()) {
                                marked[Dx1][Dy] = true;
                            }
                        }
                    }
                    if(R<8) {
                        if(Dx2 >= 0 && Dx2 < 8 && Dy >= 0 && Dy < 8 && k!=3) {
                            Gemstone* g = gemstoneContainer[Dx2][Dy];
                            if(g && g->getType() == gem1->getType()) {
                                marked[Dx2][Dy] = true;
                            }
                        }
                    }
                }
            }
        }
    }

    // 收集所有被标记的位置
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (marked[i][j]) {
                matches.push_back({i, j});
            }
        }
    }

    return matches;
}

// 高亮显示所有可消除的宝石
void MultiplayerModeGameWidget::highlightMatches() {
    if (!canOpe) return; // 操作不可用时不高亮
    if (!isVisible()) return;
    if (gemstoneContainer.size() != 8) return;
    for (const auto& row : gemstoneContainer) {
        if (row.size() != 8) return;
    }
    
    clearHighlights(); // 先清除现有高亮
    
    // 找到所有可消除的宝石
    std::vector<std::pair<int, int>> matches = findPossibleMatches();
    if (matches.empty()) {
        appendDebug("No possible matches found,resetting the game");
        reset(1);
        return ;
    }
    
    appendDebug(QString("No activity detected for %1 seconds, highlighting %2 matches")
               .arg(inactivityTimeout/1000).arg(matches.size()));
    
    // 为随机一个可消除的宝石添加高亮环
    int choice = QRandomGenerator::global()->bounded(matches.size()) ,num = 0;
    for (const auto& pos : matches) {
        int row = pos.first;
        int col = pos.second;
        Gemstone* gem = gemstoneContainer[row][col];
        if (gem && num == choice) {
            SelectedCircle* ring = new SelectedCircle(rootEntity);
            ring->setVisible(true);
            ring->setPosition(getPosition(row, col));
            highlightRings.push_back(ring);
            break;
        }
        num++;
    }
}

// 清除所有高亮
void MultiplayerModeGameWidget::clearHighlights() {
    for (SelectedCircle* ring : highlightRings) {
        ring->setVisible(false);
        delete ring;
    }
    highlightRings.clear();
}

void MultiplayerModeGameWidget::setDifficulty(int diff) {
    difficulty = diff;
}

int MultiplayerModeGameWidget::getDifficulty() const {
    return difficulty;
}

// ==================== Network Functions ====================

void MultiplayerModeGameWidget::sendNetData(const GameNetData& data) {
    //测试先临时禁用
    if (data.getType() == 3) return;
    if (data.getType() == 2) return;
    //测试先临时禁用

    
    if (isStop) return;
    gameWindow->getNetDataIO()->sendData(data);
}

void MultiplayerModeGameWidget::handleReceivedData(const GameNetData& data) {
    if (isStop) return;
    int type = data.getType();
    appendDebug(QString("Received data type=%1 from ID=%2").arg(type).arg(QString::fromStdString(data.getID())));

    switch (type) {
        case 1:  // Swap
            handleSwapMessage(data);
            break;
        case 2:  // Eliminate
            handleEliminateMessage(data);
            break;
        case 3:  // Generate
            handleGenerateMessage(data);
            break;
        case 4:  // Sync
            handleSyncMessage(data);
            break;
        case 14:  // Connectivity test
            handleConnectivityTest(data);
            break;
        default:
            appendDebug(QString("Unknown message type: %1").arg(type));
            break;
    }
}

void MultiplayerModeGameWidget::handleConnectivityTest(const GameNetData& data) {
    // Type 14: Server sends connectivity test to all players in room
    // Update ID mappings using logic similar to accept10 (assign 1, 2 to others)
    std::map<std::string, int> incomingMap = data.getIdToNum();
    
    this->idToNum.clear();
    this->numToId.clear();
    int index = 1;
    for (auto& pair : incomingMap) {
        std::string id = pair.first;
        if (id == myUserId) continue;
        this->numToId[index] = id;
        this->idToNum[id] = index;
        index++;
    }
    
    // Update labels with initial IDs
    if (numToId.count(1)) {
        player1ScoreLabel->setText(QString("玩家 %1: 0分").arg(QString::fromStdString(numToId[1])));
    } else {
        player1ScoreLabel->setText("等待玩家...");
    }
    
    if (numToId.count(2)) {
        player2ScoreLabel->setText(QString("玩家 %1: 0分").arg(QString::fromStdString(numToId[2])));
    } else {
        player2ScoreLabel->setText("等待玩家...");
    }

    appendDebug(QString("Connectivity test received. Players in room: %1").arg(incomingMap.size()));

    // 限制最多3个玩家
    if (incomingMap.size() > 3) {
        appendDebug("Warning: More than 3 players detected, limiting to 3");
    }

    // Check if all players are ready (based on server's response)
    // 至少需要2个玩家（包括自己），最多3个玩家
    if (incomingMap.size() >= 2 && incomingMap.size() <= 3) {
        allPlayersReady = true;
        if (waitingLabel) {
            waitingLabel->setText(QString("所有玩家已就绪！(%1/3)").arg(incomingMap.size()));
            waitingLabel->setStyleSheet("color: rgba(100,255,100,220); background: transparent;");
        }

        // Enable gameplay
        canOpe = true;

        // Start timer if not already started
        if (timer && !timer->isActive()) {
            timer->start();
            gameTimeKeeper.start();
        }

        // Start sync timer for periodic board synchronization
        if (syncTimer && !syncTimer->isActive()) {
            // syncTimer->start();
            // appendDebug("Started periodic board synchronization (every 5 seconds)");
        }

        appendDebug(QString("All players ready! Game can start with %1 players.").arg(incomingMap.size()));
    } else if (incomingMap.size() < 2) {
        if (waitingLabel) {
            waitingLabel->setText(QString("等待其他玩家... (%1/3)").arg(incomingMap.size()));
            waitingLabel->setStyleSheet("color: rgba(255,220,120,220); background: transparent;");
        }
        appendDebug("Still waiting for other players...");
    }
}

void MultiplayerModeGameWidget::handleSwapMessage(const GameNetData& data) {
    // Type 1: Swap gemstones at given coordinates
    std::string playerId = data.getID();

    // If this is from another player, update their board
    if (playerId != myUserId) {
        std::vector<std::pair<int, int>> coords = data.getCoordinates();
        if (coords.size() >= 2) {
            int row1 = coords[0].first;
            int col1 = coords[0].second;
            int row2 = coords[1].first;
            int col2 = coords[1].second;

            appendDebug(QString("Player %1 swapped (%2,%3) with (%4,%5)")
                .arg(QString::fromStdString(playerId))
                .arg(row1).arg(col1).arg(row2).arg(col2));

            // Update other player's board representation (to be implemented with visual boards)
            // For now, just log it
        }
    }
}

void MultiplayerModeGameWidget::handleEliminateMessage(const GameNetData& data) {
    // Type 2: Eliminate gemstones at given coordinates
    std::string playerId = data.getID();
    int score = data.getMyScore();

    if (playerId != myUserId) {
        std::vector<std::pair<int, int>> coords = data.getCoordinates();
        appendDebug(QString("Player %1 eliminated %2 gemstones, score=%3")
            .arg(QString::fromStdString(playerId))
            .arg(coords.size())
            .arg(score));

        // Use accept2 to handle visual update and score
        accept2(playerId, coords, std::to_string(score));
    }
}

void MultiplayerModeGameWidget::handleGenerateMessage(const GameNetData& data) {
    // Type 3: Generate new gemstones in given column with type
    std::string playerId = data.getID();

    if (playerId != myUserId) {
        std::vector<std::pair<int, int>> coords = data.getCoordinates();
        // coords contains (column, type) pairs
        appendDebug(QString("Player %1 generated new gemstones").arg(QString::fromStdString(playerId)));
    }
}

void MultiplayerModeGameWidget::handleSyncMessage(const GameNetData& data) {
    // Type 4: Force sync board state
    std::string playerId = data.getID();

    if (playerId != myUserId) {
        std::vector<std::vector<int>> board = data.getMyBoard();
        appendDebug(QString("Player %1 synced board").arg(QString::fromStdString(playerId)));

        int score = data.getMyScore();
        // Update other player's board using 3D window
        accept4(playerId, board, score);
        
        // Update score
        if (score >= 0) {
            updateOtherPlayerScore(playerId, score);
        }
    }
}

void MultiplayerModeGameWidget::updateOtherPlayerScore(const std::string& playerId, int score) {
    if (idToNum.find(playerId) == idToNum.end()) return;
    int num = idToNum[playerId];
    
    QLabel* label = nullptr;
    if (num == 1) label = player1ScoreLabel;
    else if (num == 2) label = player2ScoreLabel;
    
    if (label) {
        label->setText(QString("玩家 %1: %2分").arg(QString::fromStdString(playerId)).arg(score));
    }

    appendDebug(QString("Updated score for player %1: %2").arg(QString::fromStdString(playerId)).arg(score));
}

std::vector<std::vector<int>> MultiplayerModeGameWidget::getCurrentBoardState() const {
    std::vector<std::vector<int>> boardState(8, std::vector<int>(8, -1));

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (gemstoneContainer[i][j]) {
                boardState[i][j] = gemstoneContainer[i][j]->getType();
            }
        }
    }

    return boardState;
}

void MultiplayerModeGameWidget::sendBoardSyncMessage() {
    if (isStop) return;

    // Get current board state
    std::vector<std::vector<int>> boardState = getCurrentBoardState();

    // Send type=4 sync message
    GameNetData syncData;
    syncData.setType(4);
    if (gameWindow) {
        syncData.setID(gameWindow->getUserID());
    } else {
        syncData.setID(myUserId);
    }
    syncData.setMyBoard(boardState);
    syncData.setMyScore(gameScore);
    syncData.setSeconds(nowTimeHave);

    sendNetData(syncData);
    appendDebug(QString("Sent periodic board sync (score=%1, time=%2s)").arg(gameScore).arg(nowTimeHave));
}



/**
 * @Author: NAPH130
 * @Function: 获取ID到数字的映射
 */
std::map<std::string, int> MultiplayerModeGameWidget::getIdToNum() const {
    return idToNum;
}

/**
 * @Author: NAPH130
 * @Function: 设置ID到数字的映射
 */
void MultiplayerModeGameWidget::setIdToNum(const std::map<std::string, int>& map) {
    idToNum = map;
}

/**
 * @Author: NAPH130
 * @Function: 初始化小型3D窗口及其组件
 */
void MultiplayerModeGameWidget::setupSmall3DWindow(Qt3DExtras::Qt3DWindow* window, Qt3DCore::QEntity** root, Qt3DRender::QCamera** camera) {
    window->defaultFrameGraph()->setClearColor(QColor(30, 30, 35));
    *root = new Qt3DCore::QEntity();
    window->setRootEntity(*root);

    *camera = window->camera();
    (*camera)->lens()->setPerspectiveProjection(45.0f, 1.0f, 0.1f, 1000.0f);
    (*camera)->setPosition(QVector3D(0, 0, 18.0f)); 
    (*camera)->setViewCenter(QVector3D(0, 0, 0));
    
    Qt3DCore::QEntity* lightEntity = new Qt3DCore::QEntity(*root);
    Qt3DRender::QPointLight* light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor("white");
    light->setIntensity(1.0f);
    lightEntity->addComponent(light);
    
    Qt3DCore::QTransform* lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(QVector3D(0, 0, 20.0f));
    lightEntity->addComponent(lightTransform);
}

/**
 * @Author: NAPH130
 * @Function: 刷新指定玩家的宝石表格
 */
void MultiplayerModeGameWidget::refreshTabel(int num, const std::vector<std::vector<int>>& table) {
    if (table.size() != 8) {
        appendDebug(QString("refreshTabel: Invalid table rows (%1)").arg(table.size()));
        return;
    }
    for (size_t i = 0; i < table.size(); ++i) {
        if (table[i].size() != 8) {
            appendDebug(QString("refreshTabel: Invalid table columns at row %1: %2").arg(i).arg(table[i].size()));
            return;
        }
    }

    std::vector<std::vector<Gemstone*>>* targetTable;
    Qt3DCore::QEntity* targetRoot;

    if (num == 1) {
        targetTable = &player1Table;
        targetRoot = player1RootEntity;
    } else if (num == 2) {
        targetTable = &player2Table;
        targetRoot = player2RootEntity;
    } else {
        appendDebug(QString("refreshTabel: Invalid player number %1").arg(num));
        return;
    }

    if (!targetRoot) {
        appendDebug(QString("refreshTabel: targetRoot is null for player %1").arg(num));
        return;
    }

    // Ensure target table is initialized
    if (targetTable->size() != 8) {
        targetTable->resize(8);
    }
    for (auto& row : *targetTable) {
        if (row.size() != 8) {
            row.resize(8, nullptr);
        }
    }

    // STEP 1: Clear the entire board first
    // 清空棋盘，删除所有现有的宝石对象
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Gemstone* gem = (*targetTable)[r][c];
            if (gem) {
                // 从场景中移除
                gem->setParent((Qt3DCore::QNode*)nullptr);
                // 安全删除
                gem->deleteLater();
                (*targetTable)[r][c] = nullptr;
            }
        }
    }

    // STEP 2: Generate new gemstones based on the new data
    // 按照新数据生成宝石
    std::string currentStyle = gameWindow ? gameWindow->getGemstoneStyle() : "style1";
    int createdCount = 0;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int type = table[r][c];
            if (type != -1) {
                // 创建新宝石，父节点为对应的 RootEntity
                Gemstone* newGem = new Gemstone(type, currentStyle, targetRoot);
                newGem->setCanBeChosen(false);
                
                // 设置位置
                float x = (c - 3.5f) * 1.1f;
                float y = (3.5f - r) * 1.1f;
                newGem->transform()->setTranslation(QVector3D(x, y, 0));
                
                (*targetTable)[r][c] = newGem;
                createdCount++;
            }
        }
    }

    // Log update
    QString tableSample;
    if (!table.empty() && !table[0].empty()) {
        tableSample = QString("[%1, %2...]").arg(table[0][0]).arg(table[0][1]);
    }
    appendDebug(QString("refreshTabel: Player %1 board rebuilt. Created: %2. Sample: %3")
        .arg(num).arg(createdCount).arg(tableSample));

    // Request render update
    Qt3DExtras::Qt3DWindow* targetWindow = nullptr;
    if (num == 1) targetWindow = player1Window;
    else if (num == 2) targetWindow = player2Window;

    if (targetWindow) {
        targetWindow->requestUpdate();
    }
}

/**
 * @Author: NAPH130
 * @Function: 开始游戏
 */
void MultiplayerModeGameWidget::startGame() {
    isStop = false;
    // 首先刷新自己的棋盘
    reset(1);
    
    // 清理其他玩家的棋盘，防止上一局残留
    if (!player1Table.empty()) {
        for (auto& row : player1Table) {
            for (auto* gem : row) {
                if (gem) {
                    gem->setParent((Qt3DCore::QNode*)nullptr);
                    delete gem;
                }
            }
        }
        player1Table.clear();
    }
    if (!player2Table.empty()) {
        for (auto& row : player2Table) {
            for (auto* gem : row) {
                if (gem) {
                    gem->setParent((Qt3DCore::QNode*)nullptr);
                    delete gem;
                }
            }
        }
        player2Table.clear();
    }

    // 获取棋盘数据
    std::vector<std::vector<int>> myBoard = getCurrentBoardState();

    // 发送 GameNetData
    sendBoardSyncMessage();
    appendDebug("Game started, sent initial board (type=4)");

    /*
    // 延迟再次发送一次同步数据，以防止网络波动或初始化未完成导致的丢包
    QTimer::singleShot(1000, this, [this]() {
        if (!isStop) {
            sendBoardSyncMessage();
            appendDebug("Resent initial board (type=4) for redundancy");
        }
    });
    */
}


/**
 * @Author: NAPH130
 * @Function: 从服务端接收type == 4后执行方法
 */
void MultiplayerModeGameWidget::accept4(std::string id, const std::vector<std::vector<int>>& table, int score) {
    // QDialog dialog(this);
    // dialog.setWindowTitle(QString("棋盘同步, ID: %1").arg(QString::fromStdString(id)));
    // dialog.setModal(true);
    // dialog.exec();
    if (idToNum.find(id) == idToNum.end()) {
        appendDebug(QString("accept4: ID %1 not found in idToNum map").arg(QString::fromStdString(id)));
        return;
    }
    int num = idToNum[id];
    // 更新玩家分数
    if (num == 1) {
        player1ScoreLabel->setText(QString("玩家 %1: %2分").arg(QString::fromStdString(id)).arg(score));
    } else if (num == 2) {
        player2ScoreLabel->setText(QString("玩家 %1: %2分").arg(QString::fromStdString(id)).arg(score));
    }
    appendDebug(QString("accept4: Syncing board for player %1 (Slot %2)").arg(QString::fromStdString(id)).arg(num));
    refreshTabel(num, table);
}

/**
 * @Author: NAPH130
 * @Function: 从服务端接收type == 10后执行方法,初始化ID映射
 */
void MultiplayerModeGameWidget::accept10( std::map<std::string, int> incomingMap) {
    this->idToNum.clear();
    this->numToId.clear();
    int index = 1;
    
    std::string currentUserId = myUserId;
    if (gameWindow) {
        currentUserId = gameWindow->getUserID();
    }

    for (auto& pair : incomingMap) {
        std::string id = pair.first;
        if (id == currentUserId) continue;
        this->numToId[index] = id;
        this->idToNum[id] = index;
        index++;
    }
    
    // Update labels
    if (numToId.count(1)) {
        player1ScoreLabel->setText(QString("玩家 %1: 0分").arg(QString::fromStdString(numToId[1])));
    } else {
        player1ScoreLabel->setText("等待玩家...");
    }
    
    if (numToId.count(2)) {
        player2ScoreLabel->setText(QString("玩家 %1: 0分").arg(QString::fromStdString(numToId[2])));
    } else {
        player2ScoreLabel->setText("等待玩家...");
    }

    // Set players ready and start sync timer
    allPlayersReady = true;
    if (syncTimer && !syncTimer->isActive()) {
        // syncTimer->start();
        // appendDebug("Started periodic board synchronization (every 5 seconds) from accept10");
    }

    startGame();
}
/**
 * @Author: NAPH130
 * @Function: 发送消除坐标和游戏分数
 */
void MultiplayerModeGameWidget::sendCoordinates(std::vector<std::pair<int, int>> coordinates) {
    //测试 先禁用
    return;
    GameNetData data;
    data.setType(2);
    data.setID(gameWindow->getUserID());
    data.setCoordinates(coordinates);
    data.setData(std::to_string(gameScore));
    sendNetData(data);
}
/**
 * @Author: NAPH130
 * @Function: 接收type == 2
 */
void MultiplayerModeGameWidget::accept2(std::string id, std::vector<std::pair<int, int>> coordinates, std::string score) {
    if (idToNum.find(id) == idToNum.end()) return;
    int num = idToNum[id];
    
    // Update score
    try {
        int scoreInt = std::stoi(score);
        updateOtherPlayerScore(id, scoreInt);
    } catch (...) {
        appendDebug("Failed to parse score in accept2: " + QString::fromStdString(score));
    }

    // Eliminate gems
    std::vector<std::vector<Gemstone*>>* targetTable = nullptr;
    if (num == 1) targetTable = &player1Table;
    else if (num == 2) targetTable = &player2Table;

    if (targetTable) {
        // Ensure table is initialized
        if (targetTable->empty()) return;

         for (const auto& coord : coordinates) {
            int r = coord.first;
            int c = coord.second;
            if (r >= 0 && r < 8 && c >= 0 && c < 8) {
                if (targetTable->size() > r && (*targetTable)[r].size() > c) {
                    Gemstone* gem = (*targetTable)[r][c];
                    if (gem) {
                        eliminateAnime(gem);
                        (*targetTable)[r][c] = nullptr;
                    }
                }
            }
        }
    }
}

// 将匹配的宝石分组（识别连续的匹配）
std::vector<std::vector<std::pair<int, int>>> MultiplayerModeGameWidget::groupMatches(
    const std::vector<std::pair<int, int>>& matches) {
    
    std::vector<std::vector<std::pair<int, int>>> groups;
    std::set<std::pair<int, int>> processed;
    
    for (const auto& pos : matches) {
        if (processed.count(pos)) continue;
        
        std::vector<std::pair<int, int>> group;
        std::queue<std::pair<int, int>> queue;
        queue.push(pos);
        processed.insert(pos);
        
        while (!queue.empty()) {
            auto current = queue.front();
            queue.pop();
            group.push_back(current);
            
            // 检查4个方向的相邻宝石
            int dx[] = {-1, 1, 0, 0};
            int dy[] = {0, 0, -1, 1};
            
            for (int i = 0; i < 4; i++) {
                int nr = current.first + dx[i];
                int nc = current.second + dy[i];
                std::pair<int, int> neighbor = {nr, nc};
                
                if (std::find(matches.begin(), matches.end(), neighbor) != matches.end() &&
                    !processed.count(neighbor)) {
                    processed.insert(neighbor);
                    queue.push(neighbor);
                }
            }
        }
        
        groups.push_back(group);
    }
    
    return groups;
}

// 检查匹配组中是否包含特殊宝石
bool MultiplayerModeGameWidget::hasSpecialGem(const std::vector<std::pair<int, int>>& group) const {
    for (const auto& pos : group) {
        Gemstone* gem = gemstoneContainer[pos.first][pos.second];
        if (gem && gem->isSpecial()) {
            return true;
        }
    }
    return false;
}
void MultiplayerModeGameWidget::sendNowBoard() {
    if (isStop) return;
    GameNetData data;
    data.setType(4);
    data.setID(gameWindow->getUserID());
    data.setMyBoard(getCurrentBoardState());
    data.setMyScore(gameScore);
    sendNetData(data);
}
