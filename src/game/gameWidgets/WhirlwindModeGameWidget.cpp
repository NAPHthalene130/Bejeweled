#include "WhirlwindModeGameWidget.h"
#include "FinalWidget.h"
#include "MenuWidget.h"
#include "../GameWindow.h"
#include "../components/Gemstone.h"
#include "../components/RotationSquare.h"
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
#include <Qt3DRender/QPickingSettings>
#include <Qt3DRender/QRenderSettings>
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

WhirlwindModeGameWidget::WhirlwindModeGameWidget(QWidget* parent, GameWindow* gameWindow)
    : QWidget(parent), gameWindow(gameWindow), canOpe(true), nowTimeHave(0), mode(2),
      hasSelection(false), selectedTopLeftRow(-1), selectedTopLeftCol(-1) {

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
    game3dWindow->defaultFrameGraph()->setClearColor(QColor(40, 40, 45));

    // 设置3D场景
    setup3DScene();

    // 创建3D窗口容器
    container3d = QWidget::createWindowContainer(game3dWindow);
    container3d->setFixedSize(960, 960);
    container3d->setFocusPolicy(Qt::StrongFocus);
    container3d->setMouseTracking(true);
    container3d->setAttribute(Qt::WA_Hover, true);

    // 布局 - 左侧居中
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(50, 0, 50, 0);

    mainLayout->addWidget(container3d, 0, Qt::AlignLeft | Qt::AlignVCenter);
    mainLayout->addStretch(1);

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
    debugText = new QTextEdit(rightPanel);
    debugText->setVisible(false);
    debugText->setReadOnly(true);
    debugTimer = new QTimer(this);

    setLayout(mainLayout);
    mainLayout->addWidget(rightPanel, 0, Qt::AlignRight | Qt::AlignVCenter);
    container3d->installEventFilter(this);
    game3dWindow->installEventFilter(this);

    // 初始化无操作计时器
    inactivityTimer = new QTimer(this);
    inactivityTimer->setInterval(inactivityTimeout);
    inactivityTimer->setSingleShot(true);

    connect(inactivityTimer, &QTimer::timeout, this, &WhirlwindModeGameWidget::highlightMatches);

    connect(this, &WhirlwindModeGameWidget::userActionOccurred, [this]() {
        resetInactivityTimer();
    });

    inactivityTimer->stop();

    updateScoreBoard();
    updateTimeBoard();
    appendDebug("WhirlwindModeGameWidget initialized - 2x2 rotation mode");
}

void WhirlwindModeGameWidget::GameTimeKeeper::reset() {
    seconds = 0;
}

void WhirlwindModeGameWidget::GameTimeKeeper::tick() {
    ++seconds;
}

int WhirlwindModeGameWidget::GameTimeKeeper::totalSeconds() const {
    return seconds;
}

QString WhirlwindModeGameWidget::GameTimeKeeper::displayText() const {
    int m = seconds / 60;
    int s = seconds % 60;
    return QString("游戏进行时间：%1:%2")
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}

void WhirlwindModeGameWidget::updateScoreBoard() {
    if (!scoreBoardLabel) return;
    scoreBoardLabel->setText(QString("当前分数：%1").arg(gameScore));
}

void WhirlwindModeGameWidget::updateTimeBoard() {
    if (!timeBoardLabel) return;
    timeBoardLabel->setText(gameTimeKeeper.displayText());
}

void WhirlwindModeGameWidget::triggerFinishIfNeeded() {
    if (isFinishing) return;
    if (gameScore < targetScore) return;
    finishToFinalWidget();
}

