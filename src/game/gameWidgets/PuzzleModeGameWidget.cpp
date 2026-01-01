#include "PuzzleModeGameWidget.h"
#include "FinalWidget.h"
#include "MenuWidget.h"
#include "../GameWindow.h"
#include "../components/Gemstone.h"
#include "../components/SelectedCircle.h"
#include "../../utils/AudioManager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QDialog>
#include <QPropertyAnimation>
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
#include <cmath>
#include <limits>
#include <iostream>
#include <algorithm>
#include <QWidget>
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

PuzzleModeGameWidget::PuzzleModeGameWidget(QWidget* parent, GameWindow* gameWindow) 
    : QWidget(parent), gameWindow(gameWindow), canOpe(true), nowTimeHave(0), mode(1),
      firstSelectedGemstone(nullptr), secondSelectedGemstone(nullptr), selectedNum(0) {
    
    // 初始化定时器
    timer = new QTimer(this);
    timer->setInterval(16);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (isFinishing) return;
        gameTimeKeeper.tick();
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
    // container3d->setFixedSize(960, 960); // 移除固定大小
    container3d->setMinimumSize(600, 600); // 设置最小大小
    container3d->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container3d->setFocusPolicy(Qt::StrongFocus);
    container3d->setMouseTracking(true); // 启用鼠标追踪
    container3d->setAttribute(Qt::WA_Hover, true); // 启用hover事件
    
    // 布局 - 左侧居中
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(50, 0, 50, 0); // 添加一些边距
    
        // 左侧容器：顶部进度条 + 下方3D窗口
    QWidget* leftPanel = new QWidget(this);
    leftPanel->setStyleSheet("background: transparent;");
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(18);

    leftLayout->addWidget(container3d, 1);

    // 将左侧容器对齐到左侧，垂直居中
    mainLayout->addWidget(leftPanel, 1);

    mainLayout->addStretch(0);
    
    rightPanel = new QWidget(this);
    rightPanel->setFixedWidth(420);
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

    auto* infoCard = new QWidget(rightPanel);
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

    infoLayout->addWidget(scoreBoardLabel);
    infoLayout->addWidget(timeBoardLabel);
    panelLayout->addWidget(infoCard, 0, Qt::AlignTop);

    panelLayout->addStretch(1);
// ———————————————————————————————————————————————————————————— debug
    debugText = new QTextEdit(this);
    debugText->setReadOnly(true);
    debugText->setFixedHeight(200); // 固定高度
    // 添加到现有布局中（例如右侧面板）
    panelLayout->addWidget(debugText);

    // 新增重置按钮
    resetButton = new QPushButton("重置状态", rightPanel);
    resetButton->setFixedSize(180, 54);
    resetButton->setCursor(Qt::PointingHandCursor);
    resetButton->setStyleSheet(R"(
        QPushButton {
            color: white;
            border-radius: 14px;
            border: 1px solid rgba(255,255,255,55);
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(120,200,255,220), stop:1 rgba(80,150,255,220));
            font-family: 'Microsoft YaHei';
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(140,220,255,235), stop:1 rgba(100,170,255,235));
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(100,170,220,235), stop:1 rgba(60,130,220,235));
        }
    )");

    auto* resetShadow = new QGraphicsDropShadowEffect(resetButton);
    resetShadow->setBlurRadius(18);
    resetShadow->setOffset(0, 8);
    resetShadow->setColor(QColor(0, 0, 0, 120));
    resetButton->setGraphicsEffect(resetShadow);

    connect(resetButton, &QPushButton::clicked, this, &PuzzleModeGameWidget::checkLastGemState);

    panelLayout->addWidget(resetButton, 0, Qt::AlignRight | Qt::AlignBottom);



    backToMenuButton = new QPushButton("返回菜单", rightPanel);
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

    connect(backToMenuButton, &QPushButton::clicked, this, [this]() {
        GameBackDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            if (timer && timer->isActive()) timer->stop();
            if (inactivityTimer) inactivityTimer->stop();
            if (this->gameWindow && this->gameWindow->getMenuWidget()) {
                this->gameWindow->switchWidget(this->gameWindow->getMenuWidget());
            }
        }
    });

    panelLayout->addWidget(backToMenuButton, 0, Qt::AlignRight | Qt::AlignBottom);

    focusInfoLabel = new QLabel(rightPanel);
    focusInfoLabel->setVisible(false);
    debugTimer = new QTimer(this);

    setLayout(mainLayout);
    mainLayout->addWidget(rightPanel, 0, Qt::AlignRight | Qt::AlignVCenter);
    container3d->installEventFilter(this);
    game3dWindow->installEventFilter(this); // 关键：在3D窗口上安装事件过滤器

    // 初始化无操作计时器
    inactivityTimer = new QTimer(this);
    inactivityTimer->setInterval(inactivityTimeout);
    inactivityTimer->setSingleShot(true);
    
    // 超时后高亮可消除的宝石
    connect(inactivityTimer, &QTimer::timeout, this, &PuzzleModeGameWidget::highlightMatches);
    
    // 任何用户操作后重置计时器
    connect(this, &PuzzleModeGameWidget::userActionOccurred, [this]() {
        resetInactivityTimer();
    });

    inactivityTimer->stop();

    updateScoreBoard();
    updateTimeBoard();
    appendDebug("SingleModeGameWidget initialized - EventFilter installed on both container and 3D window");
}

void PuzzleModeGameWidget::GameTimeKeeper::reset() {
    seconds = 0;
}

void PuzzleModeGameWidget::GameTimeKeeper::tick() {
    ++seconds;
}

int PuzzleModeGameWidget::GameTimeKeeper::totalSeconds() const {
    return seconds;
}

QString PuzzleModeGameWidget::GameTimeKeeper::displayText() const {
    int m = seconds / 60;
    int s = seconds % 60;
    return QString("游戏进行时间：%1:%2")
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}

void PuzzleModeGameWidget::updateScoreBoard() {
    if (!scoreBoardLabel) return;
    scoreBoardLabel->setText(QString("当前分数：%1").arg(gameScore));

}


void PuzzleModeGameWidget::updateTimeBoard() {
    if (!timeBoardLabel) return;
    timeBoardLabel->setText(gameTimeKeeper.displayText());
}

void PuzzleModeGameWidget::triggerFinishIfNeeded() {
    if (GemNumber == 0) 
        finishToNextLevel();
}

void PuzzleModeGameWidget::finishToNextLevel() {
    //此处应有 目标完成 前往下一关的弹幕生成

    /* ( ) */
    PuzzleModeGameWidget::reset(1);
}

// 查找所有需要消除的宝石（三连或更多）
std::vector<std::pair<int, int>> PuzzleModeGameWidget::findMatches() {
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

// 移除匹配的宝石
void PuzzleModeGameWidget::removeMatches(const std::vector<std::pair<int, int>>& matches) {
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
            // 找到特殊宝石的位置
            for (const auto& pos : group) {
                Gemstone* gem = gemstoneContainer[pos.first][pos.second];
                if (gem && gem->isSpecial()) {
                    // 消除3×3区域
                    remove3x3Area(pos.first, pos.second);
                    break;  // 只处理第一个特殊宝石
                }
            }
        } else if (group.size() == 4) {
            // 4连或更多：保留第2颗宝石作为特殊宝石
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
        GemNumber -= removedCount;
        appendDebug(QString("Still %1 Gems there.").arg(GemNumber));
        gameScore += removedCount * 10;
        GemNumber -= removedCount;
        updateScoreBoard();
        triggerFinishIfNeeded();
    }

}


void PuzzleModeGameWidget::eliminate() {
    if (isFinishing) return;
    // 查找所有匹配
    std::vector<std::pair<int, int>> matches = findMatches();
    if (!matches.empty()) {

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
        pushInLastStateQueue();
        comboCount = 0;
        // 没有匹配了，恢复操作
        canOpe = true;
        resetInactivityTimer();
        appendDebug("No matches found, game can continue");
    }
}

void PuzzleModeGameWidget::drop() {
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
            eliminate();
        });
        dropAnimGroup->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        appendDebug("No drops needed, filling new gemstones");
        eliminate();
    }
}