void WhirlwindModeGameWidget::finishToFinalWidget() {
    if (isFinishing) return;
    isFinishing = true;
    canOpe = false;

    if (timer && timer->isActive()) timer->stop();
    if (inactivityTimer) inactivityTimer->stop();
    clearHighlights();
    if (rotationSquare) rotationSquare->setVisible(false);

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
std::vector<std::pair<int, int>> WhirlwindModeGameWidget::findMatches() {
    std::vector<std::pair<int, int>> matches;
    std::vector<std::vector<bool>> marked(8, std::vector<bool>(8, false));

    // 检查水平方向
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 6; ++j) {
            Gemstone* gem1 = gemstoneContainer[i][j];
            Gemstone* gem2 = gemstoneContainer[i][j+1];
            Gemstone* gem3 = gemstoneContainer[i][j+2];

            if (gem1 && gem2 && gem3 &&
                gem1->getType() == gem2->getType() &&
                gem2->getType() == gem3->getType()) {

                marked[i][j] = true;
                marked[i][j+1] = true;
                marked[i][j+2] = true;

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
        for (int i = 0; i < 6; ++i) {
            Gemstone* gem1 = gemstoneContainer[i][j];
            Gemstone* gem2 = gemstoneContainer[i+1][j];
            Gemstone* gem3 = gemstoneContainer[i+2][j];

            if (gem1 && gem2 && gem3 &&
                gem1->getType() == gem2->getType() &&
                gem2->getType() == gem3->getType()) {

                marked[i][j] = true;
                marked[i+1][j] = true;
                marked[i+2][j] = true;

                int k = i + 3;
                while (k < 8 && gemstoneContainer[k][j] &&
                       gemstoneContainer[k][j]->getType() == gem1->getType()) {
                    marked[k][j] = true;
                    k++;
                }
            }
        }
    }

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
void WhirlwindModeGameWidget::removeMatches(const std::vector<std::pair<int, int>>& matches) {
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
        gameScore += removedCount * 10;
        updateScoreBoard();
        triggerFinishIfNeeded();
    }
}

void WhirlwindModeGameWidget::eliminate() {
    if (isFinishing) return;
    std::vector<std::pair<int, int>> matches = findMatches();
    if (!matches.empty()) {
        comboCount++;
        appendDebug(QString("Found %1 matches to eliminate").arg(matches.size()));
        AudioManager::instance().playEliminateSound(comboCount);

        canOpe = false;
        removeMatches(matches);

        if (isFinishing) return;

        QTimer::singleShot(600, this, [this]() {
            drop();
        });
    } else {
        comboCount = 0;
        canOpe = true;
        resetInactivityTimer();
        appendDebug("No matches found, game can continue");
    }
}

void WhirlwindModeGameWidget::drop() {
    if (isFinishing) return;
    appendDebug("Starting drop animation");

    QParallelAnimationGroup* dropAnimGroup = new QParallelAnimationGroup();
    bool hasDrops = false;

    for (int col = 0; col < 8; ++col) {
        int writePos = 7;
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

void WhirlwindModeGameWidget::resetGemstoneTable() {
    if (isFinishing) return;
    appendDebug("Filling empty positions with new gemstones");

    QParallelAnimationGroup* fillAnimGroup = new QParallelAnimationGroup();
    bool hasFills = false;

    for (int col = 0; col < 8; ++col) {
        for (int row = 0; row < 8; ++row) {
            if (gemstoneContainer[row][col] == nullptr) {
                int type = QRandomGenerator::global()->bounded(difficulty);

                if (col >= 2 && gemstoneContainer[row][col-1] && gemstoneContainer[row][col-2]) {
                    int type1 = gemstoneContainer[row][col-1]->getType();
                    int type2 = gemstoneContainer[row][col-2]->getType();
                    if (type1 == type2 && type == type1) {
                        type = (type + 1) % difficulty;
                    }
                }

                if (row >= 2 && gemstoneContainer[row-1][col] && gemstoneContainer[row-2][col]) {
                    int type1 = gemstoneContainer[row-1][col]->getType();
                    int type2 = gemstoneContainer[row-2][col]->getType();
                    if (type1 == type2 && type == type1) {
                        type = (type + 1) % difficulty;
                    }
                }

                Gemstone* gem = new Gemstone(type, "default", rootEntity);

                QVector3D startPos = getPosition(row - 3, col);
                QVector3D targetPos = getPosition(row, col);

                gem->transform()->setTranslation(startPos);

                connect(gem, &Gemstone::clicked, this, &WhirlwindModeGameWidget::handleGemstoneClicked);
                connect(gem, &Gemstone::pickEvent, this, [this](const QString& info) {
                    appendDebug(QString("Gemstone %1").arg(info));
                });

                gemstoneContainer[row][col] = gem;

                QPropertyAnimation* fillAnim = new QPropertyAnimation(gem->transform(), "translation");
                fillAnim->setDuration(500);
                fillAnim->setStartValue(startPos);
                fillAnim->setEndValue(targetPos);
                fillAnimGroup->addAnimation(fillAnim);

                hasFills = true;
            }
        }
    }

    if (hasFills) {
        appendDebug("Fill animation started");
        connect(fillAnimGroup, &QParallelAnimationGroup::finished, this, [this]() {
            if (isFinishing) return;
            appendDebug("Fill animation finished, checking for new matches");
            eliminate();
        });
        fillAnimGroup->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        appendDebug("No fills needed, checking for new matches");
        eliminate();
    }
}

void WhirlwindModeGameWidget::eliminateAnime(Gemstone* gemstone) {
    if (!gemstone) return;

    QPropertyAnimation* animation = new QPropertyAnimation(gemstone->transform(), "scale");
    animation->setDuration(500);
    animation->setStartValue(gemstone->transform()->scale());
    animation->setEndValue(0.0f);

    connect(animation, &QPropertyAnimation::finished, [gemstone]() {
        gemstone->setParent((Qt3DCore::QNode*)nullptr);
        delete gemstone;
    });

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

// 2x2顺时针旋转动画
void WhirlwindModeGameWidget::rotateGemstonesAnime(Gemstone* topLeft, Gemstone* topRight,
                                                     Gemstone* bottomRight, Gemstone* bottomLeft) {
    if (!topLeft || !topRight || !bottomRight || !bottomLeft) return;

    QVector3D posTL = topLeft->transform()->translation();
    QVector3D posTR = topRight->transform()->translation();
    QVector3D posBR = bottomRight->transform()->translation();
    QVector3D posBL = bottomLeft->transform()->translation();

    QParallelAnimationGroup* group = new QParallelAnimationGroup();

    // 顺时针旋转: TL->TR, TR->BR, BR->BL, BL->TL
    QPropertyAnimation* animTL = new QPropertyAnimation(topLeft->transform(), "translation");
    animTL->setDuration(500);
    animTL->setStartValue(posTL);
    animTL->setEndValue(posTR);

    QPropertyAnimation* animTR = new QPropertyAnimation(topRight->transform(), "translation");
    animTR->setDuration(500);
    animTR->setStartValue(posTR);
    animTR->setEndValue(posBR);

    QPropertyAnimation* animBR = new QPropertyAnimation(bottomRight->transform(), "translation");
    animBR->setDuration(500);
    animBR->setStartValue(posBR);
    animBR->setEndValue(posBL);

    QPropertyAnimation* animBL = new QPropertyAnimation(bottomLeft->transform(), "translation");
    animBL->setDuration(500);
    animBL->setStartValue(posBL);
    animBL->setEndValue(posTL);

    group->addAnimation(animTL);
    group->addAnimation(animTR);
    group->addAnimation(animBR);
    group->addAnimation(animBL);

    connect(group, &QParallelAnimationGroup::finished, [this]() {
        syncGemstonePositions();
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

// Whirlwind模式的点击处理 - 选择2x2区域的左上角
void WhirlwindModeGameWidget::handleGemstoneClicked(Gemstone* gem) {
    if (!gem) {
        appendDebug("handleGemstoneClicked: gem is null!");
        return;
    }
    if (!canOpe) return;
    emit userActionOccurred();

    int row = -1, col = -1;
    if (!findGemstonePosition(gem, row, col)) {
        appendDebug("ERROR: Could not find gemstone position!");
        return;
    }

    appendDebug(QString("Gemstone clicked at (%1,%2)").arg(row).arg(col));

    // 检查是否可以作为2x2方框的左上角
    if (!canFormSquare(row, col)) {
        appendDebug("Cannot form 2x2 square from this position");
        return;
    }

    // 如果已经有选择,并且点击的是同一个位置,则取消选择
    if (hasSelection && row == selectedTopLeftRow && col == selectedTopLeftCol) {
        hasSelection = false;
        selectedTopLeftRow = -1;
        selectedTopLeftCol = -1;
        rotationSquare->setVisible(false);
        appendDebug("Selection cancelled");
        return;
    }

    // 设置新的选择
    hasSelection = true;
    selectedTopLeftRow = row;
    selectedTopLeftCol = col;

    // 显示旋转框
    QVector3D topLeftPos = getPosition(row, col);
    QVector3D bottomRightPos = getPosition(row + 1, col + 1);
    rotationSquare->setPosition(topLeftPos, bottomRightPos);
    rotationSquare->setVisible(true);

    appendDebug(QString("Selected 2x2 square at (%1,%2)").arg(row).arg(col));

    // 立即执行旋转
    performRotation(row, col);
}

void WhirlwindModeGameWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        // 取消选择
        if (hasSelection) {
            hasSelection = false;
            selectedTopLeftRow = -1;
            selectedTopLeftCol = -1;
            rotationSquare->setVisible(false);
            appendDebug("Selection cleared by right click");
        }
    }
    QWidget::mousePressEvent(event);
}

void WhirlwindModeGameWidget::showEvent(QShowEvent* event) {
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

void WhirlwindModeGameWidget::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (timer && timer->isActive()) {
        timer->stop();
    }
    if (inactivityTimer) {
        inactivityTimer->stop();
    }
    clearHighlights();
}

bool WhirlwindModeGameWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == container3d) {
        if (event->type() == QEvent::FocusIn) {
            refreshDebugStatus();
            appendDebug("container3d FocusIn");
        } else if (event->type() == QEvent::FocusOut) {
            refreshDebugStatus();
            appendDebug("container3d FocusOut");
        }
    } else if (obj == game3dWindow) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            appendDebug(QString("game3dWindow MouseButtonPress at (%1, %2)")
                .arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y()));

            handleManualClick(mouseEvent->pos());
            refreshDebugStatus();
            return false;
        } else if (event->type() == QEvent::MouseMove) {
            static int moveCount = 0;
            if (++moveCount % 50 == 0) {
                appendDebug("Mouse moving over 3D window");
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

WhirlwindModeGameWidget::~WhirlwindModeGameWidget() {
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

void WhirlwindModeGameWidget::setup3DScene() {
    rootEntity = new Qt3DCore::QEntity();
    game3dWindow->setRootEntity(rootEntity);

    Qt3DRender::QRenderSettings *renderSettings = game3dWindow->renderSettings();
    if (renderSettings) {
        Qt3DRender::QPickingSettings *pickingSettings = renderSettings->pickingSettings();
        if (pickingSettings) {
            pickingSettings->setPickMethod(Qt3DRender::QPickingSettings::TrianglePicking);
            pickingSettings->setPickResultMode(Qt3DRender::QPickingSettings::NearestPick);
            pickingSettings->setFaceOrientationPickingMode(Qt3DRender::QPickingSettings::FrontAndBackFace);
        }
    }

    cameraEntity = game3dWindow->camera();
    cameraEntity->lens()->setPerspectiveProjection(45.0f, 1.0f, 0.1f, 1000.0f);
    cameraEntity->setPosition(QVector3D(0.0f, 0.0f, 20.0f));
    cameraEntity->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));

    lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight* light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor(Qt::white);
    light->setIntensity(1.0f);
    lightEntity->addComponent(light);

    Qt3DCore::QTransform* lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(QVector3D(0.0f, 0.0f, 20.0f));
    lightEntity->addComponent(lightTransform);

    // 初始化旋转框
    rotationSquare = new RotationSquare(rootEntity);

    qDebug() << "[WhirlwindModeGameWidget] 3D Scene setup complete - Rotation mode";
}

QVector3D WhirlwindModeGameWidget::getPosition(int row, int col) const {
    float startX = -3.5f * 1.5f;
    float startY = 3.5f * 1.5f;
    float spacing = 1.5f;

    float x = startX + col * spacing;
    float y = startY - row * spacing;

    return QVector3D(x, y, 0.0f);
}

void WhirlwindModeGameWidget::syncGemstonePositions() {
    for (int i = 0; i < gemstoneContainer.size(); ++i) {
        for (int j = 0; j < gemstoneContainer[i].size(); ++j) {
            Gemstone* gem = gemstoneContainer[i][j];
            if (gem) {
                gem->transform()->setTranslation(getPosition(i, j));
            }
        }
    }
}

Qt3DExtras::Qt3DWindow* WhirlwindModeGameWidget::getGame3dWindow() const {
    return game3dWindow;
}

std::vector<std::vector<Gemstone*>> WhirlwindModeGameWidget::getGemstoneContainer() const {
    return gemstoneContainer;
}

void WhirlwindModeGameWidget::setGemstoneContainer(const std::vector<std::vector<Gemstone*>>& container) {
    this->gemstoneContainer = container;
}

std::string WhirlwindModeGameWidget::getStyle() const {
    return style;
}

void WhirlwindModeGameWidget::setStyle(const std::string& style) {
    this->style = style;
}

bool WhirlwindModeGameWidget::getCanOpe() const {
    return canOpe;
}

void WhirlwindModeGameWidget::setCanOpe(bool canOpe) {
    this->canOpe = canOpe;
}

QTimer* WhirlwindModeGameWidget::getTimer() const {
    return timer;
}

void WhirlwindModeGameWidget::setTimer(QTimer* timer) {
    this->timer = timer;
}

int WhirlwindModeGameWidget::getNowTimeHave() const {
    return nowTimeHave;
}

void WhirlwindModeGameWidget::setNowTimeHave(int time) {
    this->nowTimeHave = time;
}

int WhirlwindModeGameWidget::getMode() const {
    return mode;
}

void WhirlwindModeGameWidget::setMode(int mode) {
    this->mode = mode;
}

void WhirlwindModeGameWidget::reset(int mode) {
    if (gameWindow) {
        difficulty = gameWindow->getDifficulty();
    }
    this->mode = mode;
    this->canOpe = true;
    this->isFinishing = false;
    this->gameScore = 0;
    this->targetScore = 10000;
    this->gameTimeKeeper.reset();
    this->nowTimeHave = 0;
    this->hasSelection = false;
    this->selectedTopLeftRow = -1;
    this->selectedTopLeftCol = -1;
    updateScoreBoard();
    updateTimeBoard();
    appendDebug(QString("reset mode=%1").arg(mode));

    // 清除现有的宝石
    for (auto& row : gemstoneContainer) {
        for (auto* gem : row) {
            if (gem) {
                gem->setParent((Qt3DCore::QNode*)nullptr);
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

            if (j >= 2 && gemstoneContainer[i][j-1] && gemstoneContainer[i][j-2]) {
                int type1 = gemstoneContainer[i][j-1]->getType();
                int type2 = gemstoneContainer[i][j-2]->getType();
                if (type1 == type2 && type == type1) {
                    type = (type + 1) % difficulty;
                }
            }

            if (i >= 2 && gemstoneContainer[i-1][j] && gemstoneContainer[i-2][j]) {
                int type1 = gemstoneContainer[i-1][j]->getType();
                int type2 = gemstoneContainer[i-2][j]->getType();
                if (type1 == type2 && type == type1) {
                    type = (type + 1) % difficulty;
                }
            }

            Gemstone* gem = new Gemstone(type, "default", rootEntity);

            gem->transform()->setTranslation(getPosition(i, j));

            connect(gem, &Gemstone::clicked, this, &WhirlwindModeGameWidget::handleGemstoneClicked);
            connect(gem, &Gemstone::pickEvent, this, [this](const QString& info) {
                appendDebug(QString("Gemstone %1").arg(info));
            });

            gemstoneContainer[i][j] = gem;
        }
    }
    appendDebug("created 8x8 gemstones with no initial matches");

    if (timer->isActive()) {
        timer->stop();
    }
}

void WhirlwindModeGameWidget::appendDebug(const QString& text) {
    if (!debugText) return;
    debugText->append(QString("[%1] %2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
        .arg(text));
}

void WhirlwindModeGameWidget::refreshDebugStatus() {
    if (!focusInfoLabel) return;
    bool hasFocusContainer = container3d ? container3d->hasFocus() : false;
    QWidget* active = QApplication::activeWindow();
    QString activeTitle = active ? active->windowTitle() : QString("null");
    focusInfoLabel->setText(QString("ContainerFocus=%1 | ActiveWindow=%2")
        .arg(hasFocusContainer ? "true" : "false")
        .arg(activeTitle));
}

bool WhirlwindModeGameWidget::findGemstonePosition(Gemstone* gem, int& row, int& col) const {
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

// 检查位置是否可以作为2x2方框的左上角
bool WhirlwindModeGameWidget::canFormSquare(int row, int col) const {
    // 必须保证row+1和col+1不越界
    if (row < 0 || row >= 7 || col < 0 || col >= 7) {
        return false;
    }
    // 必须保证所有4个位置都有宝石
    return gemstoneContainer[row][col] != nullptr &&
           gemstoneContainer[row][col+1] != nullptr &&
           gemstoneContainer[row+1][col] != nullptr &&
           gemstoneContainer[row+1][col+1] != nullptr;
}

// 执行2x2顺时针旋转
void WhirlwindModeGameWidget::performRotation(int topLeftRow, int topLeftCol) {
    if (!canFormSquare(topLeftRow, topLeftCol)) {
        appendDebug("ERROR: Cannot perform rotation - invalid square");
        return;
    }

    // 获取四个宝石
    Gemstone* topLeft = gemstoneContainer[topLeftRow][topLeftCol];
    Gemstone* topRight = gemstoneContainer[topLeftRow][topLeftCol+1];
    Gemstone* bottomLeft = gemstoneContainer[topLeftRow+1][topLeftCol];
    Gemstone* bottomRight = gemstoneContainer[topLeftRow+1][topLeftCol+1];

    // 在逻辑容器中顺时针旋转
    // TL -> TR位置, TR -> BR位置, BR -> BL位置, BL -> TL位置
    gemstoneContainer[topLeftRow][topLeftCol] = bottomLeft;
    gemstoneContainer[topLeftRow][topLeftCol+1] = topLeft;
    gemstoneContainer[topLeftRow+1][topLeftCol+1] = topRight;
    gemstoneContainer[topLeftRow+1][topLeftCol] = bottomRight;

    // 播放旋转动画
    rotateGemstonesAnime(topLeft, topRight, bottomRight, bottomLeft);

    // 延迟检查匹配
    QTimer::singleShot(550, this, [this]() {
        appendDebug("Rotation animation finished, checking for matches");

        std::vector<std::pair<int, int>> matches = findMatches();

        if (!matches.empty()) {
            appendDebug(QString("Found matches after rotation, starting elimination"));
            eliminate();
        } else {
            appendDebug("No matches found after rotation");
            canOpe = true;
        }

        // 隐藏旋转框
        hasSelection = false;
        selectedTopLeftRow = -1;
        selectedTopLeftCol = -1;
        rotationSquare->setVisible(false);
    });

    appendDebug(QString("Rotated 2x2 square at (%1,%2)").arg(topLeftRow).arg(topLeftCol));
}

void WhirlwindModeGameWidget::handleManualClick(const QPoint& screenPos) {
    float screenWidth = 960.0f;
    float screenHeight = 960.0f;

    float fovRadians = 45.0f * M_PI / 180.0f;
    float cameraDistance = 20.0f;
    float halfHeight = cameraDistance * std::tan(fovRadians / 2.0f);
    float halfWidth = halfHeight;

    float normalizedX = (screenPos.x() - screenWidth / 2.0f) / (screenWidth / 2.0f);
    float normalizedY = -(screenPos.y() - screenHeight / 2.0f) / (screenHeight / 2.0f);

    float worldX = normalizedX * halfWidth;
    float worldY = normalizedY * halfHeight;

    appendDebug(QString("Click at screen(%1,%2) -> world(%3, %4)")
        .arg(screenPos.x()).arg(screenPos.y())
        .arg(worldX, 0, 'f', 2).arg(worldY, 0, 'f', 2));

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

    if (closestGem && minDistance < 0.8f) {
        appendDebug(QString("Found gemstone at (%1,%2), distance=%3")
            .arg(closestRow).arg(closestCol).arg(minDistance, 0, 'f', 2));
        handleGemstoneClicked(closestGem);
    } else {
        appendDebug(QString("No gemstone found near click (min distance=%1)")
            .arg(minDistance, 0, 'f', 2));
    }
}

void WhirlwindModeGameWidget::resetInactivityTimer() {
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

std::vector<std::pair<int, int>> WhirlwindModeGameWidget::findPossibleMatches() {
    // 对于旋转模式,找到所有可能通过2x2旋转产生三消的位置
    // 简化版本:返回所有可以形成2x2方框的左上角位置
    std::vector<std::pair<int, int>> matches;

    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 7; ++j) {
            if (canFormSquare(i, j)) {
                matches.push_back({i, j});
            }
        }
    }

    return matches;
}

void WhirlwindModeGameWidget::highlightMatches() {
    if (!canOpe) return;
    if (!isVisible()) return;
    if (gemstoneContainer.size() != 8) return;
    for (const auto& row : gemstoneContainer) {
        if (row.size() != 8) return;
    }

    clearHighlights();

    std::vector<std::pair<int, int>> matches = findPossibleMatches();
    if (matches.empty()) {
        appendDebug("No possible 2x2 squares found, resetting the game");
        reset(2);
        return;
    }

    appendDebug(QString("No activity detected for %1 seconds, highlighting %2 possible squares")
               .arg(inactivityTimeout/1000).arg(matches.size()));

    // 随机选择一个2x2方框高亮
    int choice = QRandomGenerator::global()->bounded(matches.size());
    const auto& pos = matches[choice];
    int row = pos.first;
    int col = pos.second;

    RotationSquare* square = new RotationSquare(rootEntity);
    QVector3D topLeftPos = getPosition(row, col);
    QVector3D bottomRightPos = getPosition(row + 1, col + 1);
    square->setPosition(topLeftPos, bottomRightPos);
    square->setColor(QColor(255, 100, 100, 200));  // 红色提示
    square->setVisible(true);
    highlightSquares.push_back(square);
}

void WhirlwindModeGameWidget::clearHighlights() {
    for (RotationSquare* square : highlightSquares) {
        square->setVisible(false);
        delete square;
    }
    highlightSquares.clear();
}

void WhirlwindModeGameWidget::setDifficulty(int diff) {
    difficulty = diff;
}

int WhirlwindModeGameWidget::getDifficulty() const {
    return difficulty;
}

// 将匹配的宝石分组（识别连续的匹配）
std::vector<std::vector<std::pair<int, int>>> WhirlwindModeGameWidget::groupMatches(
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
void WhirlwindModeGameWidget::remove3x3Area(int centerRow, int centerCol) {
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
bool WhirlwindModeGameWidget::hasSpecialGem(const std::vector<std::pair<int, int>>& group) const {
    for (const auto& pos : group) {
        Gemstone* gem = gemstoneContainer[pos.first][pos.second];
        if (gem && gem->isSpecial()) {
            return true;
        }
    }
    return false;
}