void PuzzleModeGameWidget::eliminateAnime(Gemstone* gemstone) {
    if (!gemstone) return;
    
    QPropertyAnimation* animation = new QPropertyAnimation(gemstone->transform(), "scale");
    animation->setDuration(500); // 持续缩小直到不见
    animation->setStartValue(gemstone->transform()->scale());
    animation->setEndValue(0.0f);
    
    connect(animation, &QPropertyAnimation::finished, [gemstone]() {
        gemstone->setParent((Qt3DCore::QNode*)nullptr);
        delete gemstone;
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void PuzzleModeGameWidget::switchGemstoneAnime(Gemstone* gemstone1, Gemstone* gemstone2) {
    if (!gemstone1 || !gemstone2) return;
    
    QVector3D pos1 = gemstone1->transform()->translation();
    QVector3D pos2 = gemstone2->transform()->translation();
    
    QParallelAnimationGroup* group = new QParallelAnimationGroup();
    
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
    
    connect(group, &QParallelAnimationGroup::finished, [this]() {
        syncGemstonePositions();
    });
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void PuzzleModeGameWidget::handleGemstoneClicked(Gemstone* gem) {
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

void PuzzleModeGameWidget::mousePressEvent(QMouseEvent* event) {
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

void PuzzleModeGameWidget::showEvent(QShowEvent* event) {
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
    }
    resetInactivityTimer();
}

void PuzzleModeGameWidget::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (timer && timer->isActive()) {
        timer->stop();
    }
    if (inactivityTimer) {
        inactivityTimer->stop();
    }
    clearHighlights();
}

bool PuzzleModeGameWidget::eventFilter(QObject* obj, QEvent* event) {
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

PuzzleModeGameWidget::~PuzzleModeGameWidget() {
    clearHighlights();
    delete inactivityTimer;

    if (rootEntity) {
        delete rootEntity;
    }
    if (game3dWindow) {
        delete game3dWindow;
    }
    if (debugTimer && debugTimer->isActive()) {
        debugTimer->stop();
    }
}

#include <Qt3DRender/QPickingSettings>
#include <Qt3DRender/QRenderSettings>

void PuzzleModeGameWidget::setup3DScene() {
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

    qDebug() << "[SingleModeGameWidget] 3D Scene setup complete - InputSettings and PickingSettings configured";
}

QVector3D PuzzleModeGameWidget::getPosition(int row, int col) const {
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

void PuzzleModeGameWidget::syncGemstonePositions() {
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

Qt3DExtras::Qt3DWindow* PuzzleModeGameWidget::getGame3dWindow() const {
    return game3dWindow;
}

std::vector<std::vector<Gemstone*>> PuzzleModeGameWidget::getGemstoneContainer() const {
    return gemstoneContainer;
}

void PuzzleModeGameWidget::setGemstoneContainer(const std::vector<std::vector<Gemstone*>>& container) {
    this->gemstoneContainer = container;
}

std::string PuzzleModeGameWidget::getStyle() const {
    return style;
}

void PuzzleModeGameWidget::setStyle(const std::string& style) {
    this->style = style;
}

bool PuzzleModeGameWidget::getCanOpe() const {
    return canOpe;
}

void PuzzleModeGameWidget::setCanOpe(bool canOpe) {
    this->canOpe = canOpe;
}

QTimer* PuzzleModeGameWidget::getTimer() const {
    return timer;
}

void PuzzleModeGameWidget::setTimer(QTimer* timer) {
    this->timer = timer;
}

int PuzzleModeGameWidget::getNowTimeHave() const {
    return nowTimeHave;
}

void PuzzleModeGameWidget::setNowTimeHave(int time) {
    this->nowTimeHave = time;
}

int PuzzleModeGameWidget::getMode() const {
    return mode;
}

void PuzzleModeGameWidget::setMode(int mode) {
    this->mode = mode;
}

bool PuzzleModeGameWidget::checkConflict(int x,int y,int type) {
    int dx[4] = {0,0,1,-1};
    int dy[4] = {1,-1,0,0};
    for(int i=0;i<4;i++) {
        if(x+dx[i] >= 0&& x+dx[i] < 8&& y+dy[i] >= 0&& y+dy[i] < 8) {
            if(TempGemState[x+dx[i]][y+dy[i]] == type + '0') return true;
        }
    }
    return false;
}

void PuzzleModeGameWidget::changeIndex(int a,int b,int c,int d,int e,int f) {
    int type = QRandomGenerator::global()->bounded(difficulty);
    TempGemState[a][b] = TempGemState[c][d] = TempGemState[e][f] = '-';
    //清空要放当前这一匹配的位置
    while(checkConflict(a,b,type) ||
        checkConflict(c,d,type) || checkConflict(e,f,type)) type = (type + 1)% 8;
    TempGemState[a][b] = TempGemState[c][d] = TempGemState[e][f] = type + '0';
}

void PuzzleModeGameWidget::generateSimpleMatch() {//Tem是 列 行 存储
    bool CrossOrVertical = QRandomGenerator::global()->bounded(2);
    int StartPos = QRandomGenerator::global()->bounded(6);
    if(ConstChange) {
        if(lenthT[midX] <= 5) {
            for(int i=1;i<=3;i++) {
                for(int j = 7;j > midY + i;j --) 
                    TempGemState[midX][j] = TempGemState[midX][j-1];
            }
            TempGemState[midX][midY + 1] = TempGemState[midX][midY];
            changeIndex(midX,midY,midX,midY+2,midX,midY+3);
            lenthT[midX] += 3;
        } else if(lenthT[midX - 1] < 8&&lenthT[midX] < 8&&lenthT[midX + 1] < 8) {
            for(int i=-1;i<=1;i++) {
                for(int j = 7;j > midY + 1;j--) 
                    TempGemState[midX + i][j] = TempGemState[midX + i][j-1];
            }
            TempGemState[midX][midY+1] = TempGemState[midX][midY];
            changeIndex(midX,midY , midX-1,midY+1 , midX+1,midY+1);
            lenthT[midX - 1] ++; lenthT[midX] ++; lenthT[midX + 1] ++;
        } else {
            ConstChange = 0;
            midX = midY = 0;
            generateSimpleMatch();
        }
    } else if(CrossOrVertical || GemNumber == 0 || !CrossOrVertical) { // Cross
        int VertBound = 10;
        for(int i=0;i<3;i++) {
            VertBound = std::min(VertBound , lenthT[StartPos + i]);
        }

        int Choice_01 = QRandomGenerator::global()->bounded(4);
        if(Choice_01 == 30 && VertBound >= 1) {//横着的上下交换，前提是当前三列都有宝石
            Choice_01 = QRandomGenerator::global()->bounded(3);
            int type = QRandomGenerator::global()->bounded(difficulty);
            
        } else { 
            VertBound = 10;
            StartPos = QRandomGenerator::global()->bounded(5);
            while(VertBound >= 8) {
                StartPos = (StartPos + 1) % 5 , VertBound = 10;
                for(int i=0;i<=3;i++) {
                    VertBound = std::min(VertBound , lenthT[StartPos + i]);
                    if(lenthT[StartPos + i] == 8) {
                        VertBound = 10;
                        break;
                    }
                }
            }
            int type = QRandomGenerator::global()->bounded(difficulty);
            
            //中间两列的上移
            for(int i = 7;i>VertBound;i--) 
                TempGemState[StartPos+2][i] = TempGemState[StartPos+2][i-1] ,
                TempGemState[StartPos+1][i] = TempGemState[StartPos+1][i-1];
            lenthT[StartPos + 1] ++; lenthT[StartPos + 2] ++;
            if(StartPos >= 3) {//XXOX形式

                //第一列上移
                for(int i = 7;i>VertBound;i--) 
                    TempGemState[StartPos][i] = TempGemState[StartPos][i-1];
                lenthT[StartPos] ++;
                
                //第三列变成第四列的，交换当前这个后恢复原样
                if(TempGemState[StartPos+2][VertBound] != '-' && TempGemState[StartPos+3][VertBound] == '-') {//这时交换会出现悬空，不正确
                    if(VertBound > 0) {//与下面交换 减少开局出错概率
                        TempGemState[StartPos+2][VertBound] = TempGemState[StartPos+2][VertBound-1];
                        changeIndex(StartPos,VertBound,StartPos+1,VertBound,StartPos+2,VertBound-1);
                    } else {
                        TempGemState[StartPos+2][VertBound] = TempGemState[StartPos+2][VertBound+1];
                        changeIndex(StartPos,VertBound,StartPos+1,VertBound,StartPos+2,VertBound+1);
                    }
                    midX = StartPos + 1; midY = VertBound;
                    ConstChange = 1;
                } else {
                    TempGemState[StartPos+2][VertBound] = TempGemState[StartPos+3][VertBound];

                    changeIndex(StartPos,VertBound,StartPos+1,VertBound,StartPos+3,VertBound);
                    //替换0 1 3
                }

            } else {//XOXX形式
                for(int i = 7;i>VertBound;i--) 
                     TempGemState[StartPos+3][i] = TempGemState[StartPos+3][i-1];
                lenthT[StartPos + 3] ++;

                //第二列变成第一列的，交换当前这个后恢复原样
                if(TempGemState[StartPos+1][VertBound] != '-' && TempGemState[StartPos][VertBound] == '-') {//这时交换会出现悬空，不正确
                    if(VertBound > 0) {
                        TempGemState[StartPos+1][VertBound] = TempGemState[StartPos+1][VertBound-1];
                        changeIndex(StartPos+1,VertBound-1,StartPos+2,VertBound,StartPos+3,VertBound);
                    } else {
                        TempGemState[StartPos+1][VertBound] = TempGemState[StartPos+1][VertBound+1];
                        changeIndex(StartPos+1,VertBound+1,StartPos+2,VertBound,StartPos+3,VertBound);
                    }
                    midX = StartPos + 1; midY = VertBound;
                    ConstChange = 1;
                } else {
                    TempGemState[StartPos+1][VertBound] = TempGemState[StartPos][VertBound];

                    changeIndex(StartPos,VertBound,StartPos+2,VertBound,StartPos+3,VertBound);
                    //替换0 2 3
                }
            }

        }
    } else { // Vertical
        
    }
    GemNumber += 3;
}

void PuzzleModeGameWidget::reset(int mode) {
    this->mode = mode;
    this->canOpe = true;
    this->isFinishing = false;
    this->gameTimeKeeper.reset();
    this->nowTimeHave = 0;
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
    
    Level ++;
    ConstChange = 0;
    // 重建8x8网格
    gemstoneContainer.resize(8);
    for(int i=0; i<8; i++) {
        gemstoneContainer[i].resize(8);
        TempGemState[i] = "--------";
        lenthT[i] = 0;
    }
    GemNumber = 0;
    midX = midY = 0;

    int MemberNum = std::max(5 , std::min(10,3*Level));
    bool SpecialComplete = false;
    while(MemberNum) {
        if(Level >= 2 && !SpecialComplete) {//可能 生成一个残缺匹配，需要特殊宝石来消除
            if(QRandomGenerator::global()->bounded(2) == 20) {//暂时不写
                GemNumber += 5;
            } else {
                generateSimpleMatch();
            }
            SpecialComplete = true;
        } 
        generateSimpleMatch();
        MemberNum --;
    }

    QString tempGemStateStr;
    for (int i = 0; i < 8; ++i) {
        tempGemStateStr += QString("Row %1: %2\n").arg(i).arg(TempGemState[i]);
    }
    debugText->setText(tempGemStateStr); // 刷新显示
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if(TempGemState[j][i] < '0' || TempGemState[j][i] > '9') continue;
            int type = TempGemState[j][i] - '0';
            Gemstone* gem = new Gemstone(type, "default", rootEntity);

            gem->transform()->setTranslation(getPosition(7-i, j));

            // 连接点击信号
            connect(gem, &Gemstone::clicked, this, &PuzzleModeGameWidget::handleGemstoneClicked);
            connect(gem, &Gemstone::pickEvent, this, [this](const QString& info) { appendDebug(QString("Gemstone %1").arg(info)); });

            gemstoneContainer[7-i][j] = gem;
        }
    }
    appendDebug("created puzzle gemstones with no initial matches");
    
    // 重置选择状态
    selectedNum = 0;
    firstSelectedGemstone = nullptr;
    secondSelectedGemstone = nullptr;
    drop();
    while(!lastGemStateStack.empty()) lastGemStateStack.pop();
    pushInLastStateQueue();
    // 重置定时器
    if (timer->isActive()) {
        timer->stop();
    }
}

void PuzzleModeGameWidget::appendDebug(const QString& text) {
    if (!debugText) return; // 防止在初始化之前调用
    debugText->append(QString("[%1] %2").arg(QDateTime::currentDateTime().toString("hh:mm:ss")).arg(text));
}

void PuzzleModeGameWidget::refreshDebugStatus() {
    if (!focusInfoLabel) return;
    bool hasFocusContainer = container3d ? container3d->hasFocus() : false;
    QWidget* active = QApplication::activeWindow();
    QString activeTitle = active ? active->windowTitle() : QString("null");
    focusInfoLabel->setText(QString("ContainerFocus=%1 | ActiveWindow=%2").arg(hasFocusContainer ? "true" : "false").arg(activeTitle));
}

// 找到宝石在容器中的位置
bool PuzzleModeGameWidget::findGemstonePosition(Gemstone* gem, int& row, int& col) const {
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
bool PuzzleModeGameWidget::areAdjacent(int row1, int col1, int row2, int col2) const {
    int rowDiff = std::abs(row1 - row2);
    int colDiff = std::abs(col1 - col2);

    // 相邻的条件：要么行相同且列相差1，要么列相同且行相差1
    return (rowDiff == 1 && colDiff == 0) || (rowDiff == 0 && colDiff == 1);
}

// 执行交换
void PuzzleModeGameWidget::performSwap(Gemstone* gem1, Gemstone* gem2, int row1, int col1, int row2, int col2) {
    // gem1 不能为空，但 gem2 可以为空（nullptr表示空位）
    if (!gem1) return;

    // 先在逻辑容器中交换
    gemstoneContainer[row1][col1] = gem2;
    gemstoneContainer[row2][col2] = gem1;

    // 播放交换动画
    QVector3D pos1 = gem1->transform()->translation();
    QVector3D pos2 = getPosition(row2, col2);  // 目标位置

    if (gem2) {
        // 两个宝石交换
        QParallelAnimationGroup* group = new QParallelAnimationGroup();

        QPropertyAnimation* anim1 = new QPropertyAnimation(gem1->transform(), "translation");
        anim1->setDuration(500);
        anim1->setStartValue(pos1);
        anim1->setEndValue(pos2);

        QPropertyAnimation* anim2 = new QPropertyAnimation(gem2->transform(), "translation");
        anim2->setDuration(500);
        anim2->setStartValue(gem2->transform()->translation());
        anim2->setEndValue(pos1);

        group->addAnimation(anim1);
        group->addAnimation(anim2);

        connect(group, &QParallelAnimationGroup::finished, this, [this, gem1, gem2, row1, col1, row2, col2]() {
            appendDebug("Swap animation finished, checking for matches");

            // 检查是否有匹配（解谜模式）
            std::vector<std::pair<int, int>> matches = findMatches();

            if (!matches.empty()) {
                // 有匹配，触发消除和下落（解谜模式：下落但不填充新宝石）
                appendDebug(QString("Found matches after swap, starting elimination (puzzle mode)"));
                canOpe = false;
                
                // 播放消除音效
                comboCount++;
                AudioManager::instance().playEliminateSound(comboCount);
                
                // 移除匹配的宝石
                removeMatches(matches);
                
                // 等待消除动画完成后执行下落
                QTimer::singleShot(600, this, [this]() {
                    if (isFinishing) return;
                    appendDebug("Starting drop after elimination");
                    drop();  // 关键：调用下落函数
                });
            } else {
                // 没有匹配，交换回来
                appendDebug("No matches found, swapping back");
                comboCount = 0;

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
                    canOpe = true;
                    resetInactivityTimer();
                    appendDebug("Swap back complete");
                });

                swapBackGroup->start(QAbstractAnimation::DeleteWhenStopped);
            }
        });

        group->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        // 宝石移动到空位（也需要检查匹配！）
        QPropertyAnimation* anim = new QPropertyAnimation(gem1->transform(), "translation");
        anim->setDuration(500);
        anim->setStartValue(pos1);
        anim->setEndValue(pos2);

        connect(anim, &QPropertyAnimation::finished, this, [this, gem1, row1, col1, row2, col2]() {
            appendDebug("Move to empty space complete, checking for matches");

            // 检查是否有匹配（关键修改：空位交换后也检查匹配）
            std::vector<std::pair<int, int>> matches = findMatches();

            if (!matches.empty()) {
                // 有匹配，触发消除和下落
                appendDebug(QString("Found matches after moving to empty space, starting elimination"));
                canOpe = false;
                
                // 播放消除音效
                comboCount++;
                AudioManager::instance().playEliminateSound(comboCount);
                
                // 移除匹配的宝石
                removeMatches(matches);
                
                // 等待消除动画完成后执行下落
                QTimer::singleShot(600, this, [this]() {
                    if (isFinishing) return;
                    appendDebug("Starting drop after elimination (from empty space move)");
                    drop();
                });
            } else {
                // 没有匹配，移回原位
                appendDebug("No matches found after moving to empty space, moving back");
                comboCount = 0;

                // 在逻辑容器中移回原位
                gemstoneContainer[row1][col1] = gem1;
                gemstoneContainer[row2][col2] = nullptr;

                // 播放移回动画
                QVector3D currentPos = gem1->transform()->translation();
                QVector3D originalPos = getPosition(row1, col1);

                QPropertyAnimation* moveBackAnim = new QPropertyAnimation(gem1->transform(), "translation");
                moveBackAnim->setDuration(500);
                moveBackAnim->setStartValue(currentPos);
                moveBackAnim->setEndValue(originalPos);

                connect(moveBackAnim, &QPropertyAnimation::finished, this, [this]() {
                    canOpe = true;
                    resetInactivityTimer();
                    appendDebug("Move back to original position complete");
                });

                moveBackAnim->start(QAbstractAnimation::DeleteWhenStopped);
            }
        });

        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    // 清除选择状态
    firstSelectedGemstone = nullptr;
    secondSelectedGemstone = nullptr;
    selectedNum = 0;
    selectionRing1->setVisible(false);
    selectionRing2->setVisible(false);

    appendDebug(QString("Swapped: (%1,%2) with (%3,%4)")
        .arg(row1).arg(col1).arg(row2).arg(col2));
}



// 手动处理鼠标点击 - 将屏幕坐标转换为世界坐标并找到最近的宝石
void PuzzleModeGameWidget::handleManualClick(const QPoint& screenPos) {
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

    // 找到最接近这个位置的格子（不管是否有宝石）
    Gemstone* closestGem = nullptr;
    float minDistance = std::numeric_limits<float>::max();
    int closestRow = -1, closestCol = -1;

    for (int i = 0; i < gemstoneContainer.size(); ++i) {
        for (int j = 0; j < gemstoneContainer[i].size(); ++j) {
            // 计算格子中心位置
            QVector3D gridPos = getPosition(i, j);
            float dx = gridPos.x() - worldX;
            float dy = gridPos.y() - worldY;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < minDistance) {
                minDistance = distance;
                closestGem = gemstoneContainer[i][j];  // 可能是 nullptr（空位）
                closestRow = i;
                closestCol = j;
            }
        }
    }

    // 如果找到了足够近的格子（距离 < 0.8）
    if (minDistance < 0.8f) {
        if (closestGem) {
            // 点击的是宝石
            appendDebug(QString("Found gemstone at (%1,%2), distance=%3")
                .arg(closestRow).arg(closestCol).arg(minDistance, 0, 'f', 2));
            handleGemstoneClicked(closestGem);
        } else {
            // 点击的是空位（nullptr）
            appendDebug(QString("Clicked empty space at (%1,%2), distance=%3")
                .arg(closestRow).arg(closestCol).arg(minDistance, 0, 'f', 2));
            
            // 如果已经选择了一个宝石，尝试与空位交换
            if (selectedNum == 1 && firstSelectedGemstone) {
                int row1 = -1, col1 = -1;
                if (findGemstonePosition(firstSelectedGemstone, row1, col1)) {
                    if (areAdjacent(row1, col1, closestRow, closestCol)) {
                        appendDebug("Selected gem is adjacent to empty space, performing swap!");
                        performSwap(firstSelectedGemstone, nullptr, row1, col1, closestRow, closestCol);
                    } else {
                        appendDebug("Selected gem is NOT adjacent to empty space");
                    }
                }
            }
        }
    } else {
        appendDebug(QString("No grid found near click (min distance=%1)")
            .arg(minDistance, 0, 'f', 2));
    }
}

// 重置无操作计时器————————————————————————————————————————————————————————————————————————————————————————————————————————————
void PuzzleModeGameWidget::resetInactivityTimer() {
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

std::vector<std::pair<int, int>> PuzzleModeGameWidget::findPossibleMatches() {
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
void PuzzleModeGameWidget::highlightMatches() {
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
void PuzzleModeGameWidget::clearHighlights() {
    for (SelectedCircle* ring : highlightRings) {
        ring->setVisible(false);
        delete ring;
    }
    highlightRings.clear();
}

void PuzzleModeGameWidget::setDifficulty(int diff) {
    difficulty = diff;
}

int PuzzleModeGameWidget::getDifficulty() const {
    return difficulty;
}

void PuzzleModeGameWidget::backToLastGemstoneState(std::string LastState) {
    if (isFinishing) return ;

    appendDebug("Reverting to last gemstone state");

    // 清除当前宝石
    for (auto& row : gemstoneContainer) {
        for (auto* gem : row) {
            if (gem) {
                gem->setParent((Qt3DCore::QNode*)nullptr); // 从场景中分离
                delete gem;
            }
        }
    }
    gemstoneContainer.clear();

    GemNumber = 0;
    gemstoneContainer.resize(8);
    for (int i = 0; i < 8; ++i) {
        gemstoneContainer[i].resize(8);
        for (int j = 0; j < 8; ++j) {
            if(LastState[i * 8 + j] == '-') {
                continue;
            }
            GemNumber ++;
            int type = LastState[i * 8 + j] - '0'; // 假设LastState是一个长度为64的字符串，每个字符表示一个宝石类型

            Gemstone* gem = new Gemstone(type, "default", rootEntity);

            gem->transform()->setTranslation(getPosition(i, j));

            // 连接点击信号
            connect(gem, &Gemstone::clicked, this, &PuzzleModeGameWidget::handleGemstoneClicked);
            connect(gem, &Gemstone::pickEvent, this, [this](const QString& info) { appendDebug(QString("Gemstone %1").arg(info)); });

            gemstoneContainer[i][j] = gem;
        }
    }

    // 重新设置宝石的位置
    // syncGemstonePositions();

    appendDebug("Reverted to last gemstone state");

    return ;
}

void PuzzleModeGameWidget::checkLastGemState() {
    if (isFinishing) return;
    appendDebug("Checking for last gemstone state to revert to");
    if (lastGemStateStack.size() == 1) {
        appendDebug("No last gemstone state found");
        return;
    }
    lastGemStateStack.pop();
    backToLastGemstoneState(lastGemStateStack.top());
    return ;
}

// 将匹配的宝石分组（识别连续的匹配）
std::vector<std::vector<std::pair<int, int>>> PuzzleModeGameWidget::groupMatches(
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

// 消除以特殊宝石为中心的3×3区域
void PuzzleModeGameWidget::remove3x3Area(int centerRow, int centerCol) {
    appendDebug(QString("Special gem exploding at (%1,%2) - removing 3x3 area")
        .arg(centerRow).arg(centerCol));
    
    int removed = 0;
    for (int i = centerRow - 1; i <= centerRow + 1; i++) {
        for (int j = centerCol - 1; j <= centerCol + 1; j++) {
            if (i >= 0 && i < 8 && j >= 0 && j < 8) {
                Gemstone* gem = gemstoneContainer[i][j];
                if (gem) {
                    removed++;
                    eliminateAnime(gem);
                    gemstoneContainer[i][j] = nullptr;
                }
            }
        }
    }
    
    if (removed > 0) {
        gameScore += removed * 15;  // 特殊宝石消除给更多分数
        updateScoreBoard();
        appendDebug(QString("Special gem removed %1 gems in 3x3 area").arg(removed));
    }
}

// 检查匹配组中是否包含特殊宝石
bool PuzzleModeGameWidget::hasSpecialGem(const std::vector<std::pair<int, int>>& group) const {
    for (const auto& pos : group) {
        Gemstone* gem = gemstoneContainer[pos.first][pos.second];
        if (gem && gem->isSpecial()) {
            return true;
        }
    }
    return false;
}
void PuzzleModeGameWidget::pushInLastStateQueue() {
    std::string GemState = "";

    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            if(gemstoneContainer[i][j] != nullptr) {
                GemState += std::to_string(gemstoneContainer[i][j]->getType());
            }
            else {
                GemState += "-";
            }
        }
    }
    lastGemStateStack.push(GemState);
    return ;
}
